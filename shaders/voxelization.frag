// #version 450 core 

#define NUM_CLIPMAPS 4

// layout (pixel_center_integer) in vec4 gl_FragCoord; // Conserv. raster. 
in vec3 fNormal;   
in vec3 fPosition;        // World space position
in vec2 fTextureCoord;
in vec4 fAABB;            // Bounding triangle (conservative rasterization)
flat in uint fInstanceIdx; // Object geometry instance idx

uniform int uClipmap_sizes[NUM_CLIPMAPS];
layout(R32UI) uniform restrict coherent volatile uimage3D uVoxel_radiance[NUM_CLIPMAPS];
layout(RGBA8) uniform restrict writeonly image3D uVoxel_opacity[NUM_CLIPMAPS];

uniform sampler2DArray uDiffuse;
uniform sampler2D uEmissive;

uniform bool uConservative_rasterization;
uniform vec3 uCamera_position;
uniform vec3 uAABB_centers[NUM_CLIPMAPS];
uniform float uScaling_factors[NUM_CLIPMAPS];

/// Mirrors struct declaration in graphicsbatch.hpp
struct Material {
  uint diffuse_layer_idx;
  uint shading_model;
  vec2 pbr_scalar_parameters; // (roughness, metallic)
  vec4 emissive_scalars;      // instead of emissive texture, (vec3, padding)
  vec4 diffuse_scalars;       // instead of diffuse texture, (vec3, padding)
};

// Default binding set in geometry.frag shader
layout(std140, binding = 3) readonly buffer MaterialBlock {
    Material materials[];
};

ivec3 voxel_coordinate_from_world_pos(const vec3  p,   // World position
                                      const vec3  c,   // AABB center
                                      const uint  d,   // Voxel grid dimension
                                      const float s) { // Voxel grid scaling  
  const float offset = 0.5 / float(d); // Voxel grid offset
  return ivec3(d * ((p - c) * s + 0.5) + offset);
}
	
// Slightly modified and copied from: https://rauwendaal.net/2013/02/07/glslrunningaverage/
void imageAtomicAverageRGBA8(layout(r32ui) coherent volatile uimage3D voxels,
                             const vec3 nextVec3,
                             const ivec3 coord) {
  uint nextUint = packUnorm4x8(vec4(nextVec3, 1.0f / 255.0f));
  uint prevUint = 0;
  uint currUint = 0;
  vec4 currVec4;
  vec3 average;
  uint count;
 
  // Spin while threads are trying to change the voxel
  while((currUint = imageAtomicCompSwap(voxels, coord, prevUint, nextUint)) != prevUint) {
    prevUint = currUint;                    // Store packed RGB average and count
    currVec4 = unpackUnorm4x8(currUint);    // Unpack stored RGB average and count

    average =      currVec4.rgb;          // Extract RGB average
    count   = uint(currVec4.a * 255.0f);  // Extract count

    // Compute the running average
    average = (average * count + nextVec3) / (count + 1);

    // Pack new average and incremented count back into a uint
    nextUint = packUnorm4x8(vec4(average, (count + 1) / 255.0f));
  }
}

uniform float uShadow_bias;
uniform vec3 uDirectional_light_direction;
uniform vec3 uDirectional_light_intensity;
uniform mat4 uLight_space_transform;
uniform sampler2D uShadowmap;

float shadow(const vec3 world_position, const vec3 normal) {
  vec4 lightspace_position = uLight_space_transform * vec4(world_position, 1.0);
  lightspace_position.xyz /= lightspace_position.w;
  lightspace_position = lightspace_position * 0.5 + 0.5;

  const float current_depth = lightspace_position.z;

  const float closest_shadowmap_depth = texture(uShadowmap, lightspace_position.xy).r;
    
  // Bias avoids the _majority_ of shadow acne
  const float bias = uShadow_bias * dot(-uDirectional_light_direction, normal);

  return (closest_shadowmap_depth < current_depth - bias) ? 0.0 : 1.0;
}

void main() {
  const Material material = materials[fInstanceIdx];
  const uint clipmap = gl_ViewportIndex;

  // FIXME: Used in Nopper's (non-working) conservative rasterization
   if (uConservative_rasterization) {
     const vec2 p = (gl_FragCoord.xy / vec2(uClipmap_sizes[clipmap])) * 2.0 - 1.0;
     if (p.x < fAABB.x || p.y < fAABB.y || p.x > fAABB.z || p.y > fAABB.w) {
       discard;
     }
   }

  const ivec3 vpos = voxel_coordinate_from_world_pos(fPosition,
                                                     uAABB_centers[clipmap],
                                                     uClipmap_sizes[clipmap],
                                                     uScaling_factors[clipmap]);
  const vec3 radiance = texture(uDiffuse, vec3(fTextureCoord, 0)).rgb + material.diffuse_scalars.rgb;
  const vec3 emissive = texture(uEmissive, fTextureCoord).rgb + material.emissive_scalars.rgb;

  // Inject radiance if voxel NOT in shadow
  const vec3 value = uDirectional_light_intensity * shadow(fPosition, fNormal) * radiance + emissive;
  if (abs(dot(value, vec3(1.0))) > 0.0) {
      imageAtomicAverageRGBA8(uVoxel_radiance[clipmap], radiance + emissive, vpos);
  }
  imageStore(uVoxel_opacity[clipmap], vpos, vec4(1.0));
}
