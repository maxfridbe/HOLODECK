# Holodeck

**[▶ Play it in your browser](https://maxfridbe.github.io/HOLODECK/)** — built from `main` and deployed to GitHub Pages by the release workflow.

Holodeck is a 3D scene editor / simulator: you float inside a holographic grid, load models and scenes, select objects, and move, scale and rotate them with on-screen handles. It is a Rust/[Bevy](https://bevyengine.org) port of the original C++/OpenGL project (VRUPL, 2004–05); the original sources are kept in [`cpp/`](cpp/) for reference. The build system, packaging and CI come from [GameBase](https://github.com/maxfridbe/GameBase), so Holodeck ships to **Linux, Windows, macOS (Apple Silicon), Android and the browser** from one crate.

## Playing

Start with **Space** to open the menu, then **Scene → Open** and pick `demo.world` (or `fixed.world` for objects you can grab). Models and scenes bundled with the game:

| Kind | Files |
|------|-------|
| Models (`.3dbin`) | `backpack`, `blueEye`, `camera`, `cone`, `SkyDome`, `vrupl` |
| Scenes (`.world`) | `demo`, `fixed`, `fixed2`, `scenex`, `test`, `testing`, `world` |

| Input | Action |
|-------|--------|
| **W A S D** | Fly (speed ramps up while held) |
| **Ctrl + mouse** | Look around (the pointer is captured while Ctrl is held) |
| Mouse wheel | Field of view (zoom in orthographic views) |
| Left-click | Select the object under the pointer; drag a handle to use it |
| Right-click | Context menu for what is selected (Translate / Scale / Rotate / Properties / Modify Forces / Duplicate / Deselect / Unload) |
| **Space** | Show / hide the menu bar |
| **G** | Toggle the holographic grid |
| **Q** | Quit (asks first) |
| Enter / Esc | Accept / cancel the front dialog. Tab moves between fields |

**Menus** — *File* (Connect, Exit) · *Edit* (Duplicate) · *Settings* (Grid colours) · *View* (3d, Front, Back, Left, Right, Top, Bottom, HoloGrid, Wireframe, Inner Grid) · *Scene* (New, Save, Open, Add World Force) · *Model* (Load, Unload).

**Cameras** — the small camera object in the world can be selected: **View** opens a picture-in-picture window showing what it sees, **Control** flies it (Esc to finish), **Translate** moves it.

**Handles** — after choosing Translate/Scale/Rotate from the context menu, arrows (translate, world axes), cubes (scale, the object's own axes) or rings (rotate, world axes) appear around the object. Drag one and the object follows the pointer. Handles keep a constant size on screen. Faint boxes around every object show its world-space bounding box; a box turns red while it overlaps another.

**Forces** — *Scene → Add World Force* defines named forces; *Modify Forces* on an object attaches them so the object moves under simple point-mass physics.

**Networking** — *File → Connect* talks to the multi-user server (TCP 1307, same wire format as the C# server in the original project). Not available in the browser.

## Running and building

```bash
./setup_env.sh --check   # see what your machine is missing
./run_linux.sh           # play natively (release build); `./run_linux.sh debug` for debug
cargo test               # unit tests (parsers, geometry, UI logic, networking, ...)
./build_web.sh           # -> target/web_dist  (then: python3 -m http.server -d target/web_dist 8080)
./build_windows.sh   ./build_macos.sh   ./build_cargo_apk.sh   # other platforms
```

The full script list, platform notes, macOS signing caveats and the Android/Gradle paths are exactly as in GameBase — see its README. `game.env` holds this game's identity (`holodeck`, `Holodeck`, `com.vrupl.holodeck`).

### Versions and releases

Versions are `YY.MMDD.##` — for example `26.0929.03` is the third release of 29 Sep 2026. Every push to `main` runs [`release.yml`](.github/workflows/release.yml), which

1. works out the next version (today's date; `##` continues from the existing `v<YY.MMDD>.##` tags),
2. builds Linux, Windows, macOS, Android and web,
3. publishes a GitHub Release `v<version>` with all artifacts and deploys the web build to GitHub Pages,
4. commits the version back to `version.txt`, `Cargo.toml` and the Gradle files (`[skip ci]`).

`./increment_version.sh` does the same locally; `./increment_version.sh --print` shows the next version. Cargo requires semver, which forbids leading zeros, so `Cargo.toml` carries the same numbers unpadded (`26.0929.03` → `26.929.3`); tags, release names and `version.txt` use the padded form.

**One-time setup:** enable GitHub Pages under *Settings → Pages → Source: GitHub Actions* so the first deploy can publish the playable build.

### Development hooks

`HOLODECK_SCRIPT` runs scripted steps for smoke tests and screenshots (see `src/devtools.rs`):

```bash
HOLODECK_SCRIPT="scene:fixed;cam:5,10,-60,0,0;wait:60;select:0;manip:translate;wait:5;shot:out.png;wait:5;exit" cargo run --release
```

## How it is organised

```
src/lib.rs        app wiring (plugins, main camera)            src/main.rs  desktop entry point
src/objects/      3dbin + scene readers, models, bounding boxes, picking, manipulator
src/camera.rs     camera rigs, placed camera objects           src/view.rs  viewport / projection modes
src/grid.rs       holographic grid                             src/input.rs flight, look, hotkeys
src/ui3d/         menus, windows, text/list boxes, dialogs     src/net/     TCP client
src/physics/      forces and point-mass nodes                  assets/data/ bundled models and scenes
```

Original module → port: `adt` (custom string/list/hash containers) → Rust std · `math` → `glam` + `src/math.rs` · `win32` (window, DirectInput, timing) → Bevy · `holodeck` → `lib/view/input/grid` · `objects` → `src/objects` · `ui3d` → `src/ui3d` (rebuilt on `bevy_ui`) · `net` → `src/net` · `physics` → `src/physics`. Not ported: the C# `Server` and `packer` tools and the 3ds converter (kept in `cpp/`; the packer's format spec is implemented by `src/objects/format.rs`).

## Differences from the original

Fixed, on purpose:

* **Bounding boxes in negative space.** The C++ seeded the "max" corner with `FLT_MIN` (the smallest *positive* float), so any box lying in negative space got a max of ~0 — the manipulator "grabbed the edge of the box at the origin". Boxes are now fitted correctly (regression-tested).
* **Manipulator size.** Handles were sized from the *unscaled model-space* box, measured to the wrong point with a Manhattan distance, and the object's extent was multiplied by the view distance as well — hence "excessively large", worst for the rotate tool. Now: distance is to the object's real world-space centre, only the handle geometry scales with distance, and rotate rings hug the object but are capped.
* **Dragging.** Handles used to convert mouse speed into movement per frame (frame-rate dependent). Dragging now tracks the pointer along the axis / around the ring.
* Mouse look is **Ctrl + mouse** rather than always on: the original turned the camera on every mouse movement while a visible pointer moved around, which made clicking things very hard.
* Scene files: older encodings (Euler angles, run-together matrix rows) that the C++ loader could not read now load; scene cameras are not saved.
* Physics: `Reset` really resets, collisions conserve momentum, forces can be removed by name and the "attach force" UI is finished. `Model::Close` promotes a copy when its original is removed.
* Memory/UB fixes, no leaked matrices, no dangling camera pointers after loading a scene, errors shown in message boxes instead of crashes.

## License

Apache-2.0 — see [LICENSE](LICENSE).
