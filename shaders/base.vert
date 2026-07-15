#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragNormal;

layout(std430, set = 0, binding = 0) readonly buffer StorageBufferObject {
    mat4 viewProjection;
    mat4 model[];
} storageBufferObject;

void main() {
    gl_Position = storageBufferObject.viewProjection *  storageBufferObject.model[gl_InstanceIndex] * vec4(inPosition, 1.0);

    fragNormal = inNormal;
}
