//! Colours and sizes shared by the UI widgets. The palette follows the
//! original's defaults (light windows, dark text, blue folders).

use bevy::prelude::*;

pub const TEXT: Color = Color::srgb(30.0 / 255.0, 30.0 / 255.0, 30.0 / 255.0);
pub const FOLDER: Color = Color::srgb(0.0, 0.0, 230.0 / 255.0);
pub const WINDOW: Color = Color::srgb(0.93, 0.93, 0.95);
pub const WINDOW_BORDER: Color = Color::srgb(0.2, 0.2, 0.25);
pub const TITLE_BAR: Color = Color::srgb(0.16, 0.2, 0.36);
pub const TITLE_TEXT: Color = Color::WHITE;
pub const BUTTON: Color = Color::srgb(0.78, 0.8, 0.86);
pub const BUTTON_HOVER: Color = Color::srgb(0.68, 0.74, 0.92);
pub const BUTTON_PRESSED: Color = Color::srgb(0.5, 0.58, 0.85);
pub const FIELD: Color = Color::WHITE;
pub const FIELD_BORDER: Color = Color::srgb(0.35, 0.35, 0.4);
pub const FIELD_FOCUS: Color = Color::srgb(0.1, 0.4, 0.95);
pub const LIST: Color = Color::srgb(210.0 / 255.0, 210.0 / 255.0, 210.0 / 255.0);
pub const LIST_SELECTED: Color = Color::srgb(0.0, 210.0 / 255.0, 210.0 / 255.0);
pub const MENU: Color = Color::srgb(0.86, 0.86, 0.88);
pub const MENU_HOVER: Color = Color::srgb(0.25, 0.32, 0.55);
pub const MENU_TEXT_HOVER: Color = Color::WHITE;
pub const MENU_DISABLED: Color = Color::srgb(0.55, 0.55, 0.58);

pub const FONT_SIZE: f32 = 15.0;
pub const TITLE_HEIGHT: f32 = 28.0;
pub const ROW_HEIGHT: f32 = 16.0;
pub const MENU_WIDTH: f32 = 128.0;
pub const MENU_HEIGHT: f32 = 20.0;
/// Menu row height on touch screens: finger-sized, yet short enough that the
/// 11-item View menu fits a 412dp-tall landscape phone.
pub const TOUCH_MENU_HEIGHT: f32 = 32.0;
/// List box row height on touch screens.
pub const TOUCH_ROW_HEIGHT: f32 = 30.0;
/// The collapsed menu tab.
pub const MENU_TAB_WIDTH: f32 = 70.0;
pub const TOUCH_MENU_TAB_WIDTH: f32 = 96.0;
/// Narrowest a menu header may get on a narrow screen.
pub const MIN_MENU_WIDTH: f32 = 64.0;
