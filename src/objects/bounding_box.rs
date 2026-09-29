use bevy::prelude::*;

/// An axis-aligned bounding box.
///
/// Transforming a box (see [`BoundingBox::transformed`]) rotates its eight
/// corners and re-fits an axis-aligned box around them, which is what makes
/// rotated objects collide correctly.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct BoundingBox {
    pub min: Vec3,
    pub max: Vec3,
}

impl Default for BoundingBox {
    fn default() -> Self {
        Self { min: Vec3::ZERO, max: Vec3::ZERO }
    }
}

impl BoundingBox {
    pub fn new(min: Vec3, max: Vec3) -> Self {
        Self { min, max }
    }

    /// The smallest box containing all points, or `None` if there are none.
    pub fn from_points(points: impl IntoIterator<Item = Vec3>) -> Option<Self> {
        let mut points = points.into_iter();
        let first = points.next()?;
        // Seed with the first point rather than a sentinel: seeding `max`
        // with `f32::MIN_POSITIVE` (the C++ `FLT_MIN`) clamps any box that
        // lies in negative space to the origin.
        Some(points.fold(Self::new(first, first), |b, p| Self::new(b.min.min(p), b.max.max(p))))
    }

    pub fn center(&self) -> Vec3 {
        (self.min + self.max) * 0.5
    }

    pub fn half_extents(&self) -> Vec3 {
        (self.max - self.min) * 0.5
    }

    pub fn corners(&self) -> [Vec3; 8] {
        let (lo, hi) = (self.min, self.max);
        [
            Vec3::new(lo.x, lo.y, lo.z),
            Vec3::new(lo.x, lo.y, hi.z),
            Vec3::new(hi.x, lo.y, hi.z),
            Vec3::new(hi.x, lo.y, lo.z),
            Vec3::new(lo.x, hi.y, lo.z),
            Vec3::new(lo.x, hi.y, hi.z),
            Vec3::new(hi.x, hi.y, hi.z),
            Vec3::new(hi.x, hi.y, lo.z),
        ]
    }

    /// The world-space box of this local-space box under `transform`
    /// (scale, then rotation, then translation).
    pub fn transformed(&self, transform: &Transform) -> Self {
        Self::from_points(self.corners().map(|c| transform.transform_point(c)))
            .expect("a box always has eight corners")
    }

    /// True if the boxes overlap or touch on all three axes.
    pub fn intersects(&self, other: &Self) -> bool {
        self.max.cmpge(other.min).all() && self.min.cmple(other.max).all()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::f32::consts::FRAC_PI_2;

    fn unit_box() -> BoundingBox {
        BoundingBox::new(Vec3::splat(-1.0), Vec3::splat(1.0))
    }

    #[test]
    fn box_entirely_in_negative_space_keeps_its_real_max() {
        // Regression: the C++ seeded max with FLT_MIN (a tiny *positive*
        // number), so this box came back with max = 0 and its manipulator
        // "grabbed the edge of the box at the origin".
        let b = BoundingBox::from_points([Vec3::new(-9.0, -8.0, -7.0), Vec3::new(-5.0, -4.0, -3.0)]).unwrap();
        assert_eq!(b.max, Vec3::new(-5.0, -4.0, -3.0));
        assert_eq!(b.center(), Vec3::new(-7.0, -6.0, -5.0));
    }

    #[test]
    fn translating_into_negative_space_moves_the_box_with_it() {
        let t = Transform::from_translation(Vec3::new(-20.0, -20.0, -20.0));
        let b = unit_box().transformed(&t);
        assert_eq!(b.min, Vec3::splat(-21.0));
        assert_eq!(b.max, Vec3::splat(-19.0));
    }

    #[test]
    fn scale_rotation_translation_apply_in_that_order() {
        let t = Transform {
            translation: Vec3::new(10.0, 0.0, 0.0),
            rotation: Quat::from_rotation_z(FRAC_PI_2),
            scale: Vec3::new(4.0, 1.0, 1.0),
        };
        // Long axis (x, length 4 each side) is turned onto y.
        let b = unit_box().transformed(&t);
        assert!((b.half_extents() - Vec3::new(1.0, 4.0, 1.0)).length() < 1e-5);
        assert!((b.center() - Vec3::new(10.0, 0.0, 0.0)).length() < 1e-5);
    }

    #[test]
    fn rotated_box_grows_to_fit_its_corners() {
        let t = Transform::from_rotation(Quat::from_rotation_y(std::f32::consts::FRAC_PI_4));
        let b = unit_box().transformed(&t);
        assert!((b.half_extents().x - 2.0_f32.sqrt()).abs() < 1e-5);
        assert!((b.half_extents().y - 1.0).abs() < 1e-5);
    }

    #[test]
    fn intersection_includes_touching_and_excludes_gaps() {
        let a = unit_box();
        let touching = BoundingBox::new(Vec3::new(1.0, -1.0, -1.0), Vec3::new(3.0, 1.0, 1.0));
        let apart = BoundingBox::new(Vec3::new(1.5, -1.0, -1.0), Vec3::new(3.0, 1.0, 1.0));
        assert!(a.intersects(&touching));
        assert!(!a.intersects(&apart));
        assert!(touching.intersects(&a));
    }

    #[test]
    fn empty_input_has_no_box() {
        assert!(BoundingBox::from_points(std::iter::empty()).is_none());
    }
}
