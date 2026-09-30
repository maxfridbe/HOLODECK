//! Colours and sizes of the UI, taken from the original's defaults
//! (`ui3d/userinterface.cpp`, `window3d.cpp`, `button3d.cpp`, `textbox3d.cpp`,
//! `listbox.cpp`, `menubar.cpp`). Colours were given as 0-255 bytes.

use bevy::prelude::*;

const fn rgb(r: u8, g: u8, b: u8) -> Color {
    Color::srgb(r as f32 / 255.0, g as f32 / 255.0, b as f32 / 255.0)
}

/// General text (window captions, messages, list entries, button captions).
pub const TEXT: Color = rgb(30, 30, 30);
/// Folder entries in file lists.
pub const FOLDER: Color = rgb(0, 0, 230);

/// Window body: brighter while the window has focus.
pub const WINDOW_FOCUSED: Color = rgb(245, 245, 245);
pub const WINDOW: Color = rgb(222, 222, 222);
/// The one-pixel "shadow" outline around windows.
pub const WINDOW_OUTLINE: Color = Color::srgba(0.0, 0.0, 0.0, 100.0 / 255.0);
/// Title tab: white at the top fading to grey.
pub const TITLE_TOP: Color = rgb(255, 255, 255);
pub const TITLE_BOTTOM: Color = rgb(100, 100, 100);
/// Close button: dark magenta to magenta, with a red X.
pub const CLOSE_TOP: Color = rgb(50, 0, 25);
pub const CLOSE_BOTTOM: Color = rgb(200, 0, 100);
pub const CLOSE_TEXT: Color = rgb(255, 0, 0);

/// Buttons: two grey gradient blocks.
pub const BUTTON_TOP: Color = rgb(210, 210, 210);
pub const BUTTON_BOTTOM: Color = rgb(175, 175, 175);

/// Text boxes: blue text, blue frame when focused, grey otherwise.
pub const FIELD_TEXT: Color = rgb(0, 0, 225);
pub const FIELD_FRAME_FOCUSED: Color = rgb(0, 0, 225);
pub const FIELD_FRAME: Color = rgb(125, 125, 125);
pub const FIELD_FILL_FOCUSED: Color = rgb(220, 220, 220);
pub const FIELD_FILL: Color = rgb(170, 170, 170);
pub const FIELD_CURSOR: Color = rgb(220, 0, 0);

/// List boxes.
pub const LIST_FRAME: Color = rgb(54, 54, 54);
pub const LIST: Color = rgb(210, 210, 210);
pub const LIST_SELECTED: Color = rgb(0, 210, 210);
pub const SCROLL_TRACK: Color = rgb(200, 200, 200);
pub const SCROLL_THUMB: Color = rgb(50, 50, 50);
pub const SCROLL_LIGHT: Color = rgb(200, 200, 200);
pub const SCROLL_DARK: Color = rgb(100, 100, 100);

/// Menus: the selected header/item uses the full gradient, the others half
/// brightness; disabled items use the "off" colours.
pub const MENU_ON_TOP: Color = rgb(255, 255, 255);
pub const MENU_ON_BOTTOM: Color = rgb(127, 127, 127);
pub const MENU_TOP: Color = rgb(127, 127, 127);
pub const MENU_BOTTOM: Color = rgb(63, 63, 63);
pub const MENU_OFF_TOP: Color = rgb(128, 128, 128);
pub const MENU_OFF_BOTTOM: Color = rgb(32, 32, 32);
pub const MENU_TEXT: Color = rgb(30, 30, 30);
pub const MENU_TEXT_SELECTED: Color = rgb(255, 255, 255);
pub const MENU_TEXT_DISABLED: Color = rgb(60, 60, 60);
pub const MENU_CHECK: Color = rgb(0, 255, 0);

/// The original created an 18px bold Courier font. A positive height in
/// `CreateFont` is the *cell* height (ascent + descent, 1.133 em for Courier
/// New and its metric clone Liberation Mono), so the glyphs are 18 / 1.133
/// = 15.9 px.
pub const FONT_SIZE: f32 = 16.0;
/// Horizontal advance of one character of the monospaced UI font.
pub const CHAR_WIDTH: f32 = FONT_SIZE * 0.6;
/// Height of the title strip (the original's drag area was the top 32px).
pub const TITLE_HEIGHT: f32 = 24.0;
pub const ROW_HEIGHT: f32 = 16.0;
pub const MENU_WIDTH: f32 = 128.0;
pub const MENU_HEIGHT: f32 = 20.0;
/// Menu row height on touch screens: finger-sized, yet short enough that the
/// 11-item View menu fits a 412dp-tall landscape phone.
pub const TOUCH_MENU_HEIGHT: f32 = 32.0;
/// List box row height on touch screens.
pub const TOUCH_ROW_HEIGHT: f32 = 30.0;
/// The collapsed menu tab.
pub const MENU_TAB_WIDTH: f32 = 65.0;
pub const TOUCH_MENU_TAB_WIDTH: f32 = 96.0;
/// Narrowest a menu header may get on a narrow screen.
pub const MIN_MENU_WIDTH: f32 = 64.0;
