//! A scripting hook for smoke tests and screenshots.
//!
//! Set `HOLODECK_SCRIPT` to a `;`-separated list of steps, run in order, one
//! per frame unless they wait:
//!
//! ```text
//! scene:demo        load a bundled scene
//! select:3          select scene object 3
//! selectcam:1       select placed camera 1
//! ui:view           run a menu action: view, control, translate, deselect,
//!                   loadscene, loadmodel, properties, wireframe
//! manip:translate   show translate|scale|rotate handles
//! view:Top          switch view (3d, Front, Back, Left, Right, Top, Bottom)
//! cam:x,y,z,theta,phi   place the world camera (angles in degrees)
//! menu              toggle the main menu
//! size:1920,1080    resize the window (logical pixels)
//! model:cone        open a bundled model
//! tdown:0,x,y       finger 0 touches the screen at (x, y)
//! tmove:0,x,y       finger 0 moves (sent as 8 steps over 8 frames)
//! tup:0,x,y         finger 0 lifts
//! tap:x,y           finger 0 down, then up two frames later
//! wait:30           wait 30 frames
//! shot:/tmp/a.png   save a screenshot
//! exit              quit
//! ```
//!
//! Example: `HOLODECK_SCRIPT="scene:demo;wait:60;shot:demo.png;wait:5;exit" cargo run`

use std::collections::VecDeque;

use bevy::app::AppExit;
use bevy::input::touch::{TouchInput, TouchPhase};
use bevy::prelude::*;
use bevy::render::view::screenshot::{Screenshot, save_to_disk};

use crate::camera::CameraManager;
use crate::objects::manip::{ManipKind, Manipulator};
use crate::objects::model::{self, Model};
use crate::objects::pick::Selection;
use crate::settings::Settings;
use crate::ui3d::actions::{Action, UiAction};
use crate::ui3d::menu::{self, MenuBar};
use crate::view::{ViewPort, ViewType};

#[derive(Debug, PartialEq)]
pub enum Step {
    Scene(String),
    Select(usize),
    SelectCamera(usize),
    Ui(Action),
    Manip(ManipKind),
    View(ViewType),
    Cam { pos: Vec3, theta: f64, phi: f64 },
    Menu,
    Model(String),
    Size(Vec2),
    Touch { id: u64, phase: TouchPhase, at: Vec2 },
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
            Ok(vec![match name {
                "scene" => Step::Scene(arg.to_owned()),
                "select" => Step::Select(arg.trim().parse().map_err(|_| format!("bad id in '{step}'"))?),
                "selectcam" => Step::SelectCamera(arg.trim().parse().map_err(|_| format!("bad camera in '{step}'"))?),
                "ui" => Step::Ui(match arg {
                    "view" => Action::LookFromCamera,
                    "control" => Action::ControlCamera,
                    "translate" => Action::SetManip(ManipKind::Translate),
                    "deselect" => Action::Deselect,
                    "loadscene" => Action::LoadScene,
                    "loadmodel" => Action::LoadModel,
                    "properties" => Action::ObjectProperties,
                    "wireframe" => Action::ToggleWireframe,
                    _ => return Err(format!("unknown ui action '{arg}'")),
                }),
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
                "model" => Step::Model(arg.to_owned()),
                "size" => {
                    let v: Vec<f64> = arg.split(',').map(number).collect::<Result<_, _>>()?;
                    let [w, h] = v[..] else { return Err(format!("size needs w,h: '{step}'")) };
                    Step::Size(Vec2::new(w as f32, h as f32))
                }
                "tdown" | "tmove" | "tup" | "tap" => {
                    let v: Vec<f64> = arg.split(',').map(number).collect::<Result<_, _>>()?;
                    let (id, x, y) = match v[..] {
                        [x, y] if name == "tap" => (0.0, x, y),
                        [id, x, y] if name != "tap" => (id, x, y),
                        _ => return Err(format!("bad touch arguments in '{step}'")),
                    };
                    let phase = match name {
                        "tdown" | "tap" => TouchPhase::Started,
                        "tmove" => TouchPhase::Moved,
                        _ => TouchPhase::Ended,
                    };
                    let touch = Step::Touch { id: id as u64, phase, at: Vec2::new(x as f32, y as f32) };
                    if name == "tap" {
                        return Ok(vec![touch, Step::Wait(2), Step::Touch { id: 0, phase: TouchPhase::Ended, at: Vec2::new(x as f32, y as f32) }]);
                    }
                    touch
                }
                "wait" => Step::Wait(arg.trim().parse().map_err(|_| format!("bad frame count in '{step}'"))?),
                "shot" => Step::Shot(arg.to_owned()),
                "exit" => Step::Exit,
                other => return Err(format!("unknown step '{other}'")),
            }])
        })
        .collect::<Result<Vec<Vec<Step>>, String>>()
        .map(|steps| steps.into_iter().flatten().collect())
}

#[derive(Resource)]
struct Script {
    steps: VecDeque<Step>,
    waiting: u32,
    /// Last position of each scripted finger, to interpolate moves.
    fingers: std::collections::HashMap<u64, Vec2>,
}

pub struct DevtoolsPlugin;

impl Plugin for DevtoolsPlugin {
    fn build(&self, app: &mut App) {
        let Ok(text) = std::env::var("HOLODECK_SCRIPT") else { return };
        match parse_script(&text) {
            Ok(steps) => {
                app.insert_resource(Script { steps: steps.into(), waiting: 0, fingers: default() }).add_systems(Update, run_script);
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
        Step::SelectCamera(index) => {
            let entity = world.resource::<CameraManager>().cameras.get(index).and_then(|c| c.model);
            world.resource_mut::<Selection>().entity = entity;
        }
        Step::Ui(action) => {
            world.send_event(UiAction { action, window: None });
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
        Step::Model(name) => {
            if let Err(e) = model::open(world, None, &name) {
                error!("{e}");
            }
        }
        Step::Size(size) => {
            let mut windows = world.query_filtered::<&mut Window, With<bevy::window::PrimaryWindow>>();
            for mut window in windows.iter_mut(world) {
                window.resolution.set(size.x, size.y);
            }
        }
        Step::Touch { id, phase, at } => {
            let window = world.query_filtered::<Entity, With<bevy::window::PrimaryWindow>>().iter(world).next();
            let Some(window) = window else { return };
            if phase == TouchPhase::Moved {
                // Split into small steps, one per frame, like a real finger.
                let from = world.resource::<Script>().fingers.get(&id).copied().unwrap_or(at);
                let mut script = world.resource_mut::<Script>();
                for i in (1..=8).rev() {
                    let p = from.lerp(at, i as f32 / 8.0);
                    script.steps.push_front(Step::Touch { id, phase: TouchPhase::Canceled, at: p });
                }
                return;
            }
            // `Canceled` above is only a marker for an interpolated move.
            let phase = if phase == TouchPhase::Canceled { TouchPhase::Moved } else { phase };
            world.resource_mut::<Script>().fingers.insert(id, at);
            world.send_event(TouchInput { phase, position: at, window, force: None, id });
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
        let touch = parse_script("tdown:1,10,20;tmove:1,30,20;tup:1,30,20;tap:5,6").unwrap();
        assert_eq!(touch.len(), 6, "tap expands to down, wait, up");
        assert_eq!(touch[0], Step::Touch { id: 1, phase: TouchPhase::Started, at: Vec2::new(10.0, 20.0) });
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
