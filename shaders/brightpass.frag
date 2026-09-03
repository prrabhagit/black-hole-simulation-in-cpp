#version 460 core
out vec4 FragColor;
uniform vec2 uResolution;
uniform float uBloomThreshold;
layout(binding = 0) uniform sampler2D uHDRColor;

void main() {
    vec2 uv = gl_FragCoord.xy / uResolution;
    vec3 color = texture(uHDRColor, uv).rgb;

    float brightness = max(color.r, max(color.g, color.b));
    float knee = uBloomThreshold * 0.5;
    float soft = clamp(brightness - uBloomThreshold + knee, 0.0, 2.0 * knee);
    soft = (soft * soft) / (4.0 * knee + 1e-5);
    float contribution = max(soft, brightness - uBloomThreshold);

    FragColor = vec4(color * (contribution / max(brightness, 1e-5)), 1.0);
}
