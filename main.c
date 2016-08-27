
/*****************************************************************************
*
* main.c
*
* Entrypoint for the nRF52 Fatbeacon implementation.  This project is based on 
* the experimental_ble_app_eddystone_132_pca10040 as distributed in the nRF5 v11
* SDK.  Portions by Matt licensed under BSD.
*
* Copyright (c) 2016 Matt Roche
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

#include <stdbool.h>
#include <stdint.h>
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"
#include "ble_fat.h"
#include "fatbeacon.h"
#include "SEGGER_RTT.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                 /**< Include the service changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */

#define CENTRAL_LINK_COUNT              0                                 /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                 /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define APP_CFG_NON_CONN_ADV_TIMEOUT    0                                 /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables the time-out. */
#define APP_CFG_CONNECTABLE_ADV_TIMEOUT         60  
#define NON_CONNECTABLE_ADV_INTERVAL            MSEC_TO_UNITS(100, UNIT_0_625_MS) /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100 ms and 10.24 s). */
#define APP_CFG_CONNECTABLE_ADV_INTERVAL_MS     100     

#define MIN_CONN_INTERVAL                       MSEC_TO_UNITS(50, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL                       MSEC_TO_UNITS(90, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                           0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                        MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY          APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY           APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT            3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_TIMER_PRESCALER             0                                 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                 /**< Size of timer operation queues. */

static ble_gap_adv_params_t m_adv_params;                                 /**< Parameters to be passed to the stack when starting advertising. */
static ble_fat_t            m_ble_fat;
static uint8_t *            m_page_data = (uint8_t *) STATIC_PAGE; //{STATIC_BORING_WEBPAGE};
static uint16_t             m_conn_handle = BLE_CONN_HANDLE_INVALID;
static int16_t              m_last_data_pos = 0;

static uint8_t eddystone_url_data[] =   /**< Information advertised by the Eddystone Fatbeacon frame type. */
{
    APP_EDDYSTONE_URL_FRAME_TYPE,   // Eddystone URL frame type.    (Same for URL and Fatbeacon)
    APP_EDDYSTONE_RSSI,             // RSSI value at 0 m.
    APP_FATBEACON_URI,              // Scheme or prefix for URL Fatbeacon
    APP_FATBEACON_NAME              // Description displayed by the Fatbeacon, max 18 chars
};

static void advertising_start(void);

/**@brief handler for BLE fatbeacon read event 
 * 
 * @details This handler captures the read request for the fatbeacon characteristic value.
 *          It does not care what the initial values are.  Instead, to work with the current 
 *          PWA implementation, it merely returns the static page value 20 bytes at a time,
 *          incrementing its own internal offset with each read.  It continues until the last
 *          bytes are read, setting the offset to -1 which will cause it to send a 0 byte reply
 *          (if requested).  Once complete, the PWA app should close the connection.    
 *          The offset is reset by any Disconnect event, or the act of reading past the end of the 
 *          data.  
 *
 *          This doesn't follow the normal GATT spec, so this is only for testing / compatability
 *          with the PWA app. 
*/
static void fat_read_evt_handler(ble_fat_t* p_fat, uint16_t value_handle)
{   
    ret_code_t                            err_code;
    ble_gatts_rw_authorize_reply_params_t reply;

    uint16_t page_size = strlen((char *)m_page_data);
    
    if (page_size > 10000) {
        page_size = 10000;      // Imposing an arbitrary limit on page size.
    }

    memset(&reply, 0, sizeof(reply));
    reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;

    if (m_last_data_pos >= 0) // Active request
    {
        if (m_last_data_pos + FAT_CHAR_MAX_LEN >= page_size) {
            reply.params.read.len = page_size - m_last_data_pos;
        } else {
            reply.params.read.len = FAT_CHAR_MAX_LEN;
        }

        reply.params.read.p_data      = m_page_data + m_last_data_pos;        
        reply.params.read.update      = 1;
        reply.params.read.offset      = 0;
        reply.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;

        if (m_last_data_pos + FAT_CHAR_MAX_LEN < page_size) {
            m_last_data_pos += reply.params.read.len;
        } else {
            m_last_data_pos = -1;
        }
    } else {    // Send empty response as last packet
        reply.params.read.p_data      = NULL;
        reply.params.read.len         = 0;
        reply.params.read.update      = 1;
        reply.params.read.offset      = 0;
        reply.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;
        m_last_data_pos = 0; 
    }

    //SEGGER_RTT_printf(0, "Reply Char len: %d of %d at: 0x%x", reply.params.read.len, page_size, m_page_data);
    err_code = sd_ble_gatts_rw_authorize_reply(m_conn_handle, &reply);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "GATT Reply Error %d\n", err_code);
    }
    
}

static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
            {
        case BLE_GAP_EVT_CONNECTED:            
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            SEGGER_RTT_printf(0,"Got BLE Connection. Handle: %d\n", m_conn_handle);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            SEGGER_RTT_printf(0,"BLE Handle: %d Disconnected.\n", m_conn_handle);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            
            m_last_data_pos = 0;            // Reset FAT Characteristic read on disconnect.
             
            advertising_start();            // Restart the advertising
            break;

        case BLE_GAP_EVT_TIMEOUT:
              
            advertising_start();            // Restart advertising on timeout.
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Primary dispatch function for BLE events
*/
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_fat_on_ble_evt(&m_ble_fat, p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
}

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    ble_advertising_on_sys_evt(sys_evt);
}

static void gap_params_init(void)
{
   uint32_t                err_code;
   ble_gap_conn_params_t   gap_conn_params;
   ble_gap_conn_sec_mode_t sec_mode;

   BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

   err_code = sd_ble_gap_device_name_set(&sec_mode,
                                         (const uint8_t *)DEVICE_NAME,
                                         strlen(DEVICE_NAME));
   APP_ERROR_CHECK(err_code);

   memset(&gap_conn_params, 0, sizeof(gap_conn_params));

   gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
   gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
   gap_conn_params.slave_latency     = SLAVE_LATENCY;
   gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

   err_code = sd_ble_gap_ppcp_set(&gap_conn_params);

   APP_ERROR_CHECK(err_code);
}

static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = true;

    err_code = ble_conn_params_init(&cp_init);
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
    ble_advdata_t adv_data;
    ble_advdata_t scrsp_data;
    uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    ble_uuid_t    adv_uuids[] = {{APP_EDDYSTONE_UUID, BLE_UUID_TYPE_BLE}};
    ble_uuid_t    scrp_uuids[] = {{BLE_UUID_FAT_URL_SERVICE, m_ble_fat.uuid_type}};

    uint8_array_t eddystone_data_array;                             // Array for Service Data structure.
/** @snippet [Eddystone data array] */
    eddystone_data_array.p_data = (uint8_t *) eddystone_url_data;   // Pointer to the data to advertise.
    eddystone_data_array.size = sizeof(eddystone_url_data);         // Size of the data to advertise.
/** @snippet [Eddystone data array] */

    ble_advdata_service_data_t service_data;                        // Structure to hold Service Data.
    service_data.service_uuid = APP_EDDYSTONE_UUID;                 // Eddystone UUID to allow discoverability on iOS devices.
    service_data.data = eddystone_data_array;                       // Array for service advertisement data.

    // Build and set advertising data.
    memset(&adv_data, 0, sizeof(adv_data));

    adv_data.name_type               = BLE_ADVDATA_NO_NAME;
    adv_data.flags                   = flags;
    adv_data.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    adv_data.uuids_complete.p_uuids  = adv_uuids;
    adv_data.p_service_data_array    = &service_data;                // Pointer to Service Data structure.
    adv_data.service_data_count      = 1;

    memset(&scrsp_data, 0, sizeof(scrsp_data));
    scrsp_data.name_type            = BLE_ADVDATA_FULL_NAME;
    scrsp_data.include_appearance   = false;
    scrsp_data.uuids_complete.uuid_cnt = sizeof(scrp_uuids) / sizeof(scrp_uuids[0]);
    scrsp_data.uuids_complete.p_uuids = scrp_uuids;

    //err_code = ble_advdata_set(&adv_data, &scrsp_data);
    err_code = ble_advdata_set(&adv_data, NULL);
    APP_ERROR_CHECK(err_code);

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));
    
    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    m_adv_params.interval    = MSEC_TO_UNITS(APP_CFG_CONNECTABLE_ADV_INTERVAL_MS, UNIT_0_625_MS);
    m_adv_params.timeout     = APP_CFG_CONNECTABLE_ADV_TIMEOUT;
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_start(&m_adv_params);
    //err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "Error %d\n", err_code);
    }
    //APP_ERROR_CHECK(err_code);

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
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "Softdevice getcfg Error %d\n", err_code);
    }
    //APP_ERROR_CHECK(err_code);

    ble_enable_params.common_enable_params.vs_uuid_count = 2;
    SEGGER_RTT_printf(0, "UUID Count %d\n", ble_enable_params.common_enable_params.vs_uuid_count);
    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "Softlink Enable Error %d\n", err_code);
    }
    //APP_ERROR_CHECK(err_code);

    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "Softdevice bleevt handler set Error %d\n", err_code);
    }
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "Softdevice sysevt handler set Error %d\n", err_code);
    }
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

    SEGGER_RTT_WriteString(0, "Starting up BeaconBuddy\n");

    ble_stack_init();
    gap_params_init();
    conn_params_init();

    memset(&fat_init, 0, sizeof(fat_init));
    fat_init.read_evt_handler = fat_read_evt_handler;
    fat_init.val_data = m_page_data;

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

