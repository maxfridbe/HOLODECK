//! The file list used by the open/save dialogs.
//!
//! Entries are shown like the original: `<Up Dir>`, `<folder>` names, drive
//! roots on Windows, then files. Where there is no file system (the browser,
//! Android) the list shows the files bundled with the program instead.

use std::path::{Path, PathBuf};

use bevy::prelude::*;

use super::Command;
use super::actions::{Action, UiAction};
use super::widgets::{Field, ListActivated, ListBox, TextBox, Window3d, owning_window};
use crate::data;

pub const UP_DIR: &str = "<Up Dir>";

/// Attached to a file list to say what it is browsing.
#[derive(Component, Debug)]
pub struct FileBrowser {
    /// `None` when browsing the bundled files.
    pub dir: Option<PathBuf>,
    pub purpose: Command,
}

impl FileBrowser {
    pub fn extension(&self) -> &'static str {
        match self.purpose {
            Command::RequestLoadModel => "3dbin",
            _ => "world",
        }
    }
}

/// Where dialogs start: the data folder if there is one, else the working
/// directory. `None` on platforms without a usable file system.
pub fn start_dir() -> Option<PathBuf> {
    if cfg!(any(target_arch = "wasm32", target_os = "android")) {
        return None;
    }
    ["data", "assets/data"].iter().map(PathBuf::from).find(|p| p.is_dir()).or_else(|| std::env::current_dir().ok())
}

fn is_folder_entry(item: &str) -> bool {
    item.starts_with('<') && item.ends_with('>') && item != UP_DIR
}

fn is_drive(item: &str) -> bool {
    item.len() == 3 && item.ends_with(":\\")
}

/// True for list entries that are files (not folders, drives or `<Up Dir>`).
pub fn is_file_entry(item: &str) -> bool {
    !item.starts_with('<') && !is_drive(item)
}

/// Builds the list entries for a directory.
pub fn list_entries(dir: Option<&Path>, extension: &str) -> Vec<String> {
    let Some(dir) = dir else {
        return data::file_names().into_iter().filter(|n| n.ends_with(&format!(".{extension}"))).collect();
    };

    let mut items = Vec::new();
    #[cfg(windows)]
    for letter in b'A'..=b'Z' {
        let root = format!("{}:\\", letter as char);
        if Path::new(&root).exists() {
            items.push(root);
        }
    }
    if dir.parent().is_some() {
        items.push(UP_DIR.to_owned());
    }

    let mut folders = Vec::new();
    let mut files = Vec::new();
    if let Ok(read) = std::fs::read_dir(dir) {
        for entry in read.flatten() {
            let name = entry.file_name().to_string_lossy().into_owned();
            let path = entry.path();
            if path.is_dir() {
                folders.push(format!("<{name}>"));
            } else if path.extension().is_some_and(|e| e.eq_ignore_ascii_case(extension)) {
                files.push(name);
            }
        }
    }
    folders.sort_by_key(|n| n.to_lowercase());
    files.sort_by_key(|n| n.to_lowercase());
    items.extend(folders);
    items.extend(files);
    items
}

/// Where activating `item` in `dir` leads.
pub fn navigate(dir: &Path, item: &str) -> Option<PathBuf> {
    if item == UP_DIR {
        dir.parent().map(Path::to_path_buf)
    } else if is_drive(item) {
        Some(PathBuf::from(item))
    } else if is_folder_entry(item) {
        Some(dir.join(&item[1..item.len() - 1]))
    } else {
        None
    }
}

/// A short form of `dir` for the path box.
pub fn path_caption(dir: Option<&Path>) -> String {
    let full = dir.map_or_else(|| "(bundled files)".to_owned(), |d| d.display().to_string());
    let chars: Vec<char> = full.chars().collect();
    if chars.len() > 30 { format!("...{}", chars[chars.len() - 27..].iter().collect::<String>()) } else { full }
}

/// Fills a freshly created list and its path box.
pub fn populate(browser: &FileBrowser, list: &mut ListBox) {
    list.set_items(list_entries(browser.dir.as_deref(), browser.extension()));
}

/// Double-click: enter folders, or accept a file.
#[allow(clippy::too_many_arguments)]
pub fn activate_entries(
    mut activated: EventReader<ListActivated>,
    mut lists: Query<(&mut ListBox, &mut FileBrowser)>,
    parents: Query<&Parent>,
    windows: Query<&Window3d>,
    mut boxes: Query<(Entity, &Field, &mut TextBox)>,
    mut actions: EventWriter<UiAction>,
) {
    for event in activated.read() {
        let Ok((mut list, mut browser)) = lists.get_mut(event.list) else { continue };
        let Some(item) = list.selected_item().map(str::to_owned) else { continue };
        let window = owning_window(event.list, &parents, &windows);

        if is_file_entry(&item) {
            actions.send(UiAction { action: Action::FileOk, window });
            continue;
        }
        let Some(dir) = browser.dir.clone() else { continue };
        if let Some(next) = navigate(&dir, &item) {
            browser.dir = Some(next);
            list.set_items(list_entries(browser.dir.as_deref(), browser.extension()));
            for (entity, field, mut text_box) in &mut boxes {
                if field.0 == "path" && owning_window(entity, &parents, &windows) == window {
                    text_box.set_text(path_caption(browser.dir.as_deref()));
                }
            }
        }
    }
}

/// Selecting a file copies its name into the file name box.
pub fn show_selected_name(
    lists: Query<(Entity, &ListBox), (Changed<ListBox>, With<FileBrowser>)>,
    parents: Query<&Parent>,
    windows: Query<&Window3d>,
    mut boxes: Query<(Entity, &Field, &mut TextBox)>,
) {
    for (list_entity, list) in &lists {
        let Some(item) = list.selected_item().filter(|i| is_file_entry(i)) else { continue };
        let window = owning_window(list_entity, &parents, &windows);
        for (entity, field, mut text_box) in &mut boxes {
            if field.0 == "filename" && owning_window(entity, &parents, &windows) == window && text_box.text != item {
                text_box.set_text(item);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn scratch(name: &str) -> PathBuf {
        let dir = std::env::temp_dir().join(format!("holodeck-test-{name}-{}", std::process::id()));
        let _ = std::fs::remove_dir_all(&dir);
        std::fs::create_dir_all(dir.join("Sub")).unwrap();
        for f in ["b.world", "A.world", "model.3dbin", "notes.txt"] {
            std::fs::write(dir.join(f), b"x").unwrap();
        }
        dir
    }

    #[test]
    fn lists_up_dir_then_folders_then_matching_files_sorted() {
        let dir = scratch("list");
        let items = list_entries(Some(&dir), "world");
        let start = items.iter().position(|i| i == UP_DIR).unwrap();
        assert_eq!(&items[start..], [UP_DIR, "<Sub>", "A.world", "b.world"]);
        assert_eq!(list_entries(Some(&dir), "3dbin").last().unwrap(), "model.3dbin");
        let _ = std::fs::remove_dir_all(dir);
    }

    #[test]
    fn navigation_follows_the_entry_kind() {
        let dir = Path::new("/a/b");
        assert_eq!(navigate(dir, UP_DIR), Some(PathBuf::from("/a")));
        assert_eq!(navigate(dir, "<Sub>"), Some(PathBuf::from("/a/b/Sub")));
        assert_eq!(navigate(dir, "C:\\"), Some(PathBuf::from("C:\\")));
        assert_eq!(navigate(dir, "file.world"), None);
    }

    #[test]
    fn entry_classification() {
        assert!(is_file_entry("demo.world"));
        assert!(!is_file_entry("<Sub>"));
        assert!(!is_file_entry(UP_DIR));
        assert!(!is_file_entry("D:\\"));
    }

    #[test]
    fn bundled_listing_filters_by_extension() {
        let scenes = list_entries(None, "world");
        assert!(scenes.contains(&"demo.world".to_owned()));
        assert!(scenes.iter().all(|s| s.ends_with(".world")));
        assert!(list_entries(None, "3dbin").contains(&"camera.3dbin".to_owned()));
    }

    #[test]
    fn long_paths_are_shortened_from_the_left() {
        let caption = path_caption(Some(Path::new("/very/long/path/that/goes/on/and/on/forever/data")));
        assert!(caption.starts_with("...") && caption.ends_with("forever/data"));
        assert_eq!(caption.chars().count(), 30);
        assert_eq!(path_caption(None), "(bundled files)");
    }
}
