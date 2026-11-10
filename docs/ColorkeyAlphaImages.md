# Colorkey images drawn with alpha blending (opaque-box bug)

## The problem

`graphicsLib::set_surface_alpha()` enables RLE + colorkey on a surface, then `SDLL_SetAlpha()` switches it to
alpha blending. With that combination SDL2 (stock 2.30, reproduced with both ARGB8888 and the PSP ABGR8888)
ignores the colorkey and draws the key colour (75,125,125) as an **opaque box**.

Verified with a standalone SDL2 test (RLE on + colorkey + BLEND = key drawn opaque; RLE off = correct).
It is not PSP-only: PC/Android/Switch run the same code and may show it too.

Turning RLE off in `SDLL_SetAlpha()` fixes it, but it makes every alpha-blended surface (including the
full-screen fades) use the generic slow blitter, which is likely a PSP performance loss. That change was
reverted. The alternative is to convert the images below so they **do not need the colorkey** when alpha is
applied (real PNG alpha channel, or a pre-baked semi-transparent look), or to draw them without
`set_surface_alpha()`.

## Images loaded from files and passed to `set_surface_alpha()` (affected)

| Image | Where | Key pixels | Note |
|---|---|---|---|
| `images/backgrounds/classic_style_stage_select_cursor.png` (each game) | `stage_select.cpp` cursor, alpha 255 and a pulsing alpha | ~4100-4250 | The visible bug: cursor box hides the face. 96x48, two 48x48 frames. |
| `images/animations/explosion_32.png` (each game) | `graphicslib.cpp` preloaded explosion bubble, alpha 120 | 1440 | Translucent explosion bubble. |
| Map foreground layers (`map_backgrounds/*` used as the **foreground** image, `backgrounds[1]` in the map data, when its "gfx" value is not 100) | `classmap.cpp` -> `draw::set_dynamic_bg_alpha()` | thousands | Which files depend on the maps that use them as a foreground with transparency. Candidates with key pixels: `clouds.png`, `fg_clouds.png` (Rockbot2), `red_clouds.png`, `clouds_purple.png`, `montains_purple.png`, `montains_rock.png`, `red_castle.png`, `trees.png`, `clouds_anim.png` (RockMix). Skipped when graphics performance mode is LOW. |

## Not affected (no file with key pixels)

These surfaces are filled with a solid colour at runtime, so no pixel is the key colour:
fades (`draw::fade_in_screen` / `fade_out_screen`), lightning flash, inferno effect, `dark_surface` in the
stage select, `transparent_area` in `graphicsLib`, and `shared/images/black_line.png` (shadow effect, no key
pixels).

## Not related to this bug

Images with a real alpha channel (`weapon_menu_selection.png`, `jmd_logo.png`, `logo_android.png`,
`menu.png`) do not use the colorkey path in the same way.

## Still to check on the device

- Whether other opaque boxes on the PSP come from this path or from the PSP text path (menu cursor `>` is text).
- Whether the PC build shows the same box on the stage select cursor.
