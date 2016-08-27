#ifndef BLE_FAT_H__
#define BLE_FAT_H__

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>

#define FAT_SERVICE_BASE_UUID {{0xD3, 0xFF, 0x6A, 0xCA, 0x7C, 0xA9, 0xA5, 0xB6, 0xA8, 0x4B, 0x87, 0xE5, 0x00, 0x00, 0x59, 0xAE}}
#define FAT_CHARACTERISTIC_BASE_UUID {{0xFA, 0x66, 0xC9, 0xC1, 0x9B, 0x80, 0xCC, 0x9C, 0xCA, 0x46, 0x99, 0x24, 0x00, 0x00, 0xA5, 0xD1}}
#define BLE_UUID_FAT_URL_SERVICE    0x46D4
#define BLE_UUID_FAT_URL_CHAR       0x17F0

#define FAT_CHAR_MAX_LEN            (20)

/*Forward Declaration of of ble_fat_t type*/
typedef struct ble_fat_s ble_fat_t;

typedef void (*ble_fat_read_evt_handler_t) ( ble_fat_t *                p_fat,
                                             uint16_t                   value_handle
                                             );

/**@brief struct for data fields in Broadcast Capabilities characteristic*/
/*
typedef PACKED(struct)
{
    int8_t     vers_byte;
    int8_t     max_supp_total_slots;
    int8_t     max_supp_eid_slots;
    int8_t     cap_bitfield;
    int16_t    supp_frame_types;
    int8_t     supp_radio_tx_power[ECS_NUM_OF_SUPORTED_TX_POWER];
} ble_fat_brdcst_cap_t;
*/

/**@brief Fatbeacon URL Service initialization structure.
*
* @details This structure contains the initialization information for the service. The application
* must fill this structure and pass it to the service using the @ref ble_ecs_init
* function.
*/
typedef struct
{
    ble_fat_read_evt_handler_t      read_evt_handler;   /**< Event handler to be called for authorizing read requests. */
    uint8_t*                        val_data;
} ble_fat_init_t;

struct ble_fat_s
{
    uint8_t                         uuid_type;                    /**< UUID type for Fatbeacon URL Service Base UUID. */
    uint8_t                         char_uuid_type;               /**< UUID type for Fatbeacon URL Characteristic Base UUID. */
    uint16_t                        service_handle;               /**< Handle of fatbeacon url Service  */
    ble_gatts_char_handles_t        fat_url_handles;              /**< Handles related to the fatbeacon_url characteristic */
    uint16_t                        conn_handle;                  /**< Handle of the current connection (as provided by the S132 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */    
    ble_fat_read_evt_handler_t      read_evt_handler;             /**< Event handler to be called for handling read attempts. */
    uint8_t*                        val_data;
};

uint32_t ble_fat_init(ble_fat_t * p_fat, const ble_fat_init_t * p_fat_init);
void ble_fat_on_ble_evt(ble_fat_t * p_fat, ble_evt_t * p_ble_evt);

#endif
