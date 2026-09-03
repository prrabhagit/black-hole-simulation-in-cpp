#version 460 core
out vec4 FragColor;
uniform vec2 uResolution;
uniform float uExposure;
uniform float uBloomStrength;
layout(binding = 0) uniform sampler2D uHDRColor;
layout(binding = 1) uniform sampler2D uBloom;

vec3 acesFilmic(vec3 x) {
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec2 uv = gl_FragCoord.xy / uResolution;
    vec3 hdr = texture(uHDRColor, uv).rgb;
    vec3 bloom = texture(uBloom, uv).rgb;

    vec3 combined = hdr + bloom * uBloomStrength;
    combined *= uExposure;

    vec3 tonemapped = acesFilmic(combined);
    vec3 gammaCorrected = pow(tonemapped, vec3(1.0 / 2.2));

    FragColor = vec4(gammaCorrected, 1.0);
}
