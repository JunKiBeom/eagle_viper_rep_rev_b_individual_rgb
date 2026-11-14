/*
Copyright 2022 Stefan Sundin "4pplet" <4pplet@protonmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include QMK_KEYBOARD_H
#include "via.h"
#include "ws2812.h"
#include "rgblight.h"
#include "eeconfig.h"
#include "color.h"
#include "timer.h"
#include <string.h>

#define PER_LED_COUNT 16
#define DIP_LED_COUNT 8
#define PER_LED_FORMAT_VERSION 6

enum per_led_value_id {
    id_per_led_color = 1,
    id_per_led_brightness = 2,
    id_per_led_test = 3,
    id_per_led_level = 4,
    id_led_control_mode = 5,
    id_dip_led_mode = 6,
};

typedef struct {
    uint8_t version;
    uint8_t mode;
    uint8_t hue[PER_LED_COUNT];
    uint8_t saturation[PER_LED_COUNT];
    uint8_t led_brightness[PER_LED_COUNT];
    uint8_t dip_mode[DIP_LED_COUNT];
} per_led_config_t;

_Static_assert(sizeof(per_led_config_t) == VIA_EEPROM_CUSTOM_CONFIG_SIZE, "VIA custom config size mismatch");

static per_led_config_t per_led_config;
static bool per_led_dirty;
static bool per_led_new_defaults;
static bool per_led_test_active;
static uint32_t per_led_test_started;
static const pin_t dip_led_pins[DIP_LED_COUNT] = {
    LAYER_1, LAYER_2, LAYER_3, LAYER_4, LAYER_5,
    CAPS_LOCK, NUM_LOCK, SCROLL_LOCK,
};

static void dip_led_render(layer_state_t layers, layer_state_t default_layers, led_t locks) {
    uint8_t active_layer = get_highest_layer(layers | default_layers);
    for (uint8_t i = 0; i < DIP_LED_COUNT; ++i) {
        uint8_t choice = per_led_config.dip_mode[i];
        bool on = false;
        switch (choice) {
            case 0: on = locks.caps_lock; break;
            case 1: on = locks.num_lock; break;
            case 2: on = locks.scroll_lock; break;
            case 3: case 4: case 5: case 6: case 7:
                on = active_layer == (uint8_t)(choice - 3);
                break;
            case 8: on = true; break;
            case 9: on = false; break;
        }
        gpio_write_pin(dip_led_pins[i], on);
    }
}

static void per_led_defaults(void) {
    memset(&per_led_config, 0, sizeof(per_led_config));
    per_led_config.version = PER_LED_FORMAT_VERSION;
    per_led_config.mode = 0;
    memset(per_led_config.led_brightness, 255, sizeof(per_led_config.led_brightness));
    memset(per_led_config.dip_mode, 8, sizeof(per_led_config.dip_mode));
    per_led_dirty = true;
    per_led_new_defaults = true;
}

static void per_led_save(void) {
    if (per_led_dirty) {
        via_update_custom_config(&per_led_config, 0, sizeof(per_led_config));
        per_led_dirty = false;
    }
}

static void per_led_show(void) {
    for (uint8_t i = 0; i < PER_LED_COUNT; ++i) {
        uint8_t value = per_led_config.led_brightness[i];
        rgb_t rgb = hsv_to_rgb((hsv_t){per_led_config.hue[i], per_led_config.saturation[i], value});
        ws2812_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    ws2812_flush();
}

static void per_led_test(void) {
    per_led_test_active = true;
    per_led_test_started = timer_read32();
    ws2812_set_color_all(16, 16, 16);
    ws2812_set_color(0, 128, 0, 0);
    ws2812_set_color(1, 0, 128, 0);
    ws2812_set_color(2, 0, 0, 128);
    ws2812_flush();
}

static void per_led_apply_mode(void) {
    per_led_test_active = false;
    if (per_led_config.mode == 0) {
        rgblight_disable_noeeprom();
        per_led_show();
    } else {
        // Reload QMK's last saved effect, color, brightness, and on/off state.
        rgblight_reload_from_eeprom();
        if (!rgblight_is_enabled()) {
            ws2812_set_color_all(0, 0, 0);
            ws2812_flush();
        }
    }
}

void via_init_kb(void) {
    if (via_eeprom_is_valid() &&
        via_read_custom_config(&per_led_config, 0, sizeof(per_led_config)) == sizeof(per_led_config) &&
        per_led_config.version == PER_LED_FORMAT_VERSION) {
        per_led_dirty = false;
        per_led_new_defaults = false;
    } else {
        per_led_defaults();
    }
}

void keyboard_post_init_user(void) {
    // VIA may erase its EEPROM region after via_init_kb() on the first boot.
    if (per_led_new_defaults) {
        // Preserve QMK's effect and color while restoring full default brightness.
        rgblight_config_t qmk_lighting;
        eeconfig_read_rgblight(&qmk_lighting);
        qmk_lighting.val = 255;
        eeconfig_update_rgblight(&qmk_lighting);
        rgblight_reload_from_eeprom();
    }
    per_led_save();
    per_led_apply_mode();
    dip_led_render(layer_state, default_layer_state, host_keyboard_led_state());
}

bool led_update_user(led_t led_state) {
    dip_led_render(layer_state, default_layer_state, led_state);
    return true;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    dip_led_render(state, default_layer_state, host_keyboard_led_state());
    return state;
}

layer_state_t default_layer_state_set_user(layer_state_t state) {
    dip_led_render(layer_state, state, host_keyboard_led_state());
    return state;
}

void via_set_device_indication(uint8_t value) {
    if (per_led_config.mode != 0) {
        rgblight_toggle_noeeprom();
    } else if ((value & 1) == 0) {
        ws2812_set_color_all(0, 0, 0);
        ws2812_flush();
    } else {
        per_led_show();
    }
}

void housekeeping_task_user(void) {
    if (per_led_test_active && timer_elapsed32(per_led_test_started) > 1500) {
        per_led_test_active = false;
        per_led_show();
    }
    if (per_led_config.mode == 0 && rgblight_is_enabled()) {
        rgblight_disable_noeeprom();
        per_led_show();
    }
}

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    if (length < 3 || data[1] != id_custom_channel) {
        data[0] = id_unhandled;
        return;
    }

    switch (data[0]) {
        case id_custom_set_value:
            if (data[2] == id_per_led_color && length >= 6 && data[3] < PER_LED_COUNT) {
                uint8_t i = data[3];
                per_led_config.hue[i] = data[4];
                per_led_config.saturation[i] = data[5];
            } else if (data[2] == id_per_led_level && length >= 5 && data[3] < PER_LED_COUNT) {
                per_led_config.led_brightness[data[3]] = data[4];
            } else if (data[2] == id_led_control_mode && length >= 4 && data[3] <= 1) {
                per_led_config.mode = data[3];
                per_led_dirty = true;
                per_led_apply_mode();
                break;
            } else if (data[2] == id_dip_led_mode && length >= 5 && data[3] < DIP_LED_COUNT && data[4] <= 9) {
                per_led_config.dip_mode[data[3]] = data[4];
                per_led_dirty = true;
                dip_led_render(layer_state, default_layer_state, host_keyboard_led_state());
                break;
            } else if (data[2] == id_per_led_test && length >= 4 && data[3] == 1 && per_led_config.mode == 0) {
                per_led_test();
                break;
            } else {
                data[0] = id_unhandled;
                return;
            }
            per_led_dirty = true;
            per_led_test_active = false;
            if (per_led_config.mode == 0) {
                per_led_show();
            }
            break;

        case id_custom_get_value:
            if (data[2] == id_per_led_color && length >= 6 && data[3] < PER_LED_COUNT) {
                uint8_t i = data[3];
                data[4] = per_led_config.hue[i];
                data[5] = per_led_config.saturation[i];
            } else if (data[2] == id_per_led_level && length >= 5 && data[3] < PER_LED_COUNT) {
                data[4] = per_led_config.led_brightness[data[3]];
            } else if (data[2] == id_led_control_mode && length >= 4) {
                data[3] = per_led_config.mode;
            } else if (data[2] == id_dip_led_mode && length >= 5 && data[3] < DIP_LED_COUNT) {
                data[4] = per_led_config.dip_mode[data[3]];
            } else if (data[2] == id_per_led_test && length >= 4) {
                // VIA reads every custom control while loading the menu, including buttons.
                data[3] = 0;
            } else {
                data[0] = id_unhandled;
            }
            break;

        case id_custom_save:
            per_led_save();
            break;

        default:
            data[0] = id_unhandled;
            break;
    }
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
// main layer
[0] = LAYOUT_all(
  KC_ESC,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,       KC_6,      KC_7,      KC_8,      KC_9,       KC_0,    KC_MINS, KC_EQL,  KC_BSLS, KC_GRV,
  KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,       KC_Y,      KC_U,      KC_I,      KC_O,       KC_P,    KC_LBRC, KC_RBRC, KC_BSPC,
  KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,       KC_H,      KC_J,      KC_K,      KC_L,       KC_SCLN, KC_QUOT, KC_NUHS, KC_ENT,
  KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,       KC_B,      KC_N,      KC_M,      KC_COMM,    KC_DOT,  KC_SLSH, KC_RSFT, MO(1),
  KC_LCTL,  KC_LGUI,  KC_LALT,                           KC_SPC, KC_SPC, KC_SPC,                                KC_RALT,      KC_RALT, KC_RGUI, MO(1)),
// basic function layer
[1] = LAYOUT_all(
  QK_BOOT,  KC_F1,    KC_F2,    KC_F3,    KC_F4,     KC_F5,     KC_F6,     KC_F7,     KC_F8,     KC_F9,      KC_F10,  KC_F11,   KC_F12,  KC_TRNS, KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_UP,    KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS,
  KC_TRNS,  KC_LEFT,  KC_DOWN,  KC_RIGHT, KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,                          KC_TRNS, KC_TRNS, KC_TRNS,                             KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS),
// extra layer for VIA
[2] = LAYOUT_all(
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS, KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,                          KC_TRNS, KC_TRNS, KC_TRNS,                            KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS),
// extra layer for VIA
[3] = LAYOUT_all(
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS, KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,                          KC_TRNS, KC_TRNS, KC_TRNS,                            KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS),
// extra layer for VIA
[4] = LAYOUT_all(
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS, KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,   KC_TRNS,    KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS,
  KC_TRNS,  KC_TRNS,  KC_TRNS,                         KC_TRNS, KC_TRNS, KC_TRNS,                             KC_TRNS,  KC_TRNS,  KC_TRNS,  KC_TRNS)
};
