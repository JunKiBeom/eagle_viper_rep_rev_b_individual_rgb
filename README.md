# Eagle/Viper REP Rev B — QMK/VIA 개별 LED 펌웨어

[English](README-EN.md) · 한국어

Eagle/Viper REP **Rev B**용 QMK/VIA 펌웨어입니다. 하단 RGB LED 16개의 색상과 밝기를 VIA에서 설정하고, 상단 표시 LED 8개가 켜지는 조건을 각각 선택할 수 있습니다. 기존 QMK RGBLight 효과도 모드 전환으로 사용할 수 있습니다.

원본 키보드 소스의 저작권 및 기여자는 [4pplet](https://github.com/4pplet)입니다. 이 저장소는 Rev B용 수정본입니다.

## 제작 배경과 목표

이 펌웨어는 **Codex의 GPT-6 Sol**을 사용해 원본 Eagle/Viper REP Rev B 펌웨어를 수정했습니다.

02D의 **Snow Tool**처럼 LED 색상을 하나씩 변경하고 싶었습니다. 또한 단종되고 지원이 종료되어 유지보수가 어려운 02D **Snow-Pro PCB**를 대체할 보드가 필요했습니다. Eagle/Viper REP Rev B를 그 용도에 적합한 우수한 대체품으로 판단해 이 프로젝트를 시작했습니다.

목표는 Snow Tool과 비슷한 설정 자유도와 사용성을 **별도 전용 프로그램 없이** 제공하는 것입니다. 최근 커스텀 키보드에서 널리 사용하는 **QMK/VIA**를 통해 LED 색상과 표시등 동작을 쉽게 제어할 수 있도록 했습니다.

## 파일 구성

| 폴더 | 내용 |
| --- | --- |
| [`qmk/rev_b/`](qmk/rev_b/) | QMK의 `rev_b` 위치에 복사할 VIA 펌웨어 소스 |
| [`via/`](via/) | VIA에 불러올 v7 JSON |
| [`firmware/`](firmware/) | 빌드된 Rev B용 `.bin` 파일 |
| [`reference/`](reference/) | 빌드에 필요하지 않은 원본 매트릭스 도표와 기본 키맵 참고본 |

## 요구사항

| 용도 | 요구사항 |
| --- | --- |
| 하드웨어 | **Eagle/Viper REP Rev B** PCB. Rev A 및 다른 키보드에는 이 바이너리를 사용하지 마세요. |
| VIA 설정 | [VIA](https://usevia.app/)와 [`via/eagle_viper_rep_rev_b_per_led_via_v7.json`](via/eagle_viper_rep_rev_b_per_led_via_v7.json) |
| 펌웨어 설치 | STM32 DFU 모드에서 `.bin` 파일을 플래시할 수 있는 도구 |
| 소스 빌드 | QMK 빌드 환경과 `4pplet/eagle_viper_rep2/rev_b` 키보드 경로 |

기본 USB 식별자는 VID `0x4444`, PID `0x0010`입니다. 펌웨어와 VIA JSON은 **같은 v7 버전**을 사용하세요.

## 원본 펌웨어 대비 변경점

| 항목 | 원본 Rev B 소스 | 이 수정본 v7 |
| --- | --- | --- |
| 하단 RGB LED 16개 | QMK RGBLight 효과, 전체 색상·밝기 조절 | VIA에서 LED마다 색상과 밝기 설정. `Individual LEDs`와 `QMK Lighting` 모드 선택 가능 |
| 상단 표시 LED 8개 | Lock/Layer 표시용 GPIO 정의 | LED마다 Caps Lock, Num Lock, Scroll Lock, Layer 0–4, On, Off 중 하나 선택 |
| VIA 레이아웃 | 원본 키맵 구조 | 원본 VIA JSON의 레이아웃 이름과 선택지를 반영한 v7 JSON 제공 |
| 설정 저장 | QMK/VIA 기본 설정 | 개별 RGB 및 상단 LED 설정을 VIA 커스텀 설정 영역에 저장 |

상단 LED의 `Layer 0–4` 조건은 **현재 가장 높은 활성 레이어**가 해당 번호일 때 켜집니다. `On`과 `Off`는 항상 켜짐과 항상 꺼짐입니다. 개별 RGB LED 16개의 초기 밝기는 최대값 `255`입니다.

### 상단 LED 번호와 GPIO

| VIA 메뉴 | 표시등 | 원본 핀 이름 | GPIO |
| --- | --- | --- | --- |
| DIP 1 | Caps Lock | `CAPS_LOCK` | `B1` |
| DIP 2 | Num Lock | `NUM_LOCK` | `B12` |
| DIP 3 | Scroll Lock | `SCROLL_LOCK` | `B13` |
| DIP 4 | Layer 0 | `LAYER_1` | `B14` |
| DIP 5 | Layer 1 | `LAYER_2` | `B15` |
| DIP 6 | Layer 2 | `LAYER_3` | `B0` |
| DIP 7 | Layer 3 | `LAYER_4` | `B9` |
| DIP 8 | Layer 4 | `LAYER_5` | `A9` |

VIA 메뉴 순서는 원본 표시등의 Caps Lock → Num Lock → Scroll Lock → Layer 0–4 순서를 따릅니다. 원본 핀 이름의 `LAYER_1`–`LAYER_5`는 각각 VIA의 Layer 0–4 표시등에 대응합니다.

## 설치 및 VIA 설정

1. [`firmware/4pplet_eagle_viper_rep2_rev_b_individual_rgb_via_v7.bin`](firmware/4pplet_eagle_viper_rep2_rev_b_individual_rgb_via_v7.bin)을 Rev B PCB에 플래시합니다. 원본 보드 안내에 따르면 MCU 쪽의 작은 버튼을 1초 이상 누르면 DFU 모드로 진입합니다. 기존 펌웨어가 실행 중이라면 Esc를 누른 채 USB를 연결하는 Bootmagic 방법도 사용할 수 있습니다.
2. VIA의 **Design** 탭에서 [`via/eagle_viper_rep_rev_b_per_led_via_v7.json`](via/eagle_viper_rep_rev_b_per_led_via_v7.json)을 불러옵니다.
3. **Lighting → Mode**에서 `Individual LEDs` 또는 `QMK Lighting`을 선택합니다.
4. `Individual LEDs`에서 하단 RGB 색상을, `LED Brightness`에서 각 LED의 밝기를 설정합니다. `DIP LEDs`에서 상단 LED 8개의 점등 조건을 선택합니다. `QMK Lighting`에서는 QMK RGBLight 효과와 전체 조명 설정을 사용합니다.

v6에서 v7으로 업그레이드하면 커스텀 EEPROM 데이터 형식이 달라져 이전 개별 RGB 설정이 초기화됩니다. 필요하다면 업그레이드 전에 설정값을 기록해 두세요. RGB LED 16개를 높은 밝기로 동시에 켜면 USB 전력 사용량이 늘어날 수 있습니다.

## 소스 빌드

[`qmk/rev_b/`](qmk/rev_b/)의 내용을 QMK 소스 트리의 `keyboards/4pplet/eagle_viper_rep2/rev_b/`에 배치한 뒤 실행합니다.

```sh
make 4pplet/eagle_viper_rep2/rev_b:via
```

제공된 v7 바이너리는 QMK 빌드에 성공했고, VIA JSON의 식별자·레이아웃·메뉴 구조를 확인했습니다. 실제 Rev B PCB에서 LED 점등, DIP LED의 물리적 순서, VIA 조작도 사용자 확인을 거쳤습니다.

[`reference/matrix_diagram.md`](reference/matrix_diagram.md)는 원본 Rev B의 키 매트릭스 도표와 동일합니다. LED/VIA 설정 변경으로 키 매트릭스가 바뀌지 않아 도표 내용은 수정하지 않았습니다.

## 참고 자료

- [QMK RGBLight](https://docs.qmk.fm/features/rgblight)
- [QMK LED Indicators](https://docs.qmk.fm/features/led_indicators)
- [QMK Layers](https://docs.qmk.fm/feature_layers)
- [VIA Custom UI](https://caniusevia.com/docs/custom_ui/)

## 라이선스

원본 소스 파일은 Stefan Sundin “4pplet”의 저작권 고지를 유지하며 **GPL-2.0-or-later**로 배포됩니다. [LICENSE](LICENSE)에는 GPL 버전 2의 전문을 수록했습니다. 펌웨어 바이너리를 배포할 때에는 대응하는 소스도 함께 제공하세요.
