#version 450 core

#pragma stage:all

struct VertexOutput
{
    vec2 texCoord0;
    vec2 texCoord1;
    vec3 normal;
};

#pragma stage:vertex

// inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord0;
layout(location = 3) in vec2 a_TexCoord1;
layout(location = 4) in vec2 a_Color;

// outputs
layout(location = 0) out VertexOutput vertexOutput;

invariant gl_Position;

// uniforms
layout(binding = 0) uniform UniformBufferObject {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
} ubo;

void main()
{
    vertexOutput.texCoord0 = a_TexCoord0;
    vertexOutput.normal = a_Normal;
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(a_Position, 1.0);
}

#pragma stage:fragment

// inputs
layout(location = 0) in VertexOutput vertexInput;

// outputs
layout(location = 0) out vec4 outColor;

// uniforms
layout(binding = 0, set = 1) uniform MaterialBufferObject {
    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor[5];
    vec3 emissiveFactor;
    uint alphaMode;
    float alphaCutoff;
    bool doubleSided;
} materialUbo;

#ifdef HAS_BASE_COLOR_TEXTURE
layout(binding = 1, set = 1) uniform sampler2D baseColorTexSampler;
#endif

#ifdef HAS_METALLIC_ROUGHNESS_TEXTURE
layout(binding = 2, set = 1) uniform sampler2D metallicRoughnessSampler;
#endif

#ifdef HAS_NORMAL_TEXTURE
layout(binding = 3, set = 1) uniform sampler2D normalSampler;
#endif

#ifdef HAS_OCCLUSION_TEXTURE
layout(binding = 4, set = 1) uniform sampler2D occlusionSampler;
#endif

#ifdef HAS_EMISSIVE_TEXTURE
layout(binding = 5, set = 1) uniform sampler2D emissiveSampler;
#endif

vec4 getBaseColor(vec2 texCoord)
{
#ifdef HAS_BASE_COLOR_TEXTURE
    return texture(baseColorTexSampler, texCoord);
#else
    return vec4(1.0f);
#endif
}

void main() {
    float roughnessFactor = materialUbo.roughnessFactor[0];
    //outColor = vec4(vertexInput.texCoord0, 1.0f, 1.0f);
    outColor = getBaseColor(vertexInput.texCoord0);// * materialUbo.alphaCutoff;
}