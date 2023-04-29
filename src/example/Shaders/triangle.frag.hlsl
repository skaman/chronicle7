// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

struct VSOutput
{
    //[[vk::location(0)]] float3 Color : COLOR0;
    [[vk::location(1)]] float2 TexCoord : TEXCOORD0;
};

SamplerState texSampler : register(s1, space0);
Texture2D<float4> tex : register(t1, space0);

float4 main(in VSOutput input) : SV_TARGET0
{
    return tex.Sample(texSampler, input.TexCoord);
}