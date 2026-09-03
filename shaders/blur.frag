#version 460 core
out vec4 FragColor;
uniform vec2 uResolution;
uniform vec2 uDirection;
layout(binding = 0) uniform sampler2D uSource;

void main() {
    vec2 uv = gl_FragCoord.xy / uResolution;
    vec2 texel = uDirection / uResolution;

    float weights[5] = float[5](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    vec3 result = texture(uSource, uv).rgb * weights[0];
    for (int i = 1; i < 5; ++i) {
        result += texture(uSource, uv + texel * float(i)).rgb * weights[i];
        result += texture(uSource, uv - texel * float(i)).rgb * weights[i];
    }
    FragColor = vec4(result, 1.0);
}
