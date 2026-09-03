// RK4 integration of the photon Binet equation, with disk-plane crossing
// detection and a weak-field analytic fast path for large impact parameter.
// Depends on schwarzschild.glsl being included first.

struct GeodesicResult {
    bool  captured;
    bool  escaped;
    bool  hitDisk;
    vec3  exitDirection;
    vec3  diskHitPos;
    float diskHitRadius;
    vec3  diskHitTraceDir;
    float closestApproachR;
    float totalPhi;
};

vec2 geodesicRHS(vec2 y, float M) {
    return vec2(y.y, photonUDoublePrime(y.x, M));
}

vec3 reconstructPosition(OrbitalPlane plane, vec3 bhPos, float r, float phi) {
    return bhPos + r * (cos(phi) * plane.eR + sin(phi) * plane.ePhi);
}

vec3 applyWeakFieldDeflection(vec3 rayDir, OrbitalPlane plane, float M) {
    float alpha = 4.0 * M / plane.b;
    vec3 radialComponent = dot(rayDir, plane.eR) * plane.eR;
    vec3 tangentialComponent = rayDir - radialComponent;
    float tangentialLen = length(tangentialComponent);
    if (tangentialLen < 1e-6) return rayDir;
    return normalize(rayDir - alpha * tangentialLen * plane.eR * sign(dot(rayDir, plane.eR)));
}

GeodesicResult traceGeodesic(
    vec3 rayOrigin, vec3 rayDir, vec3 bhPos,
    float M, float rs,
    vec3 diskNormal, float diskInner, float diskOuter,
    int maxSteps, float baseStepPhi,
    float weakFieldBThreshold
) {
    GeodesicResult result;
    result.captured = false;
    result.escaped = false;
    result.hitDisk = false;
    result.closestApproachR = 1e9;
    result.totalPhi = 0.0;

    OrbitalPlane plane;
    if (!computeOrbitalPlane(rayOrigin, rayDir, bhPos, plane)) {
        vec3 toCam = rayOrigin - bhPos;
        float movingInward = dot(rayDir, normalize(toCam));
        result.closestApproachR = length(toCam);
        if (movingInward < 0.0) {
            result.captured = true;
        } else {
            result.escaped = true;
            result.exitDirection = rayDir;
        }
        return result;
    }

    // --- Weak-field fast path (Phase 14) ---
    if (plane.b > weakFieldBThreshold) {
        float denom = dot(rayDir, diskNormal);
        if (abs(denom) > 1e-6) {
            float t = dot(bhPos - rayOrigin, diskNormal) / denom;
            if (t > 0.0) {
                vec3 hitPos = rayOrigin + t * rayDir;
                float hitR = length(hitPos - bhPos);
                if (hitR >= diskInner && hitR <= diskOuter) {
                    result.hitDisk = true;
                    result.diskHitPos = hitPos;
                    result.diskHitRadius = hitR;
                    result.diskHitTraceDir = rayDir;
                    return result;
                }
            }
        }
        result.escaped = true;
        result.exitDirection = applyWeakFieldDeflection(rayDir, plane, M);
        return result;
    }
    // --- End fast path ---

    float u = plane.u0, uPrime = plane.uPrime0, phi = 0.0;
    const float uHorizon = 1.0 / rs;
    const float uMin = 1.0 / (1000.0 * rs);
    const float uPhotonSphere = 1.0 / (3.0 * M);
    result.closestApproachR = 1.0 / u;

    vec3 prevPos = reconstructPosition(plane, bhPos, 1.0 / u, phi);
    float prevSignedDist = dot(prevPos - bhPos, diskNormal);

    for (int i = 0; i < maxSteps; ++i) {
        float proximity = abs(u - uPhotonSphere) / uPhotonSphere;
        float h = baseStepPhi * clamp(4.0 * proximity + 0.05, 0.05, 1.0);

        vec2 y = vec2(u, uPrime);
        vec2 k1 = geodesicRHS(y, M);
        vec2 k2 = geodesicRHS(y + 0.5 * h * k1, M);
        vec2 k3 = geodesicRHS(y + 0.5 * h * k2, M);
        vec2 k4 = geodesicRHS(y + h * k3, M);
        y = y + (h / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
        u = y.x; uPrime = y.y; phi += h;

        float r = 1.0 / max(u, 1e-8);
        result.closestApproachR = min(result.closestApproachR, r);

        vec3 currPos = reconstructPosition(plane, bhPos, r, phi);
        float currSignedDist = dot(currPos - bhPos, diskNormal);

        if (prevSignedDist * currSignedDist < 0.0) {
            float t = prevSignedDist / (prevSignedDist - currSignedDist);
            vec3 hitPos = mix(prevPos, currPos, t);
            float hitRadius = length(hitPos - bhPos);

            if (hitRadius >= diskInner && hitRadius <= diskOuter) {
                result.hitDisk = true;
                result.diskHitPos = hitPos;
                result.diskHitRadius = hitRadius;
                result.diskHitTraceDir = normalize(currPos - prevPos);
                result.totalPhi = phi;
                return result;
            }
        }

        prevPos = currPos;
        prevSignedDist = currSignedDist;

        if (u >= uHorizon) { result.captured = true; result.totalPhi = phi; return result; }
        if (u <= uMin && uPrime < 0.0) break;
    }

    result.escaped = true;
    result.totalPhi = phi;
    float r = 1.0 / max(u, 1e-8);
    float drdphi = -uPrime * r * r;
    vec3 dPosDPhi =
        (drdphi * cos(phi) - r * sin(phi)) * plane.eR +
        (drdphi * sin(phi) + r * cos(phi)) * plane.ePhi;
    result.exitDirection = normalize(dPosDPhi);
    return result;
}
