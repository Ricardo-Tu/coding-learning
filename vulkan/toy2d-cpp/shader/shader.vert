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
layout(location = 0) in vec3 positions;
layout(location = 1) in vec3 colors;
layout(location = 0) out vec3 fragColor;
// vec2 positions[3] = vec2[](
//     vec2(0.0, -0.5),
//     vec2(0.5, 0.5),
//     vec2(-0.5, 0.5)
// );

// vec3 color[3] = vec3[](
//     vec3(1.0, 0.0, 0.0),
//     vec3(0.0, 1.0, 0.0),
//     vec3(0.0, 0.0, 1.0)
// );

void main()
{
    fragColor = colors;
    gl_Position = vec4(positions, 1.0);
    // fragColor = inColor;
    // gl_Position = vec4(inPosition, 0.0, 1.0);
}