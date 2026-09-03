#pragma once
#include <cmath>
#include <algorithm>

namespace bh::physics::schwarzschild {

[[nodiscard]] inline constexpr double schwarzschildRadius(double M) noexcept {
    return 2.0 * M;
}

[[nodiscard]] inline constexpr double photonSphereRadius(double M) noexcept {
    return 1.5 * schwarzschildRadius(M);
}

[[nodiscard]] inline double criticalImpactParameter(double M) noexcept {
    return 3.0 * std::sqrt(3.0) * M;
}

[[nodiscard]] inline double effectivePotential(double r, double L, double M) noexcept {
    if (r <= 0.0) return 0.0;
    const double rs = schwarzschildRadius(M);
    return (1.0 - rs / r) * (L * L) / (r * r);
}

[[nodiscard]] inline constexpr double uDoublePrime(double u, double M) noexcept {
    return 3.0 * M * u * u - u;
}

/// Exact angular shadow radius (radians) for a static observer at
/// coordinate radius r_o (Phase 8).
[[nodiscard]] inline double exactShadowAngularRadius(double r_o, double M) noexcept {
    const double rs = schwarzschildRadius(M);
    const double bc = criticalImpactParameter(M);
    const double sin2psi = (bc * bc / (r_o * r_o)) * (1.0 - rs / r_o);
    return std::asin(std::sqrt(std::clamp(sin2psi, 0.0, 1.0)));
}

/// First-order weak-field deflection angle (Phase 9), valid for b >> M.
[[nodiscard]] inline constexpr double weakFieldDeflection(double b, double M) noexcept {
    return 4.0 * M / b;
}

} // namespace bh::physics::schwarzschild
