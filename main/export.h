#pragma once

#include <esp_lv_adapter.h>

void export_ui_init(void);      /* call once after main_create(), like profiles_ui_init() */
void export_record_tick(void);  /* call every ~10ms from the main loop, like pressure_pid_update() */