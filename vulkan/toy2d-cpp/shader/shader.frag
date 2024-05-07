// #version 450
// layout(location = 0) in vec3 fragColor; 
// layout(location = 0) out vec4 outColor;

// void main()
// {
//     outColor = vec4(fragColor, 1.0);
// }
#version 450

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, vec2(fragTexCoord));
}
