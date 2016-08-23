/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup experimental_ble_sdk_app_eddystone_main main.c
 * @{
 * @ingroup experimental_ble_sdk_app_eddystone
 * @brief Eddystone Beacon UID Transmitter sample application main file.
 *
 * This file contains the source code for an Eddystone beacon transmitter sample application.
 */

#include <stdbool.h>
#include <stdint.h>
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"
#include "ble_fat.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                 /**< Include the service changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */

#define CENTRAL_LINK_COUNT              0                                 /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           0                                 /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define APP_CFG_NON_CONN_ADV_TIMEOUT    0                                 /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables the time-out. */
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(100, UNIT_0_625_MS) /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100 ms and 10.24 s). */

// Eddystone common data
#define APP_EDDYSTONE_UUID              0xFEAA                            /**< UUID for Eddystone beacons according to specification. */
#define APP_EDDYSTONE_RSSI              0xEE                              /**< 0xEE = -18 dB is the approximate signal strength at 0 m. */
#define APP_FATBEACON_URI               0x0E

// Eddystone URL data
#define APP_EDDYSTONE_URL_FRAME_TYPE    0x10                              /**< URL Frame type is fixed at 0x10. */
#define APP_EDDYSTONE_URL_SCHEME        0x00                              /**< 0x00 = "http://www" URL prefix scheme according to specification. */
#define APP_EDDYSTONE_URL_URL           'H', 'i', ' ', 'F', \
                                        'a', 't', 'b', 'e', \
                                        'a', 'c', 'o', 'n', \
                                        '!'

#define STATIC_BORING_WEBPAGE '<', 'h', 't', 'm', 'l', '>', '<', 'h', 'e', 'a', 'd', '>', \
                              '<', 't', 'i', 't', 'l', 'e', '>', 'F', 'a', 't', 'b', 'e', \
                              'a', 'c', 'o', 'n', '<', '/', 't', 'i', 't', 'l', 'e', '>', \
                              '<', '/', 'h', 'e', 'a', 'd', '>', '<', 'b', 'o', 'd', 'y', \
                              '>', '<', 'h', '1', '>', 'I', 't', ' ', 'w', 'o', 'r', 'k', \
                              'e', 'd', '!', '<', '/', 'h', '1', '>', '<', '/', 'b', 'o', \
                              'd', 'y', '>', '<', '/', 'h', 't', 'm', 'l', '>'

#define DEAD_BEEF                       0xDEADBEEF                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_TIMER_PRESCALER             0                                 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                 /**< Size of timer operation queues. */

static ble_gap_adv_params_t m_adv_params;                                 /**< Parameters to be passed to the stack when starting advertising. */
static ble_fat_t            m_ble_fat;

static uint8_t eddystone_url_data[] =   /**< Information advertised by the Eddystone URL frame type. */
{
    APP_EDDYSTONE_URL_FRAME_TYPE,   // Eddystone URL frame type.
    APP_EDDYSTONE_RSSI,             // RSSI value at 0 m.
    APP_FATBEACON_URI,             // Scheme or prefix for URL Fatbeacon
    APP_EDDYSTONE_URL_URL           // URL with a maximum length of 17 bytes. Last byte is suffix (".com", ".org", etc.)
};


/**@brief handler for BLE fatbeacon read event */
static void fat_read_evt_handler(ble_fat_t* p_fat, uint16_t value_handle)
{
    ret_code_t                            err_code;
    ble_gatts_rw_authorize_reply_params_t reply;

    static uint16_t last_data_pos = 0;
    uint8_t value_buffer[FAT_ADV_SLOT_CHAR_LENGTH_MAX] = {0};
    uint8_t page_data[] = {STATIC_BORING_WEBPAGE};
    ble_gatts_value_t value = {.len = sizeof(value_buffer), .offset = 0, .p_value = &(value_buffer[0])};

    memset(&reply, 0, sizeof(reply));
    //err_code = sd_ble_gatts_value_get(m_conn_handle, val_handle, &value);
    //APP_ERROR_CHECK(err_code);

    if (last_data_pos + FAT_ADV_SLOT_CHAR_LENGTH_MAX >= sizeof(page_data)) {
        reply.params.read.len = sizeof(page_data) - last_data_pos;
    } else {
        reply.params.read.len = FAT_ADV_SLOT_CHAR_LENGTH_MAX;
    }
    reply.params.read.len = value.len;
    reply.params.read.p_data = (const uint8_t *)page_data + last_data_pos;
    reply.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;

    last_data_pos += reply.params.read.len;

    err_code = sd_ble_gatts_rw_authorize_reply(value_handle, &reply);
    APP_ERROR_CHECK(err_code);
}

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for initializing the advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    ble_uuid_t    adv_uuids[] = {{APP_EDDYSTONE_UUID, BLE_UUID_TYPE_BLE}};

    uint8_array_t eddystone_data_array;                             // Array for Service Data structure.
/** @snippet [Eddystone data array] */
    eddystone_data_array.p_data = (uint8_t *) eddystone_url_data;   // Pointer to the data to advertise.
    eddystone_data_array.size = sizeof(eddystone_url_data);         // Size of the data to advertise.
/** @snippet [Eddystone data array] */

    ble_advdata_service_data_t service_data;                        // Structure to hold Service Data.
    service_data.service_uuid = APP_EDDYSTONE_UUID;                 // Eddystone UUID to allow discoverability on iOS devices.
    service_data.data = eddystone_data_array;                       // Array for service advertisement data.

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_NO_NAME;
    advdata.flags                   = flags;
    advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = adv_uuids;
    advdata.p_service_data_array    = &service_data;                // Pointer to Service Data structure.
    advdata.service_data_count      = 1;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
    m_adv_params.p_peer_addr = NULL;                                // Undirected advertisement.
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.timeout     = APP_CFG_NON_CONN_ADV_TIMEOUT;
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code;

    //err_code = sd_ble_gap_adv_start(&m_adv_params);
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    //ble_enable_params.common_enable_params.vs_uuid_count = 10;

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for doing power management.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    ble_fat_init_t fat_init;

    // Initialize.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    err_code = bsp_init(BSP_INIT_LED, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);

    ble_stack_init();

    memset(&fat_init, 0, sizeof(fat_init));
    fat_init.read_evt_handler = fat_read_evt_handler;

    err_code = ble_fat_init(&m_ble_fat, &fat_init);
    APP_ERROR_CHECK(err_code);

    advertising_init();
    LEDS_ON(LEDS_MASK);
    // Start execution.
    advertising_start();

    // Enter main loop.
    for (;; )
    {
        power_manage();
    }
}


/**
 * @}
 */
