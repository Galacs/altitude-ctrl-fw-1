#include <string.h>
#include "esp_err.h"

#include "can_manager.h"

#define RX_QUEUE_LENGTH  20   /* number of can_frame_t slots */

/* -------------------------------------------------------------------------- */
/* ISR callback: receives frame and pushes a copy into the queue              */
/* -------------------------------------------------------------------------- */
static bool rx_done_isr(twai_node_handle_t handle,
                                  const twai_rx_done_event_data_t *edata,
                                  void *user_ctx)
{
    can_manager_t *mgr = (can_manager_t *)user_ctx;
    can_frame_t frame;
    uint8_t buf[8];
    twai_frame_t rx_frame = {
        .buffer     = buf,
        .buffer_len = sizeof(buf),
    };

    if (twai_node_receive_from_isr(handle, &rx_frame) != ESP_OK) {
        return false;
    }

    frame.id     = rx_frame.header.id;
    frame.length = (rx_frame.header.dlc <= 8) ? rx_frame.header.dlc : 8;  // DLC mapping
    frame.rtr    = rx_frame.header.rtr;
    memcpy(frame.data, rx_frame.buffer, frame.length);

    BaseType_t woken = pdFALSE;
    xQueueSendFromISR(mgr->rx_queue, &frame, &woken);
    return (woken == pdTRUE);
}

/* -------------------------------------------------------------------------- */
bool can_manager_init(can_manager_t *mgr, gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t bitrate)
{
    if (!mgr) return false;
    memset(mgr, 0, sizeof(*mgr));

    // Create queue for passing frames from ISR to task
    mgr->rx_queue = xQueueCreate(RX_QUEUE_LENGTH, sizeof(can_frame_t));
    if (mgr->rx_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create RX queue");
        return false;
    }

    twai_onchip_node_config_t node_config = {
        .io_cfg.tx = tx_pin,
        .io_cfg.rx = rx_pin,
        .io_cfg.quanta_clk_out = GPIO_NUM_NC,
        .io_cfg.bus_off_indicator = GPIO_NUM_NC,
        .bit_timing.bitrate = bitrate,
        .tx_queue_depth = 10,
        .flags.enable_loopback = true,
        .flags.enable_self_test = true,
    };

    esp_err_t err = twai_new_node_onchip(&node_config, &mgr->node);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create TWAI node: %s", esp_err_to_name(err));
        vQueueDelete(mgr->rx_queue);
        return false;
    }

    // Register the receive callback, passing mgr as user context
    twai_event_callbacks_t cbs = {
        .on_rx_done = rx_done_isr,
    };
    err = twai_node_register_event_callbacks(mgr->node, &cbs, mgr);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register callbacks: %s", esp_err_to_name(err));
        twai_node_delete(mgr->node);
        vQueueDelete(mgr->rx_queue);
        return false;
    }

    err = twai_node_enable(mgr->node);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable TWAI node: %s", esp_err_to_name(err));
        twai_node_delete(mgr->node);
        vQueueDelete(mgr->rx_queue);
        return false;
    }

    ESP_LOGI(TAG, "TWAI driver initialised @ %lu bit/s", bitrate);
    return true;
}

/* -------------------------------------------------------------------------- */
void can_manager_send(can_manager_t *mgr, uint32_t id, const void *data, size_t len)
{
    if (len > 8) {
        ESP_LOGE(TAG, "Payload too long (%u bytes)", len);
        return;
    }

    uint8_t tx_buf[8];
    memcpy(tx_buf, data, len);

    twai_frame_t tx_frame = {
        .header.id = id,
        .header.dlc = len,          // Classic CAN: DLC equals length
        .buffer     = tx_buf,
        .buffer_len = len,
    };

    esp_err_t err = twai_node_transmit(mgr->node, &tx_frame, pdMS_TO_TICKS(1000));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to queue TX (ID 0x%lx): %s", id, esp_err_to_name(err));
        return;
    }

    // Wait until transmission finishes (blocking)
    err = twai_node_transmit_wait_all_done(mgr->node, pdMS_TO_TICKS(1000));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "TX wait failed (ID 0x%lx): %s", id, esp_err_to_name(err));
    }
}

/* -------------------------------------------------------------------------- */
bool can_manager_register_callback(can_manager_t *mgr, uint32_t id, can_callback_t cb)
{
    if (mgr->callback_count >= CALLBACKS_NUMBER) {
        ESP_LOGE(TAG, "Too many callbacks (max %d)", CALLBACKS_NUMBER);
        return false;
    }
    mgr->callbacks[mgr->callback_count].id       = id;
    mgr->callbacks[mgr->callback_count].function = cb;
    mgr->callback_count++;
    ESP_LOGI(TAG, "Callback registered for ID 0x%lx", id);
    return true;
}

/* -------------------------------------------------------------------------- */
void can_manager_update(can_manager_t *mgr)
{
    can_frame_t frame;
    while (xQueueReceive(mgr->rx_queue, &frame, 0) == pdTRUE) {
        for (uint8_t i = 0; i < mgr->callback_count; i++) {
            if (mgr->callbacks[i].id == frame.id) {
                mgr->callbacks[i].function(&frame);
            }
        }
    }
}