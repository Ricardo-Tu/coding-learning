//#version 450
//
//layout(location = 0) in vec2 inPosition;
//layout(location = 1) in vec2 inTexcoord;
//
//layout(location = 0) out vec2 outTexcoord;
//
//layout(set = 0, binding = 0) uniform UniformBuffer {
//    mat4 project;
//    mat4 view;
//} ubo;
//
//layout(push_constant) uniform PushConstant {
//    mat4 model;
//} pc;
//
//void main() {
//    gl_Position = ubo.project * ubo.view * pc.model * vec4(inPosition, 0.0, 1.0);
//    outTexcoord = inTexcoord;
//}

#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 project;
} mvp;


layout(location = 0) in vec3 positions;
layout(location = 1) in vec3 colors;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;


void main()
{
    gl_Position = mvp.project * mvp.view * mvp.model * vec4(positions, 1.0);
    fragColor = colors;
    fragTexCoord = inTexCoord;
}