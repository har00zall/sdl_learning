#version 450
layout(location = 0) in vec3 v_normal;
layout(location = 0) out vec4 outColor;

void main() {
    // Simple directional light simulation for now
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(v_normal), lightDir), 0.2);
    outColor = vec4(vec3(0.8, 0.3, 0.3) * diff, 1.0);
}
