
const float M_PI = 3.141592653589793;

uniform float screen_width;
uniform float screen_height;

uniform sampler2D tangent_sampler;
uniform sampler2D tangent_normal_sampler;
uniform sampler2D geometric_normal_sampler;
uniform sampler2D depth_sampler;
uniform sampler2D position_sampler;
uniform sampler2D diffuse_sampler;
uniform sampler2D pbr_parameters_sampler;
uniform sampler2D ambient_occlusion_sampler;
// uniform sampler2D emissive_sampler;
uniform usampler2D shading_model_id_sampler;
uniform samplerCubeArray environment_map_sampler;
uniform sampler2D shadow_map_sampler;

// Shadow map related
uniform bool shadowmapping;
uniform mat4 light_space_transform;
uniform vec3 directional_light_direction;

uniform bool normalmapping;

uniform vec3 camera; // TEST

out vec4 outColor; // Defaults to zero when the frag shader only has 1 out variable

struct PBRInputs {
    vec3 L;
    vec3 V;
    vec3 N;
    vec3 H; // Halfvector 
    float NdotL;
    float NdotV;
    float VdotL;
    float VdotH;
    float NdotH;
    float LdotH;
    float metallic;
    float roughness;
    vec3 base_color;
};

struct PointLight { // NOTE: Defined in voxelization.frag also
    vec4 position;  // (X, Y, Z, padding)
    vec4 intensity; // (R, G, B, padding)
};

layout(std140, binding = 4) buffer PointLightBlock {
    PointLight pointlights[];
};

/// Approximation of the Fresnel factor: Expresses the reflection of light reflected by each microfacet
/// https://www.cs.virginia.edu/%7Ejdl/bib/appearance/analytic%20models/schlick94b.pdf : page 7 : equation (15)
vec3 fresnel_schlick(vec3 F0, PBRInputs inputs) {
    return F0 + (vec3(1.0) - F0) * pow(1.0 - inputs.VdotH, 5.0);
}

/// Approximation of geometrical attenuation coefficient: Expresses the ratio of light that is not self-obstrcuted by the surface
/// https://www.cs.virginia.edu/%7Ejdl/bib/appearance/analytic%20models/schlick94b.pdf : page 8 : equation (19)
float geometric_occlusion_schlick(PBRInputs inputs) {
    const float k = sqrt(2.0 * inputs.roughness * inputs.roughness / M_PI);
    const float GL = inputs.NdotL / (inputs.NdotL - k * inputs.NdotL + k);
    const float GV = inputs.NdotV / (inputs.NdotV - k * inputs.NdotV + k);
    return GL * GV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
// [Unreal Engine PBR] https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float microfaced_distribution_trowbridge_reitz(PBRInputs inputs) {
    const float a = inputs.roughness * inputs.roughness; // Disney's reparameterization of roughness
    const float f = (inputs.NdotH * inputs.NdotH) * (a * a - 1.0) + 1.0; // Epic [2013]
    return (a * a) / (M_PI * f * f);
}

/// Lambertian diffuse BRDF
vec3 diffuse_lambertian(vec3 rgb) {
    return rgb / M_PI;
}

/// Disney Implementation of diffuse from Physically-Based Shading at Disney by Brent Burley. See Section 5.3. 
/// http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
vec3 diffuse_disney(PBRInputs inputs) {
    const float f90 = 2.0 * inputs.LdotH * inputs.LdotH * inputs.roughness - 0.5;
    return (inputs.base_color / M_PI) * (1.0 + f90 * pow((1.0 - inputs.NdotL), 5.0)) * (1.0 + f90 * pow((1.0 - inputs.NdotV), 5.0));
}
    
vec3 schlick_brdf(PBRInputs inputs) {
    const vec3 f0 = vec3(0.04);
    const vec3 black = vec3(0.0);
    const vec3 normal_incidence = mix(f0, inputs.base_color, inputs.metallic); // F0/R(0) (Fresnel)
    inputs.base_color = mix(inputs.base_color * (1.0 - f0), black, inputs.metallic);

    const vec3 F = fresnel_schlick(normal_incidence, inputs);
    const vec3 fdiffuse = (1.0 - F) * diffuse_lambertian(inputs.base_color); 

    const float G = geometric_occlusion_schlick(inputs);
    const float D = microfaced_distribution_trowbridge_reitz(inputs);
    const vec3 fspecular = (F * G * D) / (4.0 * inputs.NdotL * inputs.NdotV);
    
    return fdiffuse + fspecular;
}

vec3 schlick_render(vec2 frag_coord, vec3 position, vec3 normal, vec3 diffuse, vec3 ambient_occlusion) {
    PBRInputs pbr_inputs;
    pbr_inputs.roughness = texture(pbr_parameters_sampler, frag_coord).g;
    pbr_inputs.metallic = texture(pbr_parameters_sampler, frag_coord).b;
    pbr_inputs.roughness *= pbr_inputs.roughness; // Convert to material roughness from perceptual roughness
    pbr_inputs.base_color = diffuse.rgb;  
    pbr_inputs.N = normal;

    vec3 L0 = vec3(0.0);
    for (int i = 0; i < pointlights.length(); i++) {
        const PointLight pointlight = pointlights[i];

        pbr_inputs.L = normalize(pointlight.position.xyz - position);
        const float distance = length(pointlight.position.xyz - position);
        const float attenuation = 1.0 / (distance * distance);
        const vec3 radiance = attenuation * pointlight.intensity.rgb;

        // Metallic roughness material model glTF specific 
        pbr_inputs.V = normalize(camera - position);
        pbr_inputs.H = normalize(pbr_inputs.L + pbr_inputs.V);
        pbr_inputs.NdotL = clamp(dot(pbr_inputs.N, pbr_inputs.L), 0.001, 1.0);
        pbr_inputs.NdotV = clamp((dot(pbr_inputs.N, pbr_inputs.V)), 0.001, 1.0);
        pbr_inputs.VdotL = clamp(dot(pbr_inputs.V, pbr_inputs.L), 0.0, 1.0);
        pbr_inputs.VdotH = clamp(dot(pbr_inputs.V, pbr_inputs.H), 0.0, 1.0);
        pbr_inputs.NdotH = clamp(dot(pbr_inputs.N, pbr_inputs.H), 0.0, 1.0);
        pbr_inputs.LdotH = clamp(dot(pbr_inputs.L, pbr_inputs.H), 0.0, 1.0);

        L0 += radiance * schlick_brdf(pbr_inputs) * pbr_inputs.NdotL;
    }
    // TODO: Seperate diffuse and specular terms from the irradiance
    // TODO: Lookup whether or not the irradiance comes in sRGB 
    const vec3 irradiance = texture(environment_map_sampler, vec4(pbr_inputs.N, 0)).rgb;
    const vec3 ambient = irradiance * diffuse_lambertian(pbr_inputs.base_color) * (1.0 - ambient_occlusion);
    L0 += ambient;
    return L0;
}

vec3 SRGB_to_linear(vec3 srgb) {
    // TODO: Look this up
    // vec3 bLess = step(vec3(0.04045), srgb.xyz);
    // return mix(srgb.xyz/vec3(12.92), pow((srgb.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess);
    return pow(srgb, vec3(2.2)); // Fast approximation
}

void main() {
    const vec2 frag_coord = vec2(gl_FragCoord.x / screen_width, gl_FragCoord.y / screen_height);
    vec3 normal = normalize(texture(geometric_normal_sampler, frag_coord).xyz);
    const vec3 tangent_normal = normalize(2 * (texture(tangent_normal_sampler, frag_coord).xyz - vec3(0.5)));
    const vec3 tangent = normalize(texture(tangent_sampler, frag_coord).xyz);
    const vec3 position = texture(position_sampler, frag_coord).xyz;
    const vec3 diffuse = SRGB_to_linear(texture(diffuse_sampler, frag_coord).rgb); // Mandated by glTF 2.0
    const vec3 ambient_occlusion = texture(ambient_occlusion_sampler, frag_coord).rgb;
    // const vec3 emissive = texture(emissive_sampler, frag_coord).rgb;
    const int  shading_model_id = int(texture(shading_model_id_sampler, frag_coord).r);

    // Tangent normal mapping
    if (normalmapping) {
        const vec3 T = tangent;
        const vec3 B = normalize(cross(normal, tangent));
        const vec3 N = normal;
        mat3 TBN = mat3(T, B, N);
        normal = normalize(TBN * tangent_normal);
    }

    // Shadowmap calculations
    // Avoids _some_ shadowmap acne
    const float shadow_bias = 0.005; // max(0.005 * (1.0 - clamp(dot(normal, directional_light_direction), 0.0, 1.0)), 0.0005);

    vec4 lightspace_position = light_space_transform * vec4(position, 1.0);
    lightspace_position.xyz /= lightspace_position.w;
    lightspace_position = lightspace_position * 0.5 + 0.5;
    const float current_shadowmap_depth = lightspace_position.z;

    float shadow = 0.0; // Amount of shadow [0.0, 1.0]
    if (current_shadowmap_depth < 1.0 && shadowmapping) { 
        const float closest_shadowmap_depth = texture(shadow_map_sampler, lightspace_position.xy).r;
        shadow = closest_shadowmap_depth < current_shadowmap_depth - shadow_bias ? 0.5 : 0.0;
    }

    vec3 color = vec3(0.0);
    switch (shading_model_id) {
        case 1: // Unlit
        color = (1.0 - shadow) * diffuse_lambertian(diffuse);
        break;     
        case 2: // Physically based rendering with parameters sourced from textures
        case 3: // Physically based rendering with parameters sourced from scalars
        color = (1.0 - shadow) * schlick_render(frag_coord, position, normal, diffuse, ambient_occlusion);
        // Emissive
        // color += emissive; // TODO: Emissive factor missing
        break;
    }

    // Tone mapping (using Reinhard operator)
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}
