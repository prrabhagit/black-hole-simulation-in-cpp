// Schwarzschild null-geodesic formulas. Geometrized units: G = c = 1, rs = 2M.

float schwarzschildRadiusFromMass(float M) { return 2.0 * M; }
float photonSphereRadiusFromMass(float M) { return 1.5 * schwarzschildRadiusFromMass(M); }
float criticalImpactParameter(float M) { return 3.0 * sqrt(3.0) * M; }

// Binet-form second derivative: u'' = 3Mu^2 - u, u = 1/r.
float photonUDoublePrime(float u, float M) {
    return 3.0 * M * u * u - u;
}

struct OrbitalPlane {
    vec3  eR;
    vec3  ePhi;
    float r0;
    float u0;
    float uPrime0;
    float b;
};

bool computeOrbitalPlane(vec3 rayOrigin, vec3 rayDir, vec3 bhPos, out OrbitalPlane plane) {
    vec3 P = rayOrigin - bhPos;
    float r0 = length(P);

    vec3 Lvec = cross(P, rayDir);
    float LvecLen = length(Lvec);

    const float kEps = 1e-6;
    if (LvecLen < kEps) return false;

    plane.eR   = P / r0;
    vec3 n     = Lvec / LvecLen;
    plane.ePhi = cross(n, plane.eR);

    plane.r0 = r0;
    plane.u0 = 1.0 / r0;

    float dRadial     = dot(rayDir, plane.eR);
    float dTangential = dot(rayDir, plane.ePhi);

    plane.uPrime0 = -plane.u0 * (dRadial / dTangential);
    plane.b = r0 * abs(dTangential);

    return true;
}
