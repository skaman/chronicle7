// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

struct VSInput {
    [[vk::location(0)]] float3 Position : POSITION0;
    //[[vk::location(1)]] float3 Color : COLOR0;
    //[[vk::location(2)]] float2 TexCoord : TEXCOORD0;
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

struct VSOutput
{
    float4 Pos : SV_POSITION;
    //[[vk::location(0)]] float3 Color : COLOR0;
    [[vk::location(1)]] float2 TexCoord : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    //output.Color = input.Color;
    //output.TexCoord = input.TexCoord;
    output.Pos = mul(ubo.projectionMatrix, mul(ubo.viewMatrix, mul(ubo.modelMatrix, float4(input.Position.xyz, 1.0))));
    return output;
}