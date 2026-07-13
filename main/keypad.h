#pragma once

#include <esp_lv_adapter.h>
#include "ui/altitude_ctrl_ui_1_mini.h"

#define KB_COLS 3
#define KB_ROWS 4
#define KB_KEY_SIZE 80
#define KB_PAD 10

void pump_target_keypad_open(lv_event_t * e);