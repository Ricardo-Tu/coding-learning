#version 450

layout(std140, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 project;
}ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) uniform mat4 u_projection_matrix;

layout(location = 0) out vec4 v_color;



void main() {
    // gl_Position = ubo.project * vec4(position, 1.0);
    v_color = color;
    // gl_Position = u_projection_matrix * vec4(position, 1.0);
    // gl_Position = vec4(position, 1.0);
    gl_Position = ubo.project * ubo.view * ubo.model * vec4(position, 1.0);
}