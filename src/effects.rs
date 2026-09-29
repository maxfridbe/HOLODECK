//! Visual feedback on objects: the selection blink and collision boxes.

use bevy::prelude::*;

use crate::objects::model::{ModelGeometry, ObjectSurface};
use crate::objects::pick::Selection;

pub struct EffectsPlugin;

impl Plugin for EffectsPlugin {
    fn build(&self, app: &mut App) {
        app.add_systems(Update, (blink_selection, draw_collision_boxes));
    }
}

const LOW: f32 = 1.0;
const HIGH: f32 = 254.0;
const RATE: f32 = 100.0;

/// A blinking red/green tint, one step of the original's flux animation.
#[derive(Default)]
struct Flux {
    value: f32,
    rising: bool,
}

impl Flux {
    fn step(&mut self, dt: f32) {
        if self.value < LOW {
            self.value = LOW;
            self.rising = true;
        }
        if self.rising {
            if self.value > HIGH {
                self.rising = false;
            }
            self.value += RATE * dt;
        } else {
            if self.value < LOW {
                self.rising = true;
            }
            self.value -= RATE * dt;
        }
    }

    fn color(&self) -> Color {
        let v = self.value.clamp(0.0, 255.0);
        Color::srgb((255.0 - v) / 255.0, v / 255.0, 0.0)
    }
}

/// Tints textured surfaces of the selected object; untextured triangles keep
/// their own vertex colours, as in the original.
fn blink_selection(
    time: Res<Time>,
    selection: Res<Selection>,
    mut flux: Local<Flux>,
    mut previous: Local<Option<Entity>>,
    children: Query<&Children>,
    surfaces: Query<(&ObjectSurface, &MeshMaterial3d<StandardMaterial>)>,
    mut materials: ResMut<Assets<StandardMaterial>>,
) {
    let mut set_tint = |entity: Entity, color: Color| {
        for child in children.iter_descendants(entity) {
            if let Ok((surface, material)) = surfaces.get(child) {
                if surface.textured {
                    if let Some(m) = materials.get_mut(&material.0) {
                        m.base_color = color;
                    }
                }
            }
        }
    };

    if *previous != selection.entity {
        if let Some(old) = previous.take() {
            set_tint(old, Color::WHITE);
        }
        *previous = selection.entity;
    }
    if let Some(entity) = selection.entity {
        flux.step(time.delta_secs());
        set_tint(entity, flux.color());
    }
}

/// Outlines each object's world-space bounding box; boxes that overlap
/// another object's turn red.
fn draw_collision_boxes(mut gizmos: Gizmos, objects: Query<(&Transform, &ModelGeometry, &Visibility)>) {
    let boxes: Vec<_> = objects
        .iter()
        .filter(|(.., v)| **v != Visibility::Hidden)
        .map(|(t, g, _)| g.world_bounds(t))
        .collect();

    for (i, b) in boxes.iter().enumerate() {
        let colliding = boxes.iter().enumerate().any(|(j, other)| i != j && b.intersects(other));
        let color = if colliding { Color::srgb(1.0, 0.0, 0.0) } else { Color::WHITE };
        gizmos.cuboid(Transform::from_translation(b.center()).with_scale((b.max - b.min).max(Vec3::splat(1e-3))), color);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn flux_oscillates_between_the_limits() {
        let mut flux = Flux::default();
        let mut min = f32::MAX;
        let mut max = f32::MIN;
        for _ in 0..1000 {
            flux.step(0.01);
            min = min.min(flux.value);
            max = max.max(flux.value);
        }
        assert!(min >= 0.0 && min < 10.0, "{min}");
        assert!(max > 250.0 && max < 260.0, "{max}");
    }

    #[test]
    fn tint_runs_from_red_to_green() {
        let low = Flux { value: 1.0, rising: true }.color().to_srgba();
        let high = Flux { value: 254.0, rising: false }.color().to_srgba();
        assert!(low.red > 0.9 && low.green < 0.1);
        assert!(high.green > 0.9 && high.red < 0.1);
    }
}
