//! Reader/writer for `.world` scene files (see `cpp/objects/SceneFileSpecification.txt`).
//!
//! ```text
//! SS
//! px py pz vx vy vz          player position/view (written as a default, unused)
//! slotCount
//! per slot:
//!   id                        -1 marks an empty slot (nothing else follows)
//!   parentId                  id of the object this is a copy of, or -1
//!   copy                      1 if this object is a copy
//!   fileName                  model name, relative to the scene
//!   <rotation>                4 rows of 4 floats (row-major matrix)
//!   sx sy sz
//!   tx ty tz
//! ```
//!
//! Older files exist in the wild, so the rotation block is read leniently:
//! 3 floats are Euler angles in degrees, 16 floats are the matrix, and 13
//! floats are a matrix whose rows were written without separators (`0` `0`
//! glued into `00`).

use std::fmt;

use bevy::prelude::*;

const MAGIC: &str = "SS";

#[derive(Debug, PartialEq)]
pub enum SceneError {
    BadMagic,
    UnexpectedEof { wanted: &'static str },
    BadNumber { line: usize, text: String },
    BadRotation { values: usize },
    /// A slot's id must equal its position in the file.
    InconsistentIndex { slot: usize, id: i64 },
    /// A copy refers to a parent that is missing or not defined before it.
    MissingParent { slot: usize, parent: i64 },
}

impl fmt::Display for SceneError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::BadMagic => write!(f, "not a scene file (missing 'SS' header)"),
            Self::UnexpectedEof { wanted } => write!(f, "scene file ended while reading {wanted}"),
            Self::BadNumber { line, text } => write!(f, "line {line}: '{text}' is not a number"),
            Self::BadRotation { values } => write!(f, "cannot interpret a rotation of {values} values"),
            Self::InconsistentIndex { slot, id } => write!(f, "slot {slot} has id {id}"),
            Self::MissingParent { slot, parent } => write!(f, "slot {slot} is a copy of missing object {parent}"),
        }
    }
}

impl std::error::Error for SceneError {}

/// One object of a scene.
#[derive(Clone, Debug, PartialEq)]
pub struct SceneEntry {
    /// Equal to the entry's slot index.
    pub id: usize,
    /// For copies, the id of the object this was cloned from.
    pub parent_id: Option<usize>,
    pub is_copy: bool,
    /// Model file name (no extension), relative to the scene.
    pub file_name: String,
    pub transform: Transform,
}

#[derive(Clone, Debug, PartialEq)]
pub struct SceneFile {
    /// Player position and view point. Written for completeness; ignored on load.
    pub player: [f32; 6],
    /// One per object slot, `None` for slots freed by unloading a model.
    pub slots: Vec<Option<SceneEntry>>,
}

impl Default for SceneFile {
    fn default() -> Self {
        Self { player: [0.0, 0.0, 0.0, 0.0, 0.0, 1.0], slots: Vec::new() }
    }
}

struct Lines<'a> {
    lines: Vec<(usize, &'a str)>,
    next: usize,
}

impl<'a> Lines<'a> {
    fn new(text: &'a str) -> Self {
        let lines = text
            .lines()
            .enumerate()
            .map(|(i, l)| (i + 1, l.trim()))
            .filter(|(_, l)| !l.is_empty())
            .collect();
        Self { lines, next: 0 }
    }

    fn line(&mut self, wanted: &'static str) -> Result<(usize, &'a str), SceneError> {
        let item = self.lines.get(self.next).copied().ok_or(SceneError::UnexpectedEof { wanted })?;
        self.next += 1;
        Ok(item)
    }

    fn peek(&self) -> Option<(usize, &'a str)> {
        self.lines.get(self.next).copied()
    }

    fn integer(&mut self, wanted: &'static str) -> Result<i64, SceneError> {
        let (line, text) = self.line(wanted)?;
        text.parse().map_err(|_| SceneError::BadNumber { line, text: text.to_owned() })
    }
}

fn floats(line: usize, text: &str) -> Result<Vec<f32>, SceneError> {
    text.split_whitespace()
        .map(|t| t.parse().map_err(|_| SceneError::BadNumber { line, text: t.to_owned() }))
        .collect()
}

fn token_count(text: &str) -> usize {
    text.split_whitespace().count()
}

/// Reads the rotation, scale and translation lines that follow a file name:
/// every line with at least three numbers, up to the next slot's id line.
fn read_transform(lines: &mut Lines) -> Result<Transform, SceneError> {
    let mut rotation_values: Vec<f32> = Vec::new();
    let mut trailing: Vec<Vec<f32>> = Vec::new();
    let mut raw_tokens: Vec<String> = Vec::new();

    while let Some((line, text)) = lines.peek() {
        if token_count(text) < 3 {
            break;
        }
        lines.next += 1;
        trailing.push(floats(line, text)?);
        raw_tokens.extend(text.split_whitespace().map(str::to_owned));
    }

    let missing = SceneError::UnexpectedEof { wanted: "object transform" };
    if trailing.len() < 3 {
        return Err(missing);
    }
    let translate = trailing.pop().expect("checked above");
    let scale = trailing.pop().expect("checked above");
    if translate.len() != 3 || scale.len() != 3 {
        return Err(SceneError::BadRotation { values: translate.len().max(scale.len()) });
    }

    // Everything before scale/translate is the rotation. Re-derive it from
    // the raw tokens so glued values can be repaired.
    let rotation_tokens = raw_tokens.len() - 6;
    for token in &raw_tokens[..rotation_tokens] {
        if token == "00" {
            rotation_values.extend([0.0, 0.0]);
        } else {
            rotation_values.push(token.parse().map_err(|_| SceneError::BadNumber { line: 0, text: token.clone() })?);
        }
    }

    let rotation = match rotation_values.len() {
        3 => {
            let [x, y, z] = [rotation_values[0], rotation_values[1], rotation_values[2]].map(f32::to_radians);
            Quat::from_rotation_z(z) * Quat::from_rotation_y(y) * Quat::from_rotation_x(x)
        }
        16 => {
            let row = |r: usize| Vec3::new(rotation_values[r * 4], rotation_values[r * 4 + 1], rotation_values[r * 4 + 2]);
            // The file holds rows; glam builds from columns.
            Quat::from_mat3(&Mat3::from_cols(row(0), row(1), row(2)).transpose()).normalize()
        }
        values => return Err(SceneError::BadRotation { values }),
    };

    Ok(Transform {
        translation: Vec3::from_slice(&translate),
        rotation,
        scale: Vec3::from_slice(&scale),
    })
}

/// Parses the text of a scene file.
pub fn parse(text: &str) -> Result<SceneFile, SceneError> {
    let mut lines = Lines::new(text);

    if lines.line("header")?.1 != MAGIC {
        return Err(SceneError::BadMagic);
    }

    let (line, player_text) = lines.line("player position")?;
    let player: [f32; 6] = floats(line, player_text)?
        .try_into()
        .map_err(|v: Vec<f32>| SceneError::BadRotation { values: v.len() })?;

    let count = lines.integer("slot count")?;
    let mut slots: Vec<Option<SceneEntry>> = Vec::new();

    for slot in 0..usize::try_from(count).unwrap_or(0) {
        let id = lines.integer("object id")?;
        if id < 0 {
            slots.push(None);
            continue;
        }
        if id != slot as i64 {
            return Err(SceneError::InconsistentIndex { slot, id });
        }
        let parent = lines.integer("parent id")?;
        let is_copy = lines.integer("copy flag")? != 0;
        let (_, file_name) = lines.line("file name")?;
        let transform = read_transform(&mut lines)?;

        let parent_id = usize::try_from(parent).ok();
        if is_copy {
            let defined = parent_id.is_some_and(|p| p < slot && slots[p].is_some());
            if !defined {
                return Err(SceneError::MissingParent { slot, parent });
            }
        }

        slots.push(Some(SceneEntry { id: slot, parent_id, is_copy, file_name: file_name.to_owned(), transform }));
    }

    Ok(SceneFile { player, slots })
}

/// Renders a scene in the current file format.
pub fn serialize(scene: &SceneFile) -> String {
    use std::fmt::Write;

    let mut out = String::new();
    let p = scene.player;
    let _ = writeln!(out, "{MAGIC}\n{} {} {} {} {} {}", p[0], p[1], p[2], p[3], p[4], p[5]);
    let _ = writeln!(out, "{}", scene.slots.len());

    for slot in &scene.slots {
        let Some(e) = slot else {
            out.push_str("-1\n");
            continue;
        };
        let parent = e.parent_id.map_or(-1, |p| p as i64);
        let _ = writeln!(out, "{}\n{}\n{}\n{}", e.id, parent, u8::from(e.is_copy), e.file_name);

        let r = Mat3::from_quat(e.transform.rotation);
        for row in 0..3 {
            let _ = writeln!(out, "{} {} {} 0", r.col(0)[row], r.col(1)[row], r.col(2)[row]);
        }
        out.push_str("0 0 0 1\n");
        let (s, t) = (e.transform.scale, e.transform.translation);
        let _ = writeln!(out, "{} {} {}\n{} {} {}", s.x, s.y, s.z, t.x, t.y, t.z);
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    fn entry(id: usize, parent: Option<usize>, name: &str, t: Transform) -> SceneEntry {
        SceneEntry { id, parent_id: parent, is_copy: parent.is_some(), file_name: name.into(), transform: t }
    }

    fn close(a: &Transform, b: &Transform) -> bool {
        (a.translation - b.translation).length() < 1e-5
            && (a.scale - b.scale).length() < 1e-5
            && a.rotation.angle_between(b.rotation) < 1e-3
    }

    #[test]
    fn round_trips_including_empty_slots_and_copies() {
        let rotated = Transform {
            translation: Vec3::new(-104.743, 0.0, 250.0),
            rotation: Quat::from_euler(EulerRot::YXZ, 0.7, -0.2, 1.1),
            scale: Vec3::new(0.5, 2.0, 0.25),
        };
        let scene = SceneFile {
            player: [0.0, 0.0, 0.0, 0.0, 0.0, 1.0],
            slots: vec![
                Some(entry(0, None, "SkyDome", rotated)),
                None,
                Some(entry(2, Some(0), "SkyDome", Transform::from_xyz(1.0, 2.0, 3.0))),
            ],
        };
        let back = parse(&serialize(&scene)).unwrap();
        assert_eq!(back.slots.len(), 3);
        assert!(back.slots[1].is_none());
        let (a, b) = (back.slots[0].as_ref().unwrap(), scene.slots[0].as_ref().unwrap());
        assert!(close(&a.transform, &b.transform), "{:?} vs {:?}", a.transform, b.transform);
        assert_eq!(back.slots[2].as_ref().unwrap().parent_id, Some(0));
        assert!(back.slots[2].as_ref().unwrap().is_copy);
    }

    #[test]
    fn reads_the_matrix_rows_as_rows_not_columns() {
        // Rotation about Y by 90 degrees, exactly as the C++ wrote it:
        // RotateYBy uses [c 0 s; 0 1 0; -s 0 c].
        let text = "SS\n0 0 0 0 0 1\n1\n0\n-1\n0\nx\n0 0 1 0\n0 1 0 0\n-1 0 0 0\n0 0 0 1\n1 1 1\n0 0 0\n";
        let scene = parse(text).unwrap();
        let t = scene.slots[0].as_ref().unwrap().transform;
        let rotated = t.rotation * Vec3::X;
        // +X turned by +90 degrees about +Y lands on -Z.
        assert!((rotated - Vec3::new(0.0, 0.0, -1.0)).length() < 1e-5, "{rotated:?}");
    }

    #[test]
    fn crlf_and_exponents_with_leading_zeros_parse() {
        let text = "SS\r\n0 0 0 0 0 1\r\n1\r\n0\r\n-1\r\n0\r\nvrupl\r\n-1 2.27997e-014 -1.50996e-007 0\r\n0 -1 -1.50996e-007 0\r\n-1.50996e-007 -1.50996e-007 1 0\r\n0 0 0 1\r\n2 2 0.810962\r\n26.2897 89.4617 2.7578\r\n";
        let scene = parse(text).unwrap();
        let e = scene.slots[0].as_ref().unwrap();
        assert_eq!(e.file_name, "vrupl");
        assert_eq!(e.transform.scale, Vec3::new(2.0, 2.0, 0.810962));
    }

    #[test]
    fn legacy_euler_rotation_is_degrees() {
        let text = "SS\n0 0 0 0 0 1\n1\n0\n-1\n0\nblueEye\n0 90 0\n1 1 1\n0 0 0\n";
        let t = parse(text).unwrap().slots[0].as_ref().unwrap().transform;
        assert!((t.rotation * Vec3::X - Vec3::new(0.0, 0.0, -1.0)).length() < 1e-5);
    }

    #[test]
    fn glued_rows_are_repaired() {
        let text = "SS\n0 0 0 0 0 1\n1\n0\n-1\n0\nblueEye\n1 0 0 00 1 0 00 0 1 00 0 0 1\n1 1 1\n6.16972 24.8821 -2.41144\n";
        let t = parse(text).unwrap().slots[0].as_ref().unwrap().transform;
        assert!(t.rotation.angle_between(Quat::IDENTITY) < 1e-5);
        assert_eq!(t.translation, Vec3::new(6.16972, 24.8821, -2.41144));
    }

    #[test]
    fn whole_matrix_on_one_line_is_accepted() {
        let text = "SS\n0 0 0 0 0 1\n1\n0\n-1\n0\nblueEye\n1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1\n1 1 1\n1 2 3\n";
        assert_eq!(parse(text).unwrap().slots[0].as_ref().unwrap().transform.translation, Vec3::new(1.0, 2.0, 3.0));
    }

    #[test]
    fn ids_must_match_slot_positions() {
        let text = "SS\n0 0 0 0 0 1\n1\n5\n-1\n0\nx\n0 0 0\n1 1 1\n0 0 0\n";
        assert_eq!(parse(text), Err(SceneError::InconsistentIndex { slot: 0, id: 5 }));
    }

    #[test]
    fn copies_need_an_earlier_parent() {
        let text = "SS\n0 0 0 0 0 1\n1\n0\n7\n1\nx\n0 0 0\n1 1 1\n0 0 0\n";
        assert!(matches!(parse(text), Err(SceneError::MissingParent { .. })));
    }

    #[test]
    fn garbage_is_rejected() {
        assert_eq!(parse("hello"), Err(SceneError::BadMagic));
        assert!(matches!(parse("SS\n0 0 0 0 0 1\n2\n0\n"), Err(SceneError::UnexpectedEof { .. })));
        assert!(matches!(parse("SS\n0 0 0\n"), Err(SceneError::BadRotation { .. })));
    }

    #[test]
    fn every_bundled_world_parses() {
        let dir = concat!(env!("CARGO_MANIFEST_DIR"), "/assets/data");
        let mut seen = 0;
        for entry in std::fs::read_dir(dir).unwrap() {
            let path = entry.unwrap().path();
            if path.extension().is_some_and(|e| e == "world") {
                let text = std::fs::read_to_string(&path).unwrap();
                let scene = parse(&text).unwrap_or_else(|e| panic!("{}: {e}", path.display()));
                assert!(scene.slots.iter().flatten().count() >= 1, "{} is empty", path.display());
                seen += 1;
            }
        }
        assert_eq!(seen, 7);
    }
}
