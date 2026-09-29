//! Small math helpers shared across the game (the C++ `WMath` / `MVector3`
//! statics that had no direct glam equivalent).

use bevy::math::Vec3;
use std::f64::consts::FRAC_PI_2;

/// Half a turn, used to clamp camera pitch just short of straight up/down.
pub const HALF_PI: f64 = FRAC_PI_2;

/// Converts the camera's spherical angles into a direction of the given
/// length. `theta` is the yaw about +Y (0 looks down +Z) and `phi` is the
/// polar angle measured from +Y (so `HALF_PI` is level).
///
/// This is `MVector3::rotate(v, theta, phi)` from the C++ code, which only
/// ever used the magnitude of `v`.
pub fn spherical_direction(magnitude: f32, theta: f64, phi: f64) -> Vec3 {
    let mag = f64::from(magnitude);
    Vec3::new(
        (mag * phi.sin() * theta.sin()) as f32,
        (mag * phi.cos()) as f32,
        (mag * phi.sin() * theta.cos()) as f32,
    )
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn level_zero_yaw_looks_down_positive_z() {
        let v = spherical_direction(2.0, 0.0, HALF_PI);
        assert!((v - Vec3::new(0.0, 0.0, 2.0)).length() < 1e-6);
    }

    #[test]
    fn positive_phi_offset_looks_down() {
        // Camera code passes `phi + HALF_PI`; a positive camera phi tilts down.
        let v = spherical_direction(1.0, 0.0, 0.3 + HALF_PI);
        assert!(v.y < 0.0);
    }

    #[test]
    fn preserves_magnitude() {
        let v = spherical_direction(5.0, 1.234, 2.345);
        assert!((v.length() - 5.0).abs() < 1e-5);
    }
}
