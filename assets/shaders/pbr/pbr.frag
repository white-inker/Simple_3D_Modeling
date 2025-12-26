#version 330 core

in VS_OUT
{
    vec3 WorldPos;
    vec3 WorldNormal;
    vec2 UV;
} fs_in;

out vec4 FragColor;

// ---------------- Lighting uniforms ----------------
uniform vec3 ViewPos;
uniform vec3 Ambient;

struct DirLight
{
    vec3 direction;
    vec3 color;
    float intensity;
};
uniform DirLight dirLight;

struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;
    float range;
};

uniform int PointLightCount;
uniform PointLight PointLights[16];

// ---------------- Material uniforms (PBR metallic-roughness) ----------------
uniform vec4  Material_BaseColorFactor;       // rgba
uniform float Material_MetallicFactor;        // [0..1]
uniform float Material_RoughnessFactor;       // [0..1],建议 >=0.04
uniform float Material_NormalScale;           // >=0
uniform float Material_OcclusionStrength;     // [0..1]
uniform vec3  Material_EmissiveFactor;        // rgb
uniform float Material_EmissiveStrength;      // >=0

// flags
uniform int Material_UseBaseColorMap;
uniform int Material_UseNormalMap;
uniform int Material_UseMetallicRoughnessMap;
uniform int Material_UseOcclusionMap;
uniform int Material_UseEmissiveMap;

// optional render hint
uniform int Material_DoubleSided; // 1: 双面材质，背面翻转法线

// samplers
uniform sampler2D BaseColorMap;            // slot 0
uniform sampler2D NormalMap;               // slot 1
uniform sampler2D MetallicRoughnessMap;    // slot 2 (常见：G=roughness, B=metallic)
uniform sampler2D OcclusionMap;            // slot 3 (R=AO)
uniform sampler2D EmissiveMap;             // slot 4

// shadow mapping
uniform mat4 LightSpaceMatrix;
uniform sampler2D ShadowMap;               // slot 5
uniform float ShadowBias;
uniform float ShadowNormalBias;
uniform int EnableShadow;

// ---------------- Helpers ----------------
const float PI = 3.14159265359;

float saturate(float x) { return clamp(x, 0.0, 1.0); }

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    // Schlick approximation
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / max(PI * denom * denom, 1e-7);
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    // k = (alpha+1)^2 / 8
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    return NdotV / max(NdotV * (1.0 - k) + k, 1e-7);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    float ggx1 = geometrySchlickGGX(NdotV, roughness);
    float ggx2 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// 利用屏幕空间导数构建切线空间（不需要预存 tangent）
// 要求：WorldPos、UV 正常
mat3 cotangentFrame(vec3 N, vec3 p, vec2 uv)
{
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);

    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invMax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invMax, B * invMax, N);
}

vec3 applyNormalMap(vec3 N, vec3 worldPos, vec2 uv)
{
    // normal map in tangent space: [0..1] -> [-1..1]
    vec3 nTex = texture(NormalMap, uv).xyz * 2.0 - 1.0;

    // scale XY, keep Z
    nTex.xy *= Material_NormalScale;
    nTex = normalize(nTex);

    mat3 TBN = cotangentFrame(N, worldPos, uv);
    return normalize(TBN * nTex);
}

float computeShadow(vec3 worldPos, vec3 normal, vec3 lightDir)
{
    if (EnableShadow == 0) return 0.0;

    normal   = normalize(normal);
    lightDir = normalize(lightDir);

    float ndotl = max(dot(normal, lightDir), 0.0);

    // receiver-plane normal bias to fight acne when the surface is far from the light map center
    float normalOffset = ShadowNormalBias * (1.0 - ndotl);
    vec4 lightSpacePos = LightSpaceMatrix * vec4(worldPos + normal * normalOffset, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    // outside light clip -> no shadow
    if (projCoords.z < 0.0 || projCoords.z > 1.0) return 0.0;

    // outside shadow map -> no shadow (and matches CLAMP_TO_BORDER=1)
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    vec2 texelSize = 1.0 / vec2(textureSize(ShadowMap, 0));
	float slopeBias = ShadowBias * (1.0 - ndotl);
    float receiverBias = ShadowBias * max(texelSize.x, texelSize.y) * 1.5;
    float bias = max(slopeBias, receiverBias);

    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
    {
        float closest = texture(ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
        float currentDepth = projCoords.z - bias;
        shadow += (currentDepth > closest) ? 1.0 : 0.0;
    }

    return shadow / 9.0;
}


// 点光衰减：逆平方 + range 截断（smoothstep）
vec3 pointLightRadiance(PointLight light, vec3 worldPos, out vec3 L, out float NdotL, vec3 N)
{
    vec3 toL = light.position - worldPos;
    float d = length(toL);
    if (d < 1e-4)
    {
        L = vec3(0,0,1);
        NdotL = 0.0;
        return vec3(0);
    }

    L = toL / d;
    NdotL = saturate(dot(N, L));
    if (NdotL <= 0.0) return vec3(0);

    // inverse square attenuation
    float atten = 1.0 / max(d * d, 1e-4);

    // range cutoff: x=d/range, 1->0
    float x = d / max(light.range, 1e-4);
    float cutoff = smoothstep(1.0, 0.0, x);

    vec3 radiance = light.color * light.intensity * atten * cutoff;
    return radiance;
}

void main()
{
    // --------- Material inputs ----------
    vec4 base = Material_BaseColorFactor;
    if (Material_UseBaseColorMap == 1)
        base *= texture(BaseColorMap, fs_in.UV);

    float metallic  = Material_MetallicFactor;
    float roughness = Material_RoughnessFactor;

    if (Material_UseMetallicRoughnessMap == 1)
    {
        // 常见 packing：G=roughness, B=metallic（glTF metallicRoughness 约定）
        vec4 mr = texture(MetallicRoughnessMap, fs_in.UV);
        roughness *= mr.g;
        metallic  *= mr.b;
    }

    // clamp for stability
    metallic  = clamp(metallic, 0.0, 1.0);
    roughness = clamp(roughness, 0.04, 1.0);

    float ao = 1.0;
    if (Material_UseOcclusionMap == 1)
    {
        float aoTex = texture(OcclusionMap, fs_in.UV).r;
        ao = mix(1.0, aoTex, clamp(Material_OcclusionStrength, 0.0, 1.0));
    }

    vec3 emissive = Material_EmissiveFactor * Material_EmissiveStrength;
    if (Material_UseEmissiveMap == 1)
    emissive *= texture(EmissiveMap, fs_in.UV).rgb;

    // --------- Geometry ----------
    vec3 N = normalize(fs_in.WorldNormal);
    if (Material_DoubleSided == 1 && !gl_FrontFacing)
    N = -N;

    if (Material_UseNormalMap == 1)
    N = applyNormalMap(N, fs_in.WorldPos, fs_in.UV);

    vec3 V = normalize(ViewPos - fs_in.WorldPos);

    // baseColor in linear space
    vec3 albedo = base.rgb;
    float alpha = base.a;

    // F0: dielectric 0.04, metal uses albedo
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // --------- Lighting accumulation ----------
    vec3 Lo = vec3(0.0);

    // Directional light
    if (dirLight.intensity > 0.0)
    {
        vec3 L = normalize(-dirLight.direction);
        float NdotL = saturate(dot(N, L));
        if (NdotL > 0.0)
        {
            vec3 H = normalize(V + L);

            float D = distributionGGX(N, H, roughness);
            float G = geometrySmith(N, V, L, roughness);
            vec3  F = fresnelSchlick(saturate(dot(H, V)), F0);

            vec3 numerator = D * G * F;
            float denom = max(4.0 * saturate(dot(N, V)) * NdotL, 1e-7);
            vec3 specular = numerator / denom;

            vec3 kS = F;
            vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

            vec3 radiance = dirLight.color * dirLight.intensity;

            // Lambert diffuse: albedo / PI
            vec3 diffuse = kD * albedo / PI;

            float shadow = computeShadow(fs_in.WorldPos, N, L);

            Lo += (diffuse + specular) * radiance * NdotL * (1.0 - shadow);
        }
    }

    // Point lights
    int count = min(PointLightCount, 16);
    for (int i = 0; i < count; ++i)
    {
        vec3 L;
        float NdotL;
        vec3 radiance = pointLightRadiance(PointLights[i], fs_in.WorldPos, L, NdotL, N);
        if (NdotL <= 0.0) continue;
        if (radiance.x + radiance.y + radiance.z <= 0.0) continue;

        vec3 H = normalize(V + L);

        float D = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);
        vec3  F = fresnelSchlick(saturate(dot(H, V)), F0);

        vec3 numerator = D * G * F;
        float denom = max(4.0 * saturate(dot(N, V)) * NdotL, 1e-7);
        vec3 specular = numerator / denom;

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        vec3 diffuse = kD * albedo / PI;

        Lo += (diffuse + specular) * radiance * NdotL;
    }

    // Ambient term (no IBL): use uniform Ambient as approximation
    vec3 ambient = Ambient * albedo * ao;

    vec3 color = ambient + Lo + emissive;

    // 如果你没有开启 sRGB framebuffer/纹理，建议做输出 gamma 校正：
    // color = pow(color, vec3(1.0/2.2));

	color = color / (color + vec3(1.0)); // Reinhard
    FragColor = vec4(color, alpha);
}
