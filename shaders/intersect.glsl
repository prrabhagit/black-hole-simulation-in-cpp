// Analytic ray-sphere intersection. Returns nearest positive t, or -1 if no hit.
float intersectSphere(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float radius) {
    vec3 oc = rayOrigin - sphereCenter;
    float b = dot(oc, rayDir);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - c;
    if (discriminant < 0.0) return -1.0;
    float sqrtDisc = sqrt(discriminant);
    float t0 = -b - sqrtDisc;
    float t1 = -b + sqrtDisc;
    if (t0 > 0.0) return t0;
    if (t1 > 0.0) return t1;
    return -1.0;
}
