
#include "fat_ble_handler.h"

/**@brief Initialize the FAT with initial values for the characteristics and other necessary modules */
static void services_and_modules_init(void)
{
    ret_code_t err_code;
    ble_fat_init_t fat_init;
    ble_fat_init_params_t init_params;
    int8_t tx_powers[FAT_NUM_OF_SUPORTED_TX_POWER] = FAT_SUPPORTED_TX_POWER;

    /*Init the broadcast capabilities characteristic*/
    memset(&init_params.brdcst_cap, 0, sizeof(init_params.brdcst_cap));
    init_params.brdcst_cap.vers_byte            = EDDYSTONE_SPEC_VERSION_BYTE;
    init_params.brdcst_cap.max_supp_total_slots = APP_MAX_ADV_SLOTS;
    init_params.brdcst_cap.max_supp_eid_slots   = APP_MAX_EID_SLOTS;
    init_params.brdcst_cap.cap_bitfield         = ( (APP_IS_VARIABLE_ADV_SUPPORTED << FAT_BRDCST_VAR_ADV_SUPPORTED_Pos)
                                                  | (APP_IS_VARIABLE_TX_POWER_SUPPORTED << FAT_BRDCST_VAR_TX_POWER_SUPPORTED_Pos))
                                                  & (FAT_BRDCST_VAR_RFU_MASK);
    init_params.brdcst_cap.supp_frame_types     = ( (APP_IS_URL_SUPPORTED << FAT_FRAME_TYPE_URL_SUPPORTED_Pos)
                                                  | (APP_IS_UID_SUPPORTED << FAT_FRAME_TYPE_UID_SUPPORTED_Pos)
                                                  | (APP_IS_TLM_SUPPORTED << FAT_FRAME_TYPE_TLM_SUPPORTED_Pos)
                                                  | (APP_IS_EID_SUPPORTED << FAT_FRAME_TYPE_EID_SUPPORTED_Pos))
                                                  & (FAT_FRAME_TYPE_RFU_MASK);
    memcpy(init_params.brdcst_cap.supp_radio_tx_power, tx_powers, FAT_NUM_OF_SUPORTED_TX_POWER);

    /*Init the active slots characteristic*/
    init_params.active_slot = 0;

    /*Init the advertising intervals characteristic*/
    init_params.adv_intrvl = APP_CFG_NON_CONN_ADV_INTERVAL_MS;

    /*Init the radio tx power characteristic*/
    init_params.adv_tx_pwr = APP_CFG_DEFAULT_RADIO_TX_POWER;

    /*Init the radio tx power characteristic (Currently not implemented)*/
    init_params.radio_tx_pwr = 0x00;

    /*Init the lock state characteristic*/
    init_params.lock_state.read = BLE_FAT_LOCK_STATE_LOCKED;

    uint8_t eddystone_default_data[] = DEFAULT_FRAME_DATA;
    init_params.rw_adv_slot.frame_type = (eddystone_frame_type_t)(DEFAULT_FRAME_TYPE);
    init_params.rw_adv_slot.p_data = (int8_t *)(eddystone_default_data);
    init_params.rw_adv_slot.char_length = sizeof(eddystone_default_data) + 1; // plus the frame_type
    //To set a TLM slot as default slot in firmware, just set the frame_type as TLM, and char_length as 1.
    //And p_data as NULL

    init_params.factory_reset = 0;
    init_params.remain_cnntbl.r_is_non_connectable_supported = 1;

    //Initialize evt handlers and the service
    memset(&fat_init, NULL, sizeof(fat_init));
    fat_init.write_evt_handler = fat_write_evt_handler;
    fat_init.read_evt_handler = fat_read_evt_handler;
    fat_init.p_init_vals = &(init_params);

    err_code = ble_fat_init(&m_ble_fat, &fat_init);
    APP_ERROR_CHECK(err_code);

    //Initialize the security module
    eddystone_security_init_t security_init =
    {
        .msg_cb = ble_eddystone_security_cb,
    };

    err_code = eddystone_flash_init(eddystone_pstorage_cb_handler);
    APP_ERROR_CHECK(err_code);

    err_code = eddystone_security_init(&security_init);
    APP_ERROR_CHECK(err_code);

    //Initialize the slots with the initial values of the characteristics
    eddystone_adv_slots_init(&fat_init);

    eddystone_advertising_manager_init(m_ble_fat.uuid_type);
}

void fat_ble_init()
{
    ble_stack_init();
    gap_params_init();
    conn_params_init();

    services_and_modules_init();
}