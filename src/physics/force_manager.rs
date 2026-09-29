use bevy::prelude::Resource;

use super::Force;

/// The world's list of named forces, created from the "Add World Force"
/// dialog and offered to objects in the "Modify Forces" dialog.
#[derive(Resource, Default, Debug)]
pub struct ForceManager {
    forces: Vec<Force>,
}

impl ForceManager {
    pub fn forces(&self) -> &[Force] {
        &self.forces
    }

    pub fn by_name(&self, name: &str) -> Option<&Force> {
        self.forces.iter().find(|f| f.name == name)
    }

    /// Adds a force. Returns `false` (and leaves the list alone) if a force
    /// with that name already exists.
    pub fn add(&mut self, force: Force) -> bool {
        if self.by_name(&force.name).is_some() {
            return false;
        }
        self.forces.push(force);
        true
    }

    pub fn remove(&mut self, name: &str) -> Option<Force> {
        let index = self.forces.iter().position(|f| f.name == name)?;
        Some(self.forces.remove(index))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use bevy::math::DVec3;

    #[test]
    fn names_are_unique() {
        let mut manager = ForceManager::default();
        assert!(manager.add(Force::new("gravity", DVec3::NEG_Y, 0.0)));
        assert!(!manager.add(Force::new("gravity", DVec3::Y, 0.0)));
        assert_eq!(manager.forces().len(), 1);
        assert!(manager.remove("gravity").is_some());
        assert!(manager.remove("gravity").is_none());
    }
}
