// #version 450 core

const float M_PI = 3.141592653589793;

uniform float uScreen_height;
uniform float uScreen_width;

uniform sampler3D uVoxelRadiance;
uniform sampler3D uVoxelOpacity;

uniform bool normalmapping;
uniform sampler2D uTangent;
uniform sampler2D uTangent_normal;

uniform vec3[4] uCone_directions;

uniform vec3 uCamera_position;

uniform sampler2D uDiffuse;
uniform sampler2D uPosition; // World space
uniform sampler2D uNormal;
uniform sampler2D uPBR_parameters;

uniform float uScaling_factor;
uniform uint uVoxel_grid_dimension;
uniform vec3 uAABB_center;
uniform vec3 uAABB_min;
uniform vec3 uAABB_max;

// User customizable
uniform int   uMax_steps;
uniform float uRoughness;
uniform float uMetallic;
uniform float uRoughness_aperature; // Radians
uniform float uMetallic_aperature;  // Radians
uniform bool  uDirect_lighting;
uniform bool  uIndirect_lighting; 

// Material handling related stuff
const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

// Model dependent hard-coded constant
const vec3 base_color_factor = vec3(0.588); 

// Linear --> sRGB approximation
// See http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
// Src: https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/master/src/shaders/metallic-roughness.frag
vec3 linear_to_sRGB(const vec3 color) {
  return pow(color, vec3(INV_GAMMA));
}

// sRGB to linear approximation
// See http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
// Src: https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/master/src/shaders/metallic-roughness.frag
vec4 sRGB_to_linear(const vec4 sRGB) {
  return vec4(pow(sRGB.xyz, vec3(GAMMA)), sRGB.w);
}

// Hejl Richard tone map
// See: http://filmicworlds.com/blog/filmic-tonemapping-operators/
// Src: https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/master/src/shaders/tonemapping.glsl
vec3 tone_map_hejl_richard(vec3 color) {
    color = max(vec3(0.0), color - vec3(0.004));
    return (color*(6.2*color+.5))/(color*(6.2*color+1.7)+0.06);
}

vec3 tone_map(const vec3 color) {
	return linear_to_sRGB(tone_map_hejl_richard(color));
}

ivec3 voxel_coordinate_from_world_pos(const vec3 pos) {
  vec3 vpos = (pos - uAABB_center) * uScaling_factor;
  vpos = clamp(vpos, vec3(-1.0), vec3(1.0));
  const vec3 vgrid = vec3(uVoxel_grid_dimension);
  vpos = vgrid * (vpos * vec3(0.5) + vec3(0.5));
  return ivec3(vpos);
}

vec3 world_to_voxelspace(const vec3 pos) {
  vec3 vpos = (pos - uAABB_center) * uScaling_factor;
  vpos = clamp(vpos, vec3(-1.0), vec3(1.0));
  return vpos * vec3(0.5) + vec3(0.5);
}

bool is_inside_scene(const vec3 p) {
  if (p.x > uAABB_max.x || p.x < uAABB_min.x) { return false; }
  if (p.y > uAABB_max.y || p.y < uAABB_min.y) { return false; }
  if (p.z > uAABB_max.z || p.z < uAABB_min.z) { return false; }
  return true;
}

out vec4 color;

uniform float uVoxel_size_LOD0;

vec4 trace_cone(const vec3 origin,
                const vec3 direction,
                const float half_angle) {
  const float voxel_size_LOD0 = uVoxel_size_LOD0;

  float occlusion = 0.0;
  vec3 radiance = vec3(0.0);
  float cone_distance = voxel_size_LOD0; // Avoid self-occlusion

  while (cone_distance < 10.0 && occlusion < 1.0) {

    const vec3 world_position = origin + cone_distance * direction;
    if (!is_inside_scene(world_position)) { break; }
    
    const float cone_diameter = max(2.0 * tan(half_angle) * cone_distance, voxel_size_LOD0);
    cone_distance += cone_diameter;

    const float mip = log2(cone_diameter / voxel_size_LOD0);

    const vec3 p = world_to_voxelspace(world_position);
    const float a = textureLod(uVoxelOpacity, p, mip).r;

    // Front-to-back acculumation
    radiance += (1.0 - occlusion) * a * textureLod(uVoxelRadiance, p, mip).rgb;
    occlusion += (1.0 - occlusion) * a;
  }

  return vec4(radiance, occlusion);
}

uniform float uShadow_bias;
uniform vec3 uDirectional_light_direction;
uniform mat4 uLight_space_transform;
uniform sampler2D uShadowmap;

bool shadow(const vec3 world_position, const vec3 normal) {
  vec4 lightspace_position = uLight_space_transform * vec4(world_position, 1.0);
  lightspace_position.xyz /= lightspace_position.w;
  lightspace_position = lightspace_position * 0.5 + 0.5;

  const float current_depth = lightspace_position.z;

  if (current_depth < 1.0) { 
    const float closest_shadowmap_depth = texture(uShadowmap, lightspace_position.xy).r;
    
    // Bias avoids the _majority_ of shadow acne
    const float bias = uShadow_bias * dot(-uDirectional_light_direction, normal);

    return closest_shadowmap_depth < current_depth - bias;
  }
  return false;
}
  
void main() {
  const vec2 frag_coord = vec2(gl_FragCoord.x / uScreen_width, gl_FragCoord.y / uScreen_height);

  // Diffuse cones
  const vec3 origin = texture(uPosition, frag_coord).xyz;

  vec3 normal = texture(uNormal, frag_coord).xyz;
  vec3 fNormal = normal; // NOTE: Using geometric normal due to minimal jitter in hard shadows produced

  // Tangent normal mapping & TBN tranformation
  const vec3 T = normalize(texture(uTangent, frag_coord).xyz);
  const vec3 B = normalize(cross(normal, T));
  const vec3 N = normal;
  mat3 TBN = mat3(1.0);

  if (normalmapping) {
    TBN = mat3(T, B, N);
    const vec3 tangent_normal = normalize(2.0 * (texture(uTangent_normal, frag_coord).xyz - vec3(0.5)));
    normal = normalize(TBN * tangent_normal);  
  }

  // Material parameters
  const float roughness = pow(texture(uPBR_parameters, frag_coord).g, 2.0);
  const float metallic = texture(uPBR_parameters, frag_coord).b;
  const float roughness_aperature = uRoughness_aperature;
  const float metallic_aperature = uMetallic_aperature;

  // Diffuse cones
  color += trace_cone(origin, normal, roughness_aperature * roughness);
  #define NUM_DIFFUSE_CONES 4
  for (uint i = 0; i < NUM_DIFFUSE_CONES; i++) {
    const vec3 direction = TBN * normalize(uCone_directions[i]);
    color += trace_cone(origin, direction, roughness_aperature * roughness) * dot(direction, normal);
  }
   
  const vec3 f0 = vec3(0.04);
  const vec3 base_color = sRGB_to_linear(texture(uDiffuse, frag_coord)).rgb * base_color_factor;
  // const vec3 diffuse = base_color * (vec3(1.0) - f0) * (1.0 - metallic); // glTF correct
  const vec3 diffuse = base_color; // Looks better than above (although incorrect..)
  color.rgb *= diffuse; // / M_PI; // Unclear whether to use or not

  // Specular cone
  const vec4 specular = vec4(mix(f0, base_color, metallic), 1.0);
  // const vec4 specular = vec4(1.0);
  const vec3 reflection = reflect(normalize(uCamera_position - origin), normal);
  color += specular * trace_cone(origin, reflection, metallic_aperature * metallic); 

  if (!uIndirect_lighting) {
    color.rgb = vec3(0.0);
  }

  if (uDirect_lighting) {
    if (!shadow(origin, fNormal)) {
      color.rgb += diffuse * max(dot(-uDirectional_light_direction, normal), 0.0);
    }
  }

  color.rgb = linear_to_sRGB(color.rgb);
  // color.rgb = tone_map(color.rgb); // Overexposures everything
}
