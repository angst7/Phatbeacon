#include "ble_fat.h"
#include <string.h>
#include "endian_convert.h"

#define BLE_UUID_FAT_URL_SERVICE    0x46D4
#define BLE_UUID_FAT_URL_CHAR       0x17F0

#define FAT_SERVICE_BASE_UUID \
{{0xD3, 0xFF, 0x6A, 0xCA, 0x7C, 0xA9, 0xA5, 0xB6, 0xA8, 0x4B, 0x87, 0xE5, 0x00, 0x00, 0x59, 0xAE}}
#define FAT_CHARACTERISTIC_BASE_UUID \
{{0xFA, 0x66, 0xC9, 0xC1, 0x9B, 0x80, 0xCC, 0x9C, 0xCA, 0x46, 0x99, 0x24, 0x00, 0x00, 0xA5, 0xD1}}

#define STATIC_BORING_WEBPAGE '<','h','t','m','l','>','<','h','e','a','d','>', \
                              '<','t','i','t','l','e','>','F','a','t','b','e', \
                              'a','c','o','n','<','/','t','i','t','l','e','>', \
                              '<','/','h','e','a','d','>','<','b','o','d','y', \
                              '>','<','h','1','>','I','t',' ','w','o','r','k', \
                              'e','d','!','<','/','h','1','>','<','/','b','o', \
                              'd','y','>','</','h','t','m','l','>'

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
static void on_disconnect(ble_fat_t * p_fat, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_fat->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST: BLE_GATTS_AUTHORIZE_TYPE_WRITE event from the S132 SoftDevice.
 *
 * @param[in] p_fat     Fatbeacon URL Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_fat_t * p_fat, ble_evt_t * p_ble_evt)
{
    //ble_gatts_evt_rw_authorize_request_t * p_evt_rw_auth = &p_ble_evt->evt.gatts_evt.params.authorize_request;
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.authorize_request.request.write;


    if (p_fat->write_evt_handler != NULL)
    {
        p_fat->write_evt_handler(p_fat, p_evt_write->handle, p_evt_write->data, p_evt_write->len);
    }
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
    uint32_t err_code;

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
            on_disconnect(p_fat, p_ble_evt);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            if (p_ble_evt->evt.gatts_evt.params.authorize_request.type == BLE_GATTS_AUTHORIZE_TYPE_READ)
            {
                on_read(p_fat, p_ble_evt);
            }
            else if (p_ble_evt->evt.gatts_evt.params.authorize_request.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
            {
                if (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)
                {
                    DEBUG_PRINTF(0,"PREP_WRITE_REQUEST \r\n",0);
                }
                else if (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
                {
                    DEBUG_PRINTF(0,"EXEC_WRITE_REQUEST \r\n",0);
                }
                on_write(p_fat, p_ble_evt);
            }
            else
            {
                //BLE_GATTS_AUTHORIZE_TYPE_INVALID TODO: Report Error?
            }
            break;
        //BLE_EVT_USER_MEM_REQUEST & BLE_EVT_USER_MEM_RELEASE are for long writes to the RW ADV slot characteristic
        case BLE_EVT_USER_MEM_REQUEST:
            DEBUG_PRINTF(0,"USER_MEM_REQUEST: error: %d \r\n", err_code);
            break;

        case BLE_EVT_USER_MEM_RELEASE:
            DEBUG_PRINTF(0,"USER_MEM_RELEASE\r\n", 0);
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
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

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
    attr_md.rd_auth = 1;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    //Eddystone spec requires big endian
    ble_fat_brdcst_cap_t temp = p_fat_init->p_init_vals->brdcst_cap;
    temp.supp_frame_types = BYTES_SWAP_16BIT(temp.supp_frame_types);

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(STATIC_BORING_WEBPAGE);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = (uint8_t *)(&temp);
    attr_char_value.max_len   = sizeof(STATIC_BORING_WEBPAGE);

    return sd_ble_gatts_characteristic_add(p_fat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_fat->brdcst_cap_handles);
}

uint32_t ble_fat_init(ble_fat_t * p_fat, const ble_fat_init_t * p_fat_init)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t fat_base_uuid = FAT_SERVICE_BASE_UUID;
    ble_uuid_t    ble_char_uuid;
    ble_uuid128_t fat_char_base_uuid = FAT_CHARACTERISTIC_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_fat);
    VERIFY_PARAM_NOT_NULL(p_fat_init);

    // Initialize the service structure.
    p_fat->conn_handle                        = BLE_CONN_HANDLE_INVALID;
    p_fat->write_evt_handler                  = p_fat_init->write_evt_handler;
    p_fat->read_evt_handler                   = p_fat_init->read_evt_handler;

    // Add a custom base service UUID.
    err_code = sd_ble_uuid_vs_add(&fat_base_uuid, &p_fat->uuid_type);
    VERIFY_SUCCESS(err_code);

    // Add a custom base characteristic UUID.
    err_code = sd_ble_uuid_vs_add(&fat_char_base_uuid, &p_fat->char_uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_fat->uuid_type;
    ble_uuid.uuid = BLE_UUID_FAT_URL_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_fat->service_handle);
    VERIFY_SUCCESS(err_code);

    /*Adding chracteristics*/
    err_code = fat_url_char_add(p_fat, p_fat_init);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}
