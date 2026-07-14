#version 450
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

// Set 0, Binding 0 for our Camera Uniform Buffer
layout(set = 1, binding = 0) uniform Camera {
    mat4 viewProj;
} cam;

layout(location = 0) out vec3 v_normal;

void main() {
    v_normal = a_normal;
    gl_Position = cam.viewProj * vec4(a_position, 1.0);
}
