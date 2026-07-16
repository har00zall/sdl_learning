#version 450
layout(location = 0) in vec3 v_fragPos;
layout(location = 1) in vec3 v_normal;

layout(std140, set = 3, binding = 0) uniform FregmentUniformData {
    vec3 viewPosition;
};

layout(location = 0) out vec4 outColor;


void main() {
    // Simple directional light simulation for now
    float lightIntensity = 1.5;
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 viewDir = normalize(viewPosition - v_fragPos);
    vec3 lightColor = lightIntensity * vec3(0.85, 0.85, 1);

    vec3 ambient = 0.1 * lightColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 0.75);
    vec3 specular = spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * vec3(0.1, 0.3, 0.4);
    outColor = vec4(result, 1.0);
}
