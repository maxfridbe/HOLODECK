//! Models and scenes that ship inside the binary.
//!
//! Loose files are used when they exist (desktop), but the bundled copies
//! guarantee the demo worlds load on every platform, including the browser
//! and Android where there is no data directory to read from.

macro_rules! bundled {
    ($dir:literal: $($name:literal),+ $(,)?) => {
        &[$(($name, include_bytes!(concat!("../assets/data/", $name, $dir)) as &[u8])),+]
    };
}

const MODELS: &[(&str, &[u8])] = bundled!(".3dbin": "backpack", "blueEye", "camera", "cone", "SkyDome", "vrupl");

const SCENES: &[(&str, &[u8])] =
    bundled!(".world": "demo", "fixed", "fixed2", "scenex", "test", "testing", "world");

fn lookup(table: &'static [(&'static str, &'static [u8])], name: &str) -> Option<&'static [u8]> {
    table.iter().find(|(n, _)| n.eq_ignore_ascii_case(name)).map(|(_, bytes)| *bytes)
}

/// The bundled `.3dbin` model with this name (no extension).
pub fn model(name: &str) -> Option<&'static [u8]> {
    lookup(MODELS, name)
}

/// The bundled `.world` scene with this name (no extension).
pub fn scene(name: &str) -> Option<&'static [u8]> {
    lookup(SCENES, name)
}

/// File names (with extension) of everything bundled, sorted, for the file
/// dialog on platforms without a file system.
pub fn file_names() -> Vec<String> {
    let mut names: Vec<String> = MODELS
        .iter()
        .map(|(n, _)| format!("{n}.3dbin"))
        .chain(SCENES.iter().map(|(n, _)| format!("{n}.world")))
        .collect();
    names.sort_by_key(|n| n.to_lowercase());
    names
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn lookup_ignores_case() {
        assert!(model("skydome").is_some());
        assert!(model("nonexistent").is_none());
        assert!(scene("DEMO").is_some());
    }

    #[test]
    fn bundled_bytes_match_the_files_on_disk() {
        let on_disk = std::fs::read(concat!(env!("CARGO_MANIFEST_DIR"), "/assets/data/camera.3dbin")).unwrap();
        assert_eq!(model("camera").unwrap(), on_disk.as_slice());
    }

    #[test]
    fn listing_covers_models_and_scenes() {
        let names = file_names();
        assert!(names.contains(&"camera.3dbin".to_string()));
        assert!(names.contains(&"demo.world".to_string()));
        assert_eq!(names.len(), MODELS.len() + SCENES.len());
    }
}
