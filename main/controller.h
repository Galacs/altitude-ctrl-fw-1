#pragma once
#include "pid.h"
#include <stdbool.h>
#include <stdint.h>

#define PRESSURE_CTRL_SAMPLE_PERIOD_S   0.25f
#define PRESSURE_CTRL_SAMPLE_PERIOD_MS  ((uint32_t)(PRESSURE_CTRL_SAMPLE_PERIOD_S * 1000.0f))

#define VALVE_POS_MIN                   0.0f
#define VALVE_POS_MAX                   100.0f
#define VALVE_RATE_LIMIT_PER_S          4.0f
#define VALVE_RATE_LIMIT_PER_CYCLE      (VALVE_RATE_LIMIT_PER_S * PRESSURE_CTRL_SAMPLE_PERIOD_S)

#define PRESSURE_PID_KP                 0.6f
#define PRESSURE_PID_TI                 3.0f
#define PRESSURE_PID_TD                 0.0f
#define PRESSURE_PID_I_LIMIT            12.0f

#define PRESSURE_DEADZONE               0.0f

#define FF_BLEND_NEAR                   2.0f
#define FF_BLEND_FAR                    5.0f
#define FF_WEIGHT_AT_FULL               0.6f

#define PUMP_ON_ERROR_THRESHOLD         -1.5f
#define PUMP_OFF_ERROR_THRESHOLD        -0.5f
#define PUMP_MIN_RUN_TIME_S             5.0f
#define PUMP_MIN_OFF_TIME_S             3.0f
#define PUMP_MAX_PRESSURE_CUTOFF        85.0f

#define PRESSURE_LPF_SMOOTHING          0.3f

#define FF_NUM_POINTS                   3
static const float ff_pressure_points[FF_NUM_POINTS] = {45.0f, 35.0f, 25.0f};
static const float ff_valve_points[FF_NUM_POINTS]   = {20.0f,  5.0f,  0.0f};

static epid_t     pressure_pid;
static epid_lpf_t pressure_lpf;
static bool       pressure_ctrl_ready = false;
static bool       auto_enabled_prev   = false;

static bool       pump_state          = false;
static int64_t    pump_last_toggle_us = 0;
static float      valve_output_prev   = 0.0f;

void pressure_ctrl_init(void);
void pressure_ctrl_update(void);