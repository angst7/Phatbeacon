/*****************************************************************************
*
* ble_fat.c
*
* This contains the initialization for the Eddystone Fatbeacon service 
* and characteristic, and its handler functions.
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
********************************************************************************/

#include "ble_fat.h"
#include <string.h>
#include "SEGGER_RTT.h"

static uint8_t m_page_content[10000] = {0};

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the S132 SoftDevice.
 *
 * @param[in] p_fat     Fatbeacon URL Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_fat_t * p_fat, ble_evt_t * p_ble_evt)
{
    p_fat->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S132 SoftDevice.
 *
 * @param[in] p_fat     Fatbeacon URL Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_fat_t * p_fat) //, ble_evt_t * p_ble_evt)
{
    //UNUSED_PARAMETER(p_ble_evt);
    p_fat->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST: BLE_GATTS_AUTHORIZE_TYPE_READ event from the S132 SoftDevice.
 *
 * @param[in] p_fat     Fatbeacon URL Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_read(ble_fat_t * p_fat, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_read_t * p_evt_read = &p_ble_evt->evt.gatts_evt.params.authorize_request.request.read;

    p_fat->read_evt_handler(p_fat, p_evt_read->handle);
   
}

void ble_fat_on_ble_evt(ble_fat_t * p_fat, ble_evt_t * p_ble_evt)
{

    if ((p_fat == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_fat, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_fat); //, p_ble_evt);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            if (p_ble_evt->evt.gatts_evt.params.authorize_request.type == BLE_GATTS_AUTHORIZE_TYPE_READ)
            {
                on_read(p_fat, p_ble_evt);
            }            
            else
            {
                //BLE_GATTS_AUTHORIZE_TYPE_INVALID TODO: Report Error?
            }
            break;
        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for adding braodcast capability characteristic.
 *
 * @param[in] p_fat       Fatbeacon URL Service structure.
 * @param[in] p_fat_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
 
static uint32_t fat_url_char_add(ble_fat_t * p_fat, const ble_fat_init_t * p_fat_init)
{
    uint32_t            err_code = NRF_SUCCESS;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    uint32_t            page_size = strlen((char *)p_fat_init->val_data);
    uint8_t             chars_added = 0;
    static uint8_t      total_chars = 0;
    uint16_t            char_len = 0;

    total_chars = (page_size / FAT_MAX_CHAR_SIZE) + 
                                      (page_size % FAT_MAX_CHAR_SIZE > 0 ? 1 : 0);

    memcpy(m_page_content, p_fat_init->val_data, page_size);

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL; 
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_fat->char_uuid_type;
    ble_uuid.uuid = BLE_UUID_FAT_URL_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;        // No read authorization required
    attr_md.wr_auth = 0;
    attr_md.vlen    = 0;        // fixed length

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(total_chars);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = &total_chars;    // Number of subsequent characteristics to read
    attr_char_value.max_len   = sizeof(total_chars);

    err_code = sd_ble_gatts_characteristic_add(p_fat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_fat->fat_url_handles);

    while ((err_code == NRF_SUCCESS) && (chars_added < total_chars)) 
    {
        
        chars_added++;

        // If this is the last characteristic, length is the remainder, otherwise its full size
        if (chars_added < total_chars) {  
            char_len = FAT_MAX_CHAR_SIZE;
        } else {
            char_len = page_size % FAT_MAX_CHAR_SIZE;
        }

        ble_uuid.uuid = BLE_UUID_FAT_URL_CHAR+chars_added;

        attr_md.vloc    = BLE_GATTS_VLOC_USER;

        attr_char_value.p_uuid    = &ble_uuid;
        attr_char_value.p_attr_md = &attr_md;
        attr_char_value.init_len  = char_len;
        attr_char_value.init_offs = 0;        
        attr_char_value.p_value   = m_page_content + ((chars_added-1) * FAT_MAX_CHAR_SIZE);
        attr_char_value.max_len   = char_len;

        err_code = sd_ble_gatts_characteristic_add(p_fat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_fat->fat_url_part_handles[chars_added-1]);
        if (err_code != NRF_SUCCESS) {
            SEGGER_RTT_printf(0, "Failed adding characteristic #%d: %d\n", chars_added-1, err_code);
        }

    }

    return err_code;
}


uint32_t ble_fat_init(ble_fat_t * p_fat, const ble_fat_init_t * p_fat_init)
{
    uint32_t      err_code = 0;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t fat_base_uuid = FAT_SERVICE_BASE_UUID;
    ble_uuid128_t fat_char_base_uuid = FAT_CHARACTERISTIC_BASE_UUID;

    memset(p_fat, 0, sizeof(p_fat));

    // Initialize the service structure.
    p_fat->conn_handle                        = BLE_CONN_HANDLE_INVALID;
    p_fat->read_evt_handler                   = p_fat_init->read_evt_handler;
    p_fat->val_data                           = p_fat_init->val_data;

    // Add a custom base service UUID.
    err_code = sd_ble_uuid_vs_add(&fat_base_uuid, &p_fat->uuid_type);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "UUID vs Add S Error %d\n", err_code);
    }

    ble_uuid.type = p_fat->uuid_type;
    ble_uuid.uuid = BLE_UUID_FAT_URL_SERVICE;


    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_fat->service_handle);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "GATTS Svc add Error %d\n", err_code);
    }

    // Add a custom base characteristic UUID.
    err_code = sd_ble_uuid_vs_add(&fat_char_base_uuid, &p_fat->char_uuid_type);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "UUID vs Add C Error %d\n", err_code);
    }

    err_code = fat_url_char_add(p_fat, p_fat_init);
    if (err_code != NRF_SUCCESS) {
        SEGGER_RTT_printf(0, "Fatbeacon char add Error %d\n", err_code);
    }

    return NRF_SUCCESS;
}
