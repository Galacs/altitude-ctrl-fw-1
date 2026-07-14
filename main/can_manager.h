#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "driver/gpio.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CALLBACKS_NUMBER   10

/* ---------- CAN frame representation ---------- */
typedef struct {
    uint32_t id;
    uint8_t  data[8];
    uint8_t  length;
    bool     rtr;
} can_frame_t;

/* ---------- Message definition macro (no _Static_assert) ---------- */
/**
 * @brief Define a packed CAN message structure with an associated CAN ID.
 *
 * @param struct_name  Name of the struct (typedef’d).
 * @param can_id       CAN identifier (creates enum struct_name##_CAN_ID).
 * @param ...          Struct members.
 *
 * The struct is packed and its size is checked at compile time (≤ 8 bytes).
 */
#define CAN_STRUCT(struct_name, can_id, ...)                                     \
    typedef struct __attribute__((packed)) {                                     \
        __VA_ARGS__                                                              \
    } struct_name;                                                               \
    enum { struct_name ## _CAN_ID = can_id };                                    \
    typedef char struct_name ## _size_check[sizeof(struct_name) <= 8 ? 1 : -1]   \
        __attribute__((unused))

/* ---------- Convenience send macro ---------- */
/**
 * @brief Send a typed CAN message.
 *
 * @param mgr   Pointer to an initialised can_manager_t.
 * @param stype The struct type (e.g. MyMsg).
 * @param msg   Variable of that type.
 */
#define CAN_SEND_STRUCT(mgr, stype, msg)                                        \
    do {                                                                         \
        stype *__can_send_ptr = &(msg);  /* type check */                       \
        (void)__can_send_ptr;                                                    \
        can_manager_send((mgr), stype ## _CAN_ID, __can_send_ptr, sizeof(stype)); \
    } while(0)

/* ---------- Callback type ---------- */
typedef void (*can_callback_t)(const can_frame_t *frame);

/* ---------- Manager structure ---------- */
typedef struct {
    can_callback_t function;
    uint32_t       id;
} can_callback_entry_t;

typedef struct {
    twai_node_handle_t   node;
    QueueHandle_t        rx_queue;
    can_callback_entry_t callbacks[CALLBACKS_NUMBER];
    uint8_t              callback_count;
} can_manager_t;

/* ---------- Public API ---------- */
/**
 * @brief Initialise the TWAI peripheral and start the driver.
 *
 * @param mgr     Pointer to an uninitialised can_manager_t.
 * @param tx_pin  GPIO pin for TX.
 * @param rx_pin  GPIO pin for RX.
 * @param bitrate Bus bit rate (e.g. 500000 for 500 kbit/s).
 * @return true on success.
 */
bool can_manager_init(can_manager_t *mgr, gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t bitrate);

/**
 * @brief Blocking send of raw CAN data. The call only returns after transmission completes.
 *
 * @param mgr  Initialised manager.
 * @param id   CAN identifier.
 * @param data Pointer to the payload.
 * @param len  Payload length (0–8).
 */
void can_manager_send(can_manager_t *mgr, uint32_t id, const void *data, size_t len);

/**
 * @brief Register a callback for a specific CAN ID.
 *
 * @param mgr Initialised manager.
 * @param id  CAN identifier to watch.
 * @param cb  Function to call when a frame with this ID is received.
 * @return true if registered (max CALLBACKS_NUMBER).
 */
bool can_manager_register_callback(can_manager_t *mgr, uint32_t id, can_callback_t cb);

/**
 * @brief Non‑blocking update: dispatch all queued frames to their callbacks.
 *
 * Call this repeatedly (e.g. in a task loop).
 *
 * @param mgr Initialised manager.
 */
void can_manager_update(can_manager_t *mgr);

#ifdef __cplusplus
}
#endif