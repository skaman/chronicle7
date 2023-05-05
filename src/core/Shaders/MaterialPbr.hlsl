// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma vertex mainVertex
#pragma fragment mainFragment

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSITION0;
    [[vk::location(1)]] float3 Color : COLOR0;
    [[vk::location(2)]] float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    //[[vk::location(0)]] float3 Color : COLOR0;
    [[vk::location(1)]] float2 TexCoord : TEXCOORD0;
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

#ifdef HAS_BASE_COLOR_TEXTURE
SamplerState baseColorTexSampler : register(s1, space1);
Texture2D<float4> baseColorTex : register(t1, space1);
#endif

#ifdef HAS_METALLIC_ROUGHNESS_TEXTURE
SamplerState metallicRoughnessSampler : register(s2, space1);
Texture2D<float4> metallicRoughnessTexture : register(t2, space1);
#endif

#ifdef HAS_NORMAL_TEXTURE
SamplerState normalSampler : register(s3, space1);
Texture2D<float4> normalTexture : register(t3, space1);
#endif

#ifdef HAS_OCCLUSION_TEXTURE
SamplerState occlusionSampler : register(s4, space1);
Texture2D<float4> occlusionTexture : register(t4, space1);
#endif

#ifdef HAS_EMISSIVE_TEXTURE
SamplerState emissiveSampler : register(s5, space1);
Texture2D<float4> emissiveTexture : register(t5, space1);
#endif

VSOutput mainVertex(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    output.Pos = mul(ubo.projectionMatrix, mul(ubo.viewMatrix, mul(ubo.modelMatrix, float4(input.Position.xyz, 1.0))));
    output.TexCoord = input.TexCoord;
    return output;
}

float4 mainFragment(in VSOutput input) : SV_TARGET0
{
    return baseColorTex.Sample(baseColorTexSampler, input.TexCoord);;
}