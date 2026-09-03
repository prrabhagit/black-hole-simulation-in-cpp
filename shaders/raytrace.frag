#version 460 core

out vec4 FragColor;

layout(std140, binding = 0) uniform CameraBlock {
    vec3  camPosition;      float _pad0;
    vec3  camForward;       float _pad1;
    vec3  camRight;         float _pad2;
    vec3  camUp;            float camTanHalfFovY;
    float camAspect;        vec3  _pad3;
};

layout(std140, binding = 1) uniform BlackHoleBlock {
    vec3  bhPosition;       float bhMass;
    float bhSchwarzschildRadius; float bhPhotonSphereRadius;
    float _bhPad0, _bhPad1;
};

layout(std140, binding = 2) uniform DiskBlock {
    vec3  diskNormal;       float diskInnerRadius;
    float diskOuterRadius;  float diskPeakTempKelvin;
    float _diskPad0, _diskPad1;
};

uniform vec2  uResolution;
uniform int   uMaxSteps;
uniform float uBaseStepPhi;
uniform float uWeakFieldBThreshold;
uniform int   uDebugMode;      // 0 normal, 1 photon sphere overlay, 2 image order overlay
uniform bool  uProgradeDisk;
layout(binding = 0) uniform sampler2D uEnvironmentMap;

#include "intersect.glsl"
#include "schwarzschild.glsl"
#include "geodesic.glsl"

vec3 generateRayDirection(vec2 fragCoord) {
    vec2 uv = fragCoord / uResolution;
    float ndcX = (2.0 * uv.x - 1.0) * camAspect * camTanHalfFovY;
    float ndcY = (2.0 * (1.0 - uv.y) - 1.0) * camTanHalfFovY;
    return normalize(ndcX * camRight + ndcY * camUp + camForward);
}

vec2 directionToEquirectUV(vec3 dir) {
    float theta = atan(dir.z, dir.x);
    float phi   = asin(clamp(dir.y, -1.0, 1.0));
    return vec2(theta / (2.0 * 3.14159265359) + 0.5, phi / 3.14159265359 + 0.5);
}

vec3 sampleEnvironment(vec3 dir) {
    return texture(uEnvironmentMap, directionToEquirectUV(normalize(dir))).rgb;
}

// --- Accretion disk shading ---

float diskTemperature(float r) {
    if (r <= diskInnerRadius) return 0.0;
    float s = sqrt(diskInnerRadius / r);
    float profile = pow(s * s * s * s * s * s * (1.0 - s), 0.25);
    const float kPeakNormalization = 0.4879;
    return diskPeakTempKelvin * profile / kPeakNormalization;
}

vec3 blackbodyColor(float tempKelvin) {
    float t = clamp(tempKelvin, 1000.0, 40000.0) / 100.0;
    vec3 color;

    color.r = t <= 66.0 ? 1.0 :
        clamp(1.29293618606 * pow(t - 60.0, -0.1332047592), 0.0, 1.0);

    if (t <= 66.0) {
        color.g = clamp(0.39008157876 * log(t) - 0.63184144379, 0.0, 1.0);
    } else {
        color.g = clamp(1.12989086089 * pow(t - 60.0, -0.0755148492), 0.0, 1.0);
    }

    if (t >= 66.0) {
        color.b = 1.0;
    } else if (t <= 19.0) {
        color.b = 0.0;
    } else {
        color.b = clamp(0.54320678911 * log(t - 10.0) - 1.19625408914, 0.0, 1.0);
    }

    return color;
}

vec3 diskEmission(float tempKelvin, float referenceTempKelvin) {
    vec3 hue = blackbodyColor(tempKelvin);
    float relativeIntensity = pow(max(tempKelvin, 1.0) / referenceTempKelvin, 4.0);
    return hue * relativeIntensity;
}

float orbitalSpeed(float r, float M, float rs) {
    return sqrt(M / max(r - rs, 1e-4));
}

float dopplerFactor(vec3 hitPos, vec3 bhPos, vec3 diskNorm, vec3 traceDir,
                     float r, float M, float rs, bool prograde) {
    vec3 radialDir = normalize(hitPos - bhPos);
    vec3 velocityDir = normalize(cross(diskNorm, radialDir)) * (prograde ? 1.0 : -1.0);

    vec3 photonPropagationDir = -traceDir;
    float cosTheta = dot(photonPropagationDir, velocityDir);

    float beta = clamp(orbitalSpeed(r, M, rs), 0.0, 0.999);
    float gamma = 1.0 / sqrt(1.0 - beta * beta);
    return 1.0 / (gamma * (1.0 - beta * cosTheta));
}

float gravitationalRedshiftFactor(float rEmit, float rObs, float rs) {
    float gEmit = 1.0 - rs / rEmit;
    float gObs  = 1.0 - rs / rObs;
    return sqrt(max(gEmit, 0.0) / max(gObs, 1e-6));
}

vec3 imageOrderColor(int order) {
    vec3 palette[6] = vec3[6](
        vec3(1.0, 1.0, 1.0),
        vec3(1.0, 0.5, 0.2),
        vec3(0.3, 1.0, 0.4),
        vec3(0.3, 0.6, 1.0),
        vec3(1.0, 0.3, 1.0),
        vec3(1.0, 1.0, 0.3)
    );
    return palette[clamp(order, 0, 5)];
}

void main() {
    vec3 rayDir = generateRayDirection(gl_FragCoord.xy);

    GeodesicResult geo = traceGeodesic(
        camPosition, rayDir, bhPosition,
        bhMass, bhSchwarzschildRadius,
        diskNormal, diskInnerRadius, diskOuterRadius,
        uMaxSteps, uBaseStepPhi, uWeakFieldBThreshold
    );

    float rObs = length(camPosition - bhPosition);

    vec3 color;
    if (geo.captured) {
        color = vec3(0.0);
    } else if (geo.hitDisk) {
        float tempLocal = diskTemperature(geo.diskHitRadius);
        float D = dopplerFactor(geo.diskHitPos, bhPosition, diskNormal,
                                 geo.diskHitTraceDir, geo.diskHitRadius,
                                 bhMass, bhSchwarzschildRadius, uProgradeDisk);
        float gGrav = gravitationalRedshiftFactor(geo.diskHitRadius, rObs, bhSchwarzschildRadius);
        float gTotal = D * gGrav;
        float tempObserved = gTotal * tempLocal;
        color = diskEmission(tempObserved, diskPeakTempKelvin);
    } else {
        float gEnv = 1.0 / sqrt(max(1.0 - bhSchwarzschildRadius / rObs, 1e-6));
        color = sampleEnvironment(geo.exitDirection) * pow(gEnv, 4.0);
    }

    if (uDebugMode == 1 && !geo.captured) {
        float band = 0.03 * bhPhotonSphereRadius;
        if (abs(geo.closestApproachR - bhPhotonSphereRadius) < band) {
            color = mix(color, vec3(0.1, 1.0, 0.9), 0.85);
        }
    } else if (uDebugMode == 2 && !geo.captured) {
        int order = int(floor(geo.totalPhi / 3.14159265359));
        color = mix(color, imageOrderColor(order), 0.55);
    }

    // Linear HDR output -- NO tonemap/gamma here. That happens once, in
    // composite.frag, after bloom (Phase 13).
    FragColor = vec4(color, 1.0);
}
