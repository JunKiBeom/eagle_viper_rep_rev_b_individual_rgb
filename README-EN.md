# Eagle/Viper REP Rev B — QMK/VIA Per-LED Firmware

English · [한국어](README.md)

QMK/VIA firmware for the **Eagle/Viper REP Rev B**. VIA can set the color and brightness of each of the 16 bottom RGB LEDs and choose a trigger for each of the eight top indicator LEDs. You can switch back to the standard QMK RGBLight effects at any time.

The original keyboard source was created by [4pplet](https://github.com/4pplet). This repository contains a modified Rev B version.

## Motivation and goals

This firmware was modified from the original Eagle/Viper REP Rev B source using **Codex with GPT-6 Sol**.

I wanted to change LED colors individually, as I could with 02D's **Snow Tool**. I also needed an alternative to the 02D **Snow-Pro PCB**, which has been discontinued and is no longer supported, making maintenance difficult. I chose the Eagle/Viper REP Rev B because I consider it a strong replacement for that purpose.

The goal is to offer similar flexibility and ease of use **without a separate dedicated application**. The LED colors and indicator behavior can be controlled through **QMK/VIA**, tools widely used with modern custom keyboards.

## Repository layout

| Folder | Contents |
| --- | --- |
| [`qmk/rev_b/`](qmk/rev_b/) | VIA firmware source to copy into the QMK `rev_b` directory |
| [`via/`](via/) | v7 VIA definition JSON |
| [`firmware/`](firmware/) | Prebuilt Rev B `.bin` firmware |
| [`reference/`](reference/) | Original matrix diagram and default keymap, not required for the VIA build |

## Requirements

| Purpose | Requirement |
| --- | --- |
| Hardware | **Eagle/Viper REP Rev B** PCB. Do not flash this binary to Rev A or another keyboard. |
| VIA configuration | [VIA](https://usevia.app/) and [`via/eagle_viper_rep_rev_b_per_led_via_v7.json`](via/eagle_viper_rep_rev_b_per_led_via_v7.json) |
| Flashing | A tool capable of flashing a `.bin` file in STM32 DFU mode |
| Building from source | A QMK build environment with this keyboard at `keyboards/4pplet/eagle_viper_rep2/rev_b/` |

The default USB identifiers are VID `0x4444` and PID `0x0010`. Use the **v7 firmware and v7 VIA JSON together**.

## Changes from the original firmware

| Feature | Original Rev B source | This modified v7 firmware |
| --- | --- | --- |
| 16 bottom RGB LEDs | QMK RGBLight effects with global color and brightness controls | Set each LED's color and brightness in VIA; switch between `Individual LEDs` and `QMK Lighting` modes |
| Eight top indicator LEDs | GPIO definitions for Lock/Layer indicators | Choose Caps Lock, Num Lock, Scroll Lock, Layer 0–4, On, or Off for each LED independently |
| VIA layouts | Original keymap structure | v7 JSON includes the layout names and choices from the original VIA definition |
| Settings | Standard QMK/VIA settings | Individual RGB and top LED settings are saved in VIA's custom configuration area |

A `Layer 0–4` trigger lights its LED when that layer is the **highest active layer**. `On` and `Off` keep an LED permanently on or off. The default brightness for each bottom RGB LED is the maximum value, `255`.

### Top LED numbering and GPIO pins

| VIA control | Indicator | Original pin name | GPIO |
| --- | --- | --- | --- |
| DIP 1 | Caps Lock | `CAPS_LOCK` | `B1` |
| DIP 2 | Num Lock | `NUM_LOCK` | `B12` |
| DIP 3 | Scroll Lock | `SCROLL_LOCK` | `B13` |
| DIP 4 | Layer 0 | `LAYER_1` | `B14` |
| DIP 5 | Layer 1 | `LAYER_2` | `B15` |
| DIP 6 | Layer 2 | `LAYER_3` | `B0` |
| DIP 7 | Layer 3 | `LAYER_4` | `B9` |
| DIP 8 | Layer 4 | `LAYER_5` | `A9` |

The VIA menu follows the original indicator order: Caps Lock → Num Lock → Scroll Lock → Layer 0–4. The original pin names `LAYER_1`–`LAYER_5` correspond to VIA's Layer 0–4 indicators.

## Flashing and VIA setup

1. Flash [`firmware/4pplet_eagle_viper_rep2_rev_b_individual_rgb_via_v7.bin`](firmware/4pplet_eagle_viper_rep2_rev_b_individual_rgb_via_v7.bin) to the Rev B PCB. According to the original board instructions, hold the small button near the MCU for more than one second to enter DFU mode. If firmware is already running, you can also unplug the keyboard, hold Esc, and reconnect USB to use Bootmagic.
2. Load [`via/eagle_viper_rep_rev_b_per_led_via_v7.json`](via/eagle_viper_rep_rev_b_per_led_via_v7.json) in VIA's **Design** tab.
3. In **Lighting → Mode**, choose `Individual LEDs` or `QMK Lighting`.
4. Use `Individual LEDs` for bottom RGB colors, `LED Brightness` for each LED's brightness, and `DIP LEDs` for the eight top LED triggers. In `QMK Lighting` mode, use the QMK RGBLight effects and global lighting controls.

Upgrading from v6 to v7 changes the custom EEPROM format and resets the previous individual RGB settings. Record any colors and brightness levels you want to recreate before upgrading. Running all 16 RGB LEDs at high brightness can increase USB power consumption.

## Build from source

Place the contents of [`qmk/rev_b/`](qmk/rev_b/) in your QMK tree at `keyboards/4pplet/eagle_viper_rep2/rev_b/`, then run:

```sh
make 4pplet/eagle_viper_rep2/rev_b:via
```

The included v7 binary built successfully with QMK, and the VIA JSON identifiers, layout, and menu structure were checked. The user also confirmed LED operation, the physical DIP LED order, and VIA controls on an actual Rev B PCB.

[`reference/matrix_diagram.md`](reference/matrix_diagram.md) is identical to the original Rev B key matrix diagram. The LED and VIA changes do not alter the key matrix, so the diagram did not need editing.

## References

- [QMK RGBLight](https://docs.qmk.fm/features/rgblight)
- [QMK LED Indicators](https://docs.qmk.fm/features/led_indicators)
- [QMK Layers](https://docs.qmk.fm/feature_layers)
- [VIA Custom UI](https://caniusevia.com/docs/custom_ui/)

## License

The original source files retain Stefan Sundin “4pplet”'s copyright notices and are distributed under **GPL-2.0-or-later**. [LICENSE](LICENSE) contains the full text of GPL version 2. When distributing a firmware binary, provide its corresponding source code as well.
