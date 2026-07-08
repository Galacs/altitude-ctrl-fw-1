#include <stdio.h>

// #include "can_manager.h"

// CAN_STRUCT(HeartbeatMsg, 0x200,
//     uint8_t  counter;
//     uint16_t checksum;
// );

// void on_heartbeat(const can_frame_t *frame) {
//     const HeartbeatMsg *msg = (const HeartbeatMsg *)frame->data;
//     ESP_LOGI("APP", "Heartbeat: cnt=%u, chk=%u", msg->counter, msg->checksum);
// }


void app_main(void) {
    // printf("bonsoir\n");
    // can_manager_t can_mgr;


    // if (!can_manager_init(&can_mgr, GPIO_NUM_21, GPIO_NUM_20, 500000)) {
    //     ESP_LOGE("APP", "CAN init failed");
    //     return;
    // }

    // can_manager_register_callback(&can_mgr, HeartbeatMsg_CAN_ID, on_heartbeat);

    // HeartbeatMsg hb = { .counter = 0, .checksum = 0xABCD };
    // CAN_SEND_STRUCT(&can_mgr, HeartbeatMsg, hb);

    // while (1) {
    //     can_manager_update(&can_mgr);   // dispatches all received frames
    //     vTaskDelay(pdMS_TO_TICKS(10));
    // }
    printf("Hello world!\n");
}
