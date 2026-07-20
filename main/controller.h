#pragma once
#include "pid.h"

// PID
#define PRESSURE_PID_SAMPLE_PERIOD_S   0.25f     /* control loop period [s] - was 5s, now runs more often */
#define PRESSURE_PID_SAMPLE_PERIOD_MS  ((uint32_t)(PRESSURE_PID_SAMPLE_PERIOD_S * 1000.0f))
#define VALVE_POS_MIN                  0.0f
#define VALVE_POS_MAX                  100.0f

#define PRESSURE_PID_KP                1.0f
#define PRESSURE_PID_TI                0.15f
#define PRESSURE_PID_TD                0.0f // No effect
#define PRESSURE_PID_I_LIMIT           30.0f
#define PRESSURE_DEADZONE              0.1f

#define PUMP_ON_ERROR -10
#define PUMP_OFF_ERROR -5

static epid_t     pressure_pid;
static epid_lpf_t pressure_lpf;
static bool       pressure_pid_ready = false;
static bool       auto_enabled_prev  = false;
static bool       pid_pump_on = false;

#define PRESSURE_LPF_SMOOTHING  0.3f /* 0 < a < 1; lower = smoother/slower to react to real changes */

void pressure_pid_init(void);
void pressure_pid_update(void);