//! Text anchored to points in the world (axis names, manipulator values).
//!
//! Systems queue labels each frame; [`place_labels`] projects them to screen
//! positions and drives a pool of UI text nodes.

use bevy::prelude::*;

use crate::view::MainCamera;

pub struct Label {
    pub position: Vec3,
    pub text: String,
    pub color: Color,
}

#[derive(Resource, Default)]
pub struct WorldLabels {
    queue: Vec<Label>,
}

impl WorldLabels {
    pub fn add(&mut self, position: Vec3, text: impl Into<String>, color: Color) {
        self.queue.push(Label { position, text: text.into(), color });
    }
}

#[derive(Component)]
pub struct LabelNode;

#[derive(SystemSet, Debug, Clone, PartialEq, Eq, Hash)]
pub enum LabelSet {
    /// Systems that queue labels.
    Produce,
    /// Projects the queue onto the screen.
    Place,
}

/// Fixed order for label systems within `Update`.
pub fn configure(app: &mut App) {
    app.init_resource::<WorldLabels>()
        .configure_sets(Update, LabelSet::Place.after(LabelSet::Produce))
        .add_systems(Update, place_labels.in_set(LabelSet::Place));
}

const FONT_SIZE: f32 = 16.0;

fn place_labels(
    mut commands: Commands,
    mut labels: ResMut<WorldLabels>,
    camera: Query<(&Camera, &Transform), With<MainCamera>>,
    mut nodes: Query<(Entity, &mut Node, &mut Text, &mut TextColor, &mut Visibility), With<LabelNode>>,
    ui_camera: Query<Entity, With<MainCamera>>,
) {
    let queued = std::mem::take(&mut labels.queue);
    let projected: Vec<(Vec2, &Label)> = camera
        .get_single()
        .map(|(camera, transform)| {
            let global = GlobalTransform::from(*transform);
            queued
                .iter()
                .filter_map(|l| camera.world_to_viewport(&global, l.position).ok().map(|p| (p, l)))
                .collect()
        })
        .unwrap_or_default();

    let mut pool = nodes.iter_mut();
    for (screen, label) in &projected {
        if let Some((_, mut node, mut text, mut color, mut visibility)) = pool.next() {
            node.left = Val::Px(screen.x);
            node.top = Val::Px(screen.y - FONT_SIZE / 2.0);
            if text.0 != label.text {
                text.0.clone_from(&label.text);
            }
            color.0 = label.color;
            *visibility = Visibility::Inherited;
        } else if let Ok(target) = ui_camera.get_single() {
            commands.spawn((
                LabelNode,
                Text::new(label.text.clone()),
                TextFont { font_size: FONT_SIZE, ..default() },
                TextColor(label.color),
                Node { position_type: PositionType::Absolute, left: Val::Px(screen.x), top: Val::Px(screen.y - FONT_SIZE / 2.0), ..default() },
                GlobalZIndex(-10),
                TargetCamera(target),
                PickingBehavior::IGNORE,
            ));
        }
    }
    for (_, _, _, _, mut visibility) in pool {
        *visibility = Visibility::Hidden;
    }
}
