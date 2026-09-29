//! A scripting hook for smoke tests and screenshots.
//!
//! Set `HOLODECK_SCRIPT` to a `;`-separated list of steps, run in order, one
//! per frame unless they wait:
//!
//! ```text
//! scene:demo        load a bundled scene
//! select:3          select scene object 3
//! manip:translate   show translate|scale|rotate handles
//! view:Top          switch view (3d, Front, Back, Left, Right, Top, Bottom)
//! cam:x,y,z,theta,phi   place the world camera (angles in degrees)
//! menu              toggle the main menu
//! wait:30           wait 30 frames
//! shot:/tmp/a.png   save a screenshot
//! exit              quit
//! ```
//!
//! Example: `HOLODECK_SCRIPT="scene:demo;wait:60;shot:demo.png;wait:5;exit" cargo run`

use std::collections::VecDeque;

use bevy::app::AppExit;
use bevy::prelude::*;
use bevy::render::view::screenshot::{Screenshot, save_to_disk};

use crate::camera::CameraManager;
use crate::objects::manip::{ManipKind, Manipulator};
use crate::objects::model::{self, Model};
use crate::objects::pick::Selection;
use crate::settings::Settings;
use crate::ui3d::menu::{self, MenuBar};
use crate::view::{ViewPort, ViewType};

#[derive(Debug, PartialEq)]
pub enum Step {
    Scene(String),
    Select(usize),
    Manip(ManipKind),
    View(ViewType),
    Cam { pos: Vec3, theta: f64, phi: f64 },
    Menu,
    Wait(u32),
    Shot(String),
    Exit,
}

pub fn parse_script(text: &str) -> Result<Vec<Step>, String> {
    text.split(';')
        .map(str::trim)
        .filter(|s| !s.is_empty())
        .map(|step| {
            let (name, arg) = step.split_once(':').unwrap_or((step, ""));
            let number = |s: &str| s.trim().parse::<f64>().map_err(|_| format!("bad number '{s}' in '{step}'"));
            Ok(match name {
                "scene" => Step::Scene(arg.to_owned()),
                "select" => Step::Select(arg.trim().parse().map_err(|_| format!("bad id in '{step}'"))?),
                "manip" => Step::Manip(match arg {
                    "translate" => ManipKind::Translate,
                    "scale" => ManipKind::Scale,
                    "rotate" => ManipKind::Rotate,
                    _ => return Err(format!("unknown manipulator '{arg}'")),
                }),
                "view" => Step::View(
                    *ViewType::ALL.iter().find(|v| v.caption().eq_ignore_ascii_case(arg)).ok_or_else(|| format!("unknown view '{arg}'"))?,
                ),
                "cam" => {
                    let v: Vec<f64> = arg.split(',').map(number).collect::<Result<_, _>>()?;
                    let [x, y, z, theta, phi] = v[..] else { return Err(format!("cam needs x,y,z,theta,phi: '{step}'")) };
                    Step::Cam { pos: Vec3::new(x as f32, y as f32, z as f32), theta: theta.to_radians(), phi: phi.to_radians() }
                }
                "menu" => Step::Menu,
                "wait" => Step::Wait(arg.trim().parse().map_err(|_| format!("bad frame count in '{step}'"))?),
                "shot" => Step::Shot(arg.to_owned()),
                "exit" => Step::Exit,
                other => return Err(format!("unknown step '{other}'")),
            })
        })
        .collect()
}

#[derive(Resource)]
struct Script {
    steps: VecDeque<Step>,
    waiting: u32,
}

pub struct DevtoolsPlugin;

impl Plugin for DevtoolsPlugin {
    fn build(&self, app: &mut App) {
        let Ok(text) = std::env::var("HOLODECK_SCRIPT") else { return };
        match parse_script(&text) {
            Ok(steps) => {
                app.insert_resource(Script { steps: steps.into(), waiting: 0 }).add_systems(Update, run_script);
            }
            Err(e) => error!("HOLODECK_SCRIPT: {e}"),
        }
    }
}

fn run_script(world: &mut World) {
    let step = {
        let mut script = world.resource_mut::<Script>();
        if script.waiting > 0 {
            script.waiting -= 1;
            return;
        }
        script.steps.pop_front()
    };
    let Some(step) = step else { return };

    match step {
        Step::Scene(name) => match model::load_bundled_scene(world, &name) {
            Ok(errors) => errors.iter().for_each(|e| warn!("{e}")),
            Err(e) => error!("{e}"),
        },
        Step::Select(id) => {
            let entity = world.resource::<Model>().entity(id);
            world.resource_mut::<Selection>().entity = entity;
        }
        Step::Manip(kind) => world.resource_mut::<Manipulator>().kind = Some(kind),
        Step::View(view_type) => {
            let camera = world.resource::<ViewPort>().camera;
            world.resource_scope(|world, mut view: Mut<ViewPort>| {
                world.resource_scope(|world, mut settings: Mut<Settings>| {
                    if let Some(rig) = world.resource_mut::<CameraManager>().cameras.get_mut(camera) {
                        view.set_view(view_type, rig, &mut settings);
                    }
                });
            });
        }
        Step::Cam { pos, theta, phi } => {
            let camera = world.resource::<ViewPort>().camera;
            if let Some(rig) = world.resource_mut::<CameraManager>().cameras.get_mut(camera) {
                (rig.pos, rig.theta, rig.phi) = (pos, theta, phi);
            }
        }
        Step::Menu => {
            let mut query = world.query::<&mut MenuBar>();
            for mut bar in query.iter_mut(world).filter(|b| !b.is_context) {
                bar.state = if bar.is_active() { menu::MenuState::Retracting } else { menu::MenuState::Engaging };
            }
        }
        Step::Wait(frames) => world.resource_mut::<Script>().waiting = frames,
        Step::Shot(path) => {
            world.commands().spawn(Screenshot::primary_window()).observe(save_to_disk(path));
        }
        Step::Exit => {
            world.send_event(AppExit::Success);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_every_step_kind() {
        let steps = parse_script("scene:demo; select:3;manip:rotate;view:top;cam:1,2,3,90,-10;menu;wait:5;shot:/tmp/a.png;exit").unwrap();
        assert_eq!(steps.len(), 9);
        assert_eq!(steps[0], Step::Scene("demo".into()));
        assert_eq!(steps[3], Step::View(ViewType::Top));
        assert!(matches!(steps[4], Step::Cam { pos, .. } if pos == Vec3::new(1.0, 2.0, 3.0)));
        assert_eq!(steps[7], Step::Shot("/tmp/a.png".into()));
    }

    #[test]
    fn bad_scripts_explain_themselves() {
        assert!(parse_script("fly:away").unwrap_err().contains("unknown step"));
        assert!(parse_script("select:abc").is_err());
        assert!(parse_script("cam:1,2").is_err());
        assert!(parse_script("view:sideways").is_err());
        assert!(parse_script("").unwrap().is_empty());
    }
}
