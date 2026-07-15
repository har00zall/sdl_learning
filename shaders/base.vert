#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragNormal;

layout(set = 1, binding = 0) uniform UniformBufferObject {
    mat4 viewProjection;
    mat4 model[100];
} uniformBufferObject;

void main() {
    gl_Position = uniformBufferObject.viewProjection *  uniformBufferObject.model[gl_InstanceIndex] * vec4(inPosition, 1.0);

    fragNormal = inNormal;
}
