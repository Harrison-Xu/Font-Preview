# Font Preview

A 320×170 typography preview application for CardputerZero, built with C++17, LVGL 9.5, SDL2/SDL2_mixer, and FreeType.

The screen has a compact parameter panel on the left and a scrollable, full-height text preview on the right. Interface labels use the bundled Noto CJK fonts; preview text can use Noto CJK or one of four Debian system font families.

## Preview options

- Preview content: Simplified Chinese, Traditional Chinese, Japanese, Korean, English, Portuguese, Czech, Greek, Russian, and Code
- Fonts: Noto CJK, Go, Inter, DejaVu, and JetBrains Mono
- Typefaces: Sans, Sans Italic, Serif, Serif Italic, Mono, and Mono Italic
- Sizes: 8–24 px in 1 px steps, stopping at either boundary; hold Left/Right for rapid adjustment
- Weights: Regular and Bold
- Colors: Black/White, White/Black, Sepia, Terminal, Solarized, Navy, Amber, and Slate

## Controls

| CardputerZero | Desktop | Action |
| --- | --- | --- |
| `F` | Up arrow or `F` | Select the previous parameter |
| `X` | Down arrow or `X` | Select the next parameter |
| `Z` | Left arrow or `Z` | Previous value |
| `C` | Right arrow or `C` | Next value |
| `L` | `L` | Scroll preview up |
| `M` | `M` | Scroll preview down |
| Esc | Esc | Exit |

## Interface sounds

Font Preview uses three short cues from the CC0 [UI SFX](https://uisfx.com/) Arcade pack. `select` confirms a successful field or value change, `blocked` marks a size boundary or unsupported typeface, and `long-press` announces entry into rapid size adjustment. Ordinary preview scrolling and every repeated size step stay silent to avoid noisy high-frequency feedback.

Arcade feedback is always enabled when the audio device is available. Audio reinforces the existing visual response and is never the only status signal. The bundled audio is dedicated to the public domain under CC0-1.0; see `assets/audio/LICENSE-UISFX-AUDIO.txt`.

## Fonts

The project bundles the complete Regular and Bold Noto CJK faces from Debian 13 (trixie) package `fonts-noto-cjk` version `1:20240730+repack1-1`:

- `NotoSansCJK-Regular.ttc`
- `NotoSansCJK-Bold.ttc`
- `NotoSerifCJK-Regular.ttc`
- `NotoSerifCJK-Bold.ttc`

The application selects the correct regional face inside each collection: JP for Japanese, KR for Korean, SC for Simplified Chinese, and TC for Traditional Chinese. Mono uses the matching regional Noto Sans Mono face already contained in the Sans TTC files, so separate Mono OTF copies are unnecessary.

The fonts are licensed under the SIL Open Font License 1.1. See `assets/fonts/LICENSE-NOTO-CJK.txt`.

Go, Inter, DejaVu, and JetBrains Mono are loaded from their standard Debian system paths. The application package depends on `fonts-go`, `fonts-inter`, `fonts-dejavu-core`, `fonts-dejavu-extra`, `fonts-dejavu-mono`, and `fonts-jetbrains-mono`. Unsupported combinations are reported directly in the preview—for example, Inter does not provide Serif or Mono faces. Noto CJK italic selections use FreeType's synthetic italic rendering because Noto CJK does not ship native italic faces.

## Character coverage

The Latin, Greek, and Cyrillic previews begin with explicit locale character rows based on Unicode CLDR, followed by natural text or pangrams. The application intentionally does not substitute a fallback font: a missing-glyph box is part of the test result. Character coverage therefore varies with the selected family, making it possible to compare the broad DejaVu coverage with the more specialized Inter, Go, JetBrains Mono, and Noto CJK families.

## Build

Configure and build the macOS desktop preview:

```shell
cmake --preset darwin-arm64
cmake --build --preset darwin-arm64-dbg
./build/darwin-arm64/Debug/font_preview
```

Build the CardputerZero ARM64 Debian package:

```shell
cmake --preset cp0-cross
cmake --build --preset cp0-cross-rel
cpack --preset cp0-cross-deb
```

The package is written to `dist/FontPreview_0.4.0_m5stack1_arm64.deb` and installs Noto CJK fonts below `/usr/share/font_preview/fonts/`.
