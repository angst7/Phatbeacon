#ifndef BLE_FAT_H__
#define BLE_FAT_H__


#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>

/*Forward Declaration of of ble_fat_t type*/
typedef struct ble_fat_s ble_fat_t;

typedef void (*ble_fat_write_evt_handler_t) (ble_fat_t *        p_fat,
                                             ble_fat_evt_type_t evt_type,
                                             uint16_t           value_handle,
                                             uint8_t *          p_data,
                                             uint16_t           length);

typedef void (*ble_fat_read_evt_handler_t) ( ble_fat_t *                p_fat,
                                             ble_fat_evt_type_t         evt_type,
                                             uint16_t                   value_handle
                                             );

/**@brief Fatbeacon URL Service initialization structure.
*
* @details This structure contains the initialization information for the service. The application
* must fill this structure and pass it to the service using the @ref ble_ecs_init
* function.
*/
typedef struct
{
    ble_fat_init_params_t         * p_init_vals;
    ble_fat_write_evt_handler_t     write_evt_handler;  /**< Event handler to be called for authorizing write requests. */
    ble_fat_read_evt_handler_t      read_evt_handler;   /**< Event handler to be called for authorizing read requests. */
} ble_fat_init_t;

struct ble_fat_s
{
    uint8_t                         uuid_type;                    /**< UUID type for Fatbeacon URL Service Base UUID. */
    uint8_t                         char_uuid_type;               /**< UUID type for Fatbeacon URL Characteristic Base UUID. */
    uint16_t                        service_handle;               /**< Handle of fatbeacon url Service  */
    ble_gatts_char_handles_t        fat_url_handles;              /**< Handles related to the fatbeacon_url characteristic */
    uint16_t                        conn_handle;                  /**< Handle of the current connection (as provided by the S132 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    ble_fat_write_evt_handler_t     write_evt_handler;            /**< Event handler to be called for handling write attempts. */
    ble_fat_read_evt_handler_t      read_evt_handler;             /**< Event handler to be called for handling read attempts. */
};

#endif
