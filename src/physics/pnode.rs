use bevy::math::DVec3;

/// A force that can be applied to a [`PNode`].
#[derive(Clone, Debug, PartialEq)]
pub struct Force {
    pub name: String,
    /// True if the force only lasts for `duration` seconds.
    pub impulse: bool,
    /// How long an impulse lasts, in seconds.
    pub duration: f64,
    pub vector: DVec3,
    /// Scalar multiple for force fields (`f = mg`, etc).
    pub scalar_attrib: i32,
}

impl Force {
    /// Creates a force from the "Add World Force" dialog values. A non-zero
    /// duration makes it an impulse.
    pub fn new(name: impl Into<String>, vector: DVec3, duration: f64) -> Self {
        Self {
            name: name.into(),
            impulse: duration != 0.0,
            duration,
            vector,
            scalar_attrib: 1,
        }
    }
}

/// A monoparticle physics node: it only has a centre of mass.
#[derive(Clone, Debug)]
pub struct PNode {
    mass: f64,
    position: DVec3,
    velocity: DVec3,
    forces: Vec<Force>,
    net_force: DVec3,
}

impl PNode {
    pub fn new(mass: f64) -> Self {
        Self {
            mass,
            position: DVec3::ZERO,
            velocity: DVec3::ZERO,
            forces: Vec::new(),
            net_force: DVec3::ZERO,
        }
    }

    pub fn position(&self) -> DVec3 {
        self.position
    }

    pub fn set_position(&mut self, position: DVec3) {
        self.position = position;
    }

    pub fn velocity(&self) -> DVec3 {
        self.velocity
    }

    pub fn mass(&self) -> f64 {
        self.mass
    }

    pub fn forces(&self) -> &[Force] {
        &self.forces
    }

    pub fn add_force(&mut self, force: Force) {
        self.forces.push(force);
    }

    /// Removes every applied force with this name.
    pub fn remove_force(&mut self, name: &str) {
        self.forces.retain(|f| f.name != name);
    }

    /// Sums the active forces, ageing impulses by `delta_t` seconds and
    /// dropping the ones that have run out.
    pub fn update_forces(&mut self, delta_t: f64) {
        self.forces.retain(|f| !f.impulse || f.duration > 0.0);

        self.net_force = DVec3::ZERO;
        for force in &mut self.forces {
            if force.impulse {
                force.duration -= delta_t;
            }
            self.net_force += force.vector;
        }
    }

    /// `v = v0 + a*t`, with `a = f/m`.
    pub fn update_velocity(&mut self, delta_t: f64) {
        self.velocity += self.net_force * (delta_t / self.mass);
    }

    /// `x = x0 + v*t`.
    pub fn update_position(&mut self, delta_t: f64) {
        self.position += self.velocity * delta_t;
    }

    /// Perfectly elastic collision with another node, applied per axis.
    ///
    /// A massless `other` is an immovable wall: this node just bounces off.
    /// (The C++ scaled the second body's *own* velocity, which does not
    /// conserve momentum; this uses the full elastic-collision formulas.)
    pub fn collide(&mut self, other: &mut PNode) {
        if other.mass == 0.0 {
            self.velocity = -self.velocity;
            return;
        }
        let (m1, m2) = (self.mass, other.mass);
        let (v1, v2) = (self.velocity, other.velocity);
        let total = m1 + m2;
        self.velocity = (v1 * (m1 - m2) + v2 * (2.0 * m2)) / total;
        other.velocity = (v2 * (m2 - m1) + v1 * (2.0 * m1)) / total;
    }

    /// Collision against a stationary body of the given mass that has no
    /// node of its own.
    pub fn collide_with_mass(&mut self, mass: f64) {
        if mass == 0.0 {
            self.velocity = -self.velocity;
            return;
        }
        self.velocity = self.velocity * (self.mass - mass) / (self.mass + mass);
    }

    /// Stops the node.
    pub fn reset(&mut self) {
        self.velocity = DVec3::ZERO;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn constant_force_accelerates_uniformly() {
        let mut node = PNode::new(2.0);
        node.add_force(Force::new("push", DVec3::new(4.0, 0.0, 0.0), 0.0));
        for _ in 0..10 {
            node.update_forces(0.1);
            node.update_velocity(0.1);
            node.update_position(0.1);
        }
        // a = 2 m/s^2 for 1s -> v = 2
        assert!((node.velocity().x - 2.0).abs() < 1e-9);
        assert!(node.position().x > 0.0);
    }

    #[test]
    fn impulse_expires_after_its_duration() {
        let mut node = PNode::new(1.0);
        node.add_force(Force::new("kick", DVec3::Y, 0.25));
        for _ in 0..10 {
            node.update_forces(0.1);
            node.update_velocity(0.1);
        }
        assert!(node.forces().is_empty());
        // Applied for three updates (0.25 -> 0.15 -> 0.05 -> -0.05).
        assert!((node.velocity().y - 0.3).abs() < 1e-9);
    }

    #[test]
    fn massless_collider_reflects_velocity() {
        let mut node = PNode::new(5.0);
        node.add_force(Force::new("go", DVec3::X, 0.0));
        node.update_forces(1.0);
        node.update_velocity(1.0);
        let mut wall = PNode::new(0.0);
        node.collide(&mut wall);
        assert!(node.velocity().x < 0.0);
    }

    #[test]
    fn equal_masses_swap_velocity_in_head_on_collision() {
        let mut a = PNode::new(1.0);
        let mut b = PNode::new(1.0);
        a.velocity = DVec3::new(3.0, 0.0, 0.0);
        a.collide(&mut b);
        assert_eq!(a.velocity(), DVec3::ZERO);
        assert_eq!(b.velocity(), DVec3::new(3.0, 0.0, 0.0));
    }

    #[test]
    fn collision_conserves_momentum_and_energy() {
        let mut a = PNode::new(3.0);
        let mut b = PNode::new(1.0);
        a.velocity = DVec3::new(2.0, 1.0, 0.0);
        b.velocity = DVec3::new(-1.0, 0.5, 0.0);
        let momentum = |a: &PNode, b: &PNode| a.velocity * a.mass + b.velocity * b.mass;
        let energy = |a: &PNode, b: &PNode| {
            0.5 * a.mass * a.velocity.length_squared() + 0.5 * b.mass * b.velocity.length_squared()
        };
        let (p0, e0) = (momentum(&a, &b), energy(&a, &b));
        a.collide(&mut b);
        assert!((momentum(&a, &b) - p0).length() < 1e-9);
        assert!((energy(&a, &b) - e0).abs() < 1e-9);
    }

    #[test]
    fn collision_with_stationary_mass_matches_full_collision() {
        let mut a = PNode::new(3.0);
        a.velocity = DVec3::new(2.0, 0.0, 0.0);
        let mut b = PNode::new(1.0);
        let mut a2 = a.clone();
        a.collide(&mut b);
        a2.collide_with_mass(1.0);
        assert!((a.velocity() - a2.velocity()).length() < 1e-12);
    }

    #[test]
    fn remove_force_by_name() {
        let mut node = PNode::new(1.0);
        node.add_force(Force::new("a", DVec3::X, 0.0));
        node.add_force(Force::new("b", DVec3::Y, 0.0));
        node.remove_force("a");
        assert_eq!(node.forces().len(), 1);
        assert_eq!(node.forces()[0].name, "b");
    }
}
