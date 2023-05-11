// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma vertex mainVertex
#pragma fragment mainFragment

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSITION0;
    [[vk::location(1)]] float3 Color : COLOR0;
    [[vk::location(2)]] float2 TexCoord : TEXCOORD0;
    [[vk::location(3)]] float3 Normal : NORMAL0;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    //[[vk::location(0)]] float3 Color : COLOR0;
    [[vk::location(1)]] float2 TexCoord : TEXCOORD0;
    [[vk::location(2)]] float3 Normal : NORMAL0;
};

struct UBO
{
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

cbuffer ubo : register(b0, space0)
{
    UBO ubo;
}

//SamplerState texSampler : register(s1, space0);
//Texture2D<float4> tex : register(t1, space0);

struct MaterialUBO
{
    float4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    float3 emissiveFactor;
    uint alphaMode;
    float alphaCutoff;
    bool doubleSided;
};

cbuffer materialUbo : register(b0, space1)
{
    MaterialUBO materialUbo;
}

//#ifdef HAS_BASE_COLOR_TEXTURE
SamplerState baseColorTexSampler : register(s1, space1);
Texture2D<float4> baseColorTex : register(t1, space1);
//#endif

//#ifdef HAS_METALLIC_ROUGHNESS_TEXTURE
SamplerState metallicRoughnessSampler : register(s2, space1);
Texture2D<float4> metallicRoughnessTexture : register(t2, space1);
//#endif

//#ifdef HAS_NORMAL_TEXTURE
SamplerState normalSampler : register(s3, space1);
Texture2D<float4> normalTexture : register(t3, space1);
//#endif

//#ifdef HAS_OCCLUSION_TEXTURE
SamplerState occlusionSampler : register(s4, space1);
Texture2D<float4> occlusionTexture : register(t4, space1);
//#endif

//#ifdef HAS_EMISSIVE_TEXTURE
SamplerState emissiveSampler : register(s5, space1);
Texture2D<float4> emissiveTexture : register(t5, space1);
//#endif

#define PI 3.14159265359f
//
//float3 CookTorrance(float3 materialDiffuseColor, float3 normal, float3 lightDir, float3 viewDir, float3 lightColor)
//{
//    float NdotL = max(0.0f, dot(normal, lightColor));
//    float Rs = 0.0f;
//    if (NdotL > 0.0f)
//    {
//        float3 H = normalize(lightDir + viewDir);
//        float NdotH = max(0.0f, dot(normal, H));
//        float NdotV = max(0.0f, dot(normal, viewDir));
//        float VdotH = max(0.0f, dot(lightDir, H));
//        
//        // fresnel refractance
//        float F = pow(1.0f - VdotH, 5.0f);
//    }
//    
//}

VSOutput mainVertex(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    output.Pos = mul(ubo.projectionMatrix, mul(ubo.viewMatrix, mul(ubo.modelMatrix, float4(input.Position.xyz, 1.0))));
    output.TexCoord = input.TexCoord;
    output.Normal = input.Normal;
    return output;
}

float3 lin2rgb(float3 lin)
{
    return pow(lin, float3(1.0f / 2.2f));
}

float4 mainFragment(in VSOutput input) : SV_TARGET0
{
    float flux = 1.0f;
    float3 lightPos = float3(500.0f, 300.0f, 100.0f);
    float3 lightDir = lightPos - input.Pos.xyz;
    float3 lightDistance = length(lightDir);
    lightDir = normalize(lightDir);
    float3 viewDir = normalize(float3(0.0f) - input.Pos.xyz);
    //float3 normal = normalTexture.Sample(normalSampler, input.TexCoord);
    //float3 normal = float3(0.5f);
    float3 normal = input.Normal;
    float3 radiance = float3(1.0f);
    float3 color = baseColorTex.Sample(baseColorTexSampler, input.TexCoord);
    float3 irradiance = max(dot(lightDir, normal), 0.0f) * flux / (4.0f * PI * lightDistance * lightDistance);
    //return float4(lightDir, 1.0f);
    //return float4(normal, 1.0f);
    return float4(color, 1.0f);

}