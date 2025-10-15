
/*****************************************************************************************************************
 * INCLUDES
 */
#include "octopus_flash.h"
#include "octopus_key.h"
#include "octopus_gpio.h" // Include GPIO HAL for hardware-specific functionality

#include "octopus_uart_ptl.h"    // Include UART protocol header
#include "octopus_uart_upf.h"    // Include UART protocol header
#include "octopus_tickcounter.h" // Include tick counter for timing operations
#include "octopus_msgqueue.h"    // Include message queue header for task communication
#include "octopus_message.h"     // Include message id for inter-task communication
#ifdef TASK_MANAGER_STATE_MACHINE_CARINFOR
#include "octopus_vehicle.h"
#endif

/*****************************************************************************************************************
 * DEBUG SWITCH MACROS
 */

// #define ADDR_OTA_FLAG 0x1FFF18FC

#ifdef TASK_MANAGER_STATE_MACHINE_KEY
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*****************************************************************************************************************
 * GLOBAL VARIABLES
 */

// GPIO_STATUS gpio_zzd_pin_status = {(GPIO_GROUP *)GPIO_ZZD_KEY_GROUP, GPIO_ZZD_KEY_PIN, false, false, OCTOPUS_KEY_ZZD,0, 0};
// GPIO_STATUS gpio_yzd_pin_status = {(GPIO_GROUP *)GPIO_YZD_KEY_GROUP, GPIO_YZD_KEY_PIN, false, false, OCTOPUS_KEY_YZD,0, 0};
// GPIO_STATUS gpio_skd_pin_status = {(GPIO_GROUP *)GPIO_SKD_KEY_GROUP, GPIO_SKD_KEY_PIN, false, false, OCTOPUS_KEY_SKD,0, 0};
// GPIO_STATUS gpio_plus_pin_status = {(GPIO_GROUP *)GPIO_PLUS_KEY_GROUP, GPIO_PLUS_KEY_PIN, false, false, OCTOPUS_KEY_PLUS,0, 0};
// GPIO_STATUS gpio_subt_pin_status = {(GPIO_GROUP *)GPIO_SUBT_KEY_GROUP, GPIO_SUBT_KEY_PIN, false, false, OCTOPUS_KEY_SUBT,0, 0};

GPIO_KEY_STATUS key_status_power = {(GPIO_GROUP *)GPIO_POWER_KEY_GROUP, GPIO_POWER_KEY_PIN, OCTOPUS_KEY_POWER, 0, 0, 0, 0, 0, 0, 0};
// GPIO_KEY_STATUS key_status_zzd =   {(GPIO_GROUP *)GPIO_ZZD_KEY_GROUP,GPIO_ZZD_KEY_PIN,OCTOPUS_KEY_ZZD, 0, 0, 0, 0, 0, 0, 0};
// GPIO_KEY_STATUS key_status_yzd =   {(GPIO_GROUP *)GPIO_YZD_KEY_GROUP,GPIO_YZD_KEY_PIN,OCTOPUS_KEY_YZD, 0, 0, 0, 0, 0, 0, 0};
// GPIO_KEY_STATUS key_status_skd =   {(GPIO_GROUP *)GPIO_SKD_KEY_GROUP,GPIO_SKD_KEY_PIN,OCTOPUS_KEY_SKD, 0, 0, 0, 0, 0, 0, 0};
// GPIO_KEY_STATUS key_status_ddd =   {(GPIO_GROUP *)GPIO_DDD_KEY_GROUP,GPIO_DDD_KEY_PIN,OCTOPUS_KEY_DDD, 0, 0, 0, 0, 0, 0, 0};
// GPIO_KEY_STATUS key_status_plus = {(GPIO_GROUP *)GPIO_PLUS_KEY_GROUP, GPIO_PLUS_KEY_PIN, OCTOPUS_KEY_PLUS, 0, 0, 0, 0, 0, 0, 0};
// GPIO_KEY_STATUS key_status_subt = {(GPIO_GROUP *)GPIO_SUBT_KEY_GROUP, GPIO_SUBT_KEY_PIN, OCTOPUS_KEY_SUBT, 0, 0, 0, 0, 0, 0, 0};
GPIO_KEY_STATUS key_status_page = {(GPIO_GROUP *)GPIO_PAGE_KEY_GROUP, GPIO_PAGE_KEY_PIN, OCTOPUS_KEY_PAGE, 0, 0, 0, 0, 0, 0, 0};

GPIO_STATUS *gpio_array[] = {NULL};
GPIO_KEY_STATUS *gpio_key_array[] = {&key_status_power, &key_status_page, NULL};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t power_key_password[] = {OCTOPUS_KEY_POWER, OCTOPUS_KEY_POWER, OCTOPUS_KEY_POWER};
uint8_t power_key_password_index = 0;
GPIO_KEY_STATUS key_status_received_temp;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static uint32_t l_t_msg_wait_timer;
// static uint32_t l_t_msg_boot_wait_timer;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*****************************************************************************************************************
 * LOCAL FUNCTIONS DECLEAR
 */
static bool key_send_handler(ptl_frame_type_t frame_type, uint16_t param1, uint16_t param2, ptl_proc_buff_t *buff);
static bool key_receive_handler(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff);

static void task_key_action_handler(void);
static void task_key_event_dispatcher(GPIO_KEY_STATUS *key_status);
static void task_key_power_handler(GPIO_KEY_STATUS *key_status);
static void task_key_received_dispatcher(uint8_t key, uint8_t key_status);
static void task_key_local_dispatcher(uint8_t key, uint8_t key_status);

/*****************************************************************************************************************
 *  GLOBAL FUNCTIONS IMPLEMENTATION
 */
void task_key_init_running(void)
{
    LOG_LEVEL("task_key_init_running\r\n");
    ptl_register_module(MCU_TO_SOC_MOD_KEY, key_send_handler, key_receive_handler);
    OTMS(TASK_MODULE_KEY, OTMS_S_INVALID);
}

void task_key_start_running(void)
{
    LOG_LEVEL("task_key_start_running\r\n");
    OTMS(TASK_MODULE_KEY, OTMS_S_ASSERT_RUN);
}

void task_key_assert_running(void)
{
    ptl_reqest_running(MCU_TO_SOC_MOD_KEY);

    StartTickCounter(&l_t_msg_wait_timer);
    // StartTickCounter(&l_t_msg_boot_wait_timer);
    OTMS(TASK_MODULE_KEY, OTMS_S_RUNNING);
}

void task_key_running(void)
{
    task_key_action_handler();
}

void task_key_post_running(void)
{
    OTMS(TASK_MODULE_KEY, OTMS_S_ASSERT_RUN);
}

void task_key_stop_running(void)
{
    LOG_LEVEL("_stop_running\r\n");
    OTMS(TASK_MODULE_KEY, OTMS_S_INVALID);
}

static void task_key_action_handler(void)
{
    if (GetTickCounter(&l_t_msg_wait_timer) < 10)
        return;
    StartTickCounter(&l_t_msg_wait_timer);

    Msg_t *msg = get_message(TASK_MODULE_KEY);
    if (msg->msg_id == NO_MSG)
        return;

#ifdef TASK_MANAGER_STATE_MACHINE_GPIO
    uint16_t key = msg->param1;
    GPIO_KEY_STATUS *key_status = NULL;
    GPIO_STATUS *gpio_status = NULL;

    switch (msg->msg_id)
    {
    case MSG_OTSM_DEVICE_GPIO_EVENT:
        gpio_status = gpio_get_gpio_status_by_pin(msg->param1);
        if (gpio_status == NULL)
            break;

        if (!gpio_status->offon)
            send_message(TASK_MODULE_PTL_1, SOC_TO_MCU_MOD_KEY, gpio_status->key, KEY_STATE_PRESSED);
        else
            send_message(TASK_MODULE_PTL_1, SOC_TO_MCU_MOD_KEY, gpio_status->key, KEY_STATE_RELEASED);

        break;

    case MSG_OTSM_DEVICE_KEY_DOWN_EVENT:
        key_status = gpio_get_key_status_by_key(key);
        // LOG_LEVEL("key %d pressed key_status=%d\r\n",key,msg->param2);
        switch (key)
        {

        case OCTOPUS_KEY_POWER:
            task_key_power_handler(key_status);
            break;

        case OCTOPUS_KEY_PLUS:
        case OCTOPUS_KEY_SUBT:
        default:
            task_key_event_dispatcher(key_status);
            break;
        }

        break;

    case MSG_OTSM_DEVICE_KEY_UP_EVENT:
        key_status = gpio_get_key_status_by_key(key);
        // LOG_LEVEL("key %d release key_status=%d\r\n",key,msg->param2);
        switch (key)
        {

        case OCTOPUS_KEY_POWER:
            task_key_power_handler(key_status);
            break;

        case OCTOPUS_KEY_PLUS:
        case OCTOPUS_KEY_SUBT:
        default:
            task_key_event_dispatcher(key_status);
            break;
        }
        break;
    }
#endif
}

void task_key_power_handler(GPIO_KEY_STATUS *key_status)
{
#ifdef TASK_MANAGER_STATE_MACHINE_GPIO
    static uint32_t power_key_wait_timer;
    if (key_status == NULL)
        return;
    if (key_status->key != OCTOPUS_KEY_POWER)
        return;

    switch (key_status->state)
    {
    case KEY_STATE_RELEASED:
        LOG_LEVEL("OCTOPUS_KEY_POWER release key=%d key_status=%02x\r\n", key_status->key, key_status->state);
        key_status->state = KEY_STATE_NONE;
        StartTickCounter(&power_key_wait_timer);
        break;

    case KEY_STATE_PRESSED:
        LOG_LEVEL("OCTOPUS_KEY_POWER pressed key=%d key_status=%02x\r\n", key_status->key, key_status->state);
        hal_gpio_write((GPIO_GROUP *)GPIO_POWER_ENABLE_GROUP, GPIO_POWER_ENABLE_PIN, BIT_SET); // prepare to power
        if (GetTickCounter(&power_key_wait_timer) >= 300)
        {
            power_key_password_index = 0;
        }

        power_key_password_index++;

        if (power_key_password_index == sizeof(power_key_password))
        {
            send_message(TASK_MODULE_SYSTEM, MSG_OTSM_DEVICE_BLE_EVENT, MSG_OTSM_CMD_BLE_PAIR_ON, MSG_OTSM_CMD_BLE_PAIR_OFF);
            power_key_password_index = 0;
            StopTickCounter(&power_key_wait_timer);
        }
        break;

    case KEY_STATE_LONG_PRESSED:
        if (!key_status->ignore)
        {
        }
        break;

    case KEY_STATE_LONG_LONG_PRESSED:
        if (!key_status->ignore)
        {
        }
        break;

    case KEY_STATE_LONG_LONG_LONG_PRESSED:
        if (!key_status->ignore)
        {
            LOG_LEVEL("OCTOPUS_KEY_POWER pressed key=%d 3long duration=%d\r\n", key_status->key, key_status->state, key_status->press_duration);
            if (gpio_is_power_on())
            {
                send_message(TASK_MODULE_SYSTEM, MSG_OTSM_DEVICE_POWER_EVENT, FRAME_CMD_SYSTEM_POWER_OFF, 0);
            }
            else
            {
                send_message(TASK_MODULE_SYSTEM, MSG_OTSM_DEVICE_POWER_EVENT, FRAME_CMD_SYSTEM_POWER_ON, 0);
            }
            key_status->ignore = true;
        }
        break;
    case KEY_STATE_NONE:
    default:
        power_key_password_index = 0;
        break;
    }
#endif
}

void task_key_event_dispatcher(GPIO_KEY_STATUS *key_status)
{
    static uint32_t _key_wait_timer;
    static uint8_t _key_password_index;
    if (key_status == NULL)
        return;

    if (key_status->key == OCTOPUS_KEY_POWER)
        return;

    switch (key_status->state)
    {
    case KEY_STATE_RELEASED:
        LOG_LEVEL("key %02d release key_status=%02x\r\n", key_status->key, key_status->state);
        key_status->state = KEY_STATE_NONE;
        StartTickCounter(&_key_wait_timer);
        task_key_local_dispatcher(key_status->key, key_status->state);

        break;

    case KEY_STATE_PRESSED:
        LOG_LEVEL("key %02d pressed key_status=%02x\r\n", key_status->key, key_status->state);
        if (GetTickCounter(&_key_wait_timer) >= 300)
            _key_password_index = 0;
        _key_password_index++;

        task_key_local_dispatcher(key_status->key, key_status->state);
        break;

    case KEY_STATE_LONG_PRESSED:
        if (!key_status->ignore)
        {
            LOG_LEVEL("key %02d pressed key_status=%02x duration=%d\r\n", key_status->key, key_status->state, key_status->press_duration);
            key_status->ignore = true;
            send_message(TASK_MODULE_PTL_1, SOC_TO_MCU_MOD_KEY, key_status->key, key_status->state);
        }
        break;

    case KEY_STATE_LONG_LONG_PRESSED:
        if (!key_status->ignore)
        {
            LOG_LEVEL("key %02d pressed key_status=%02x duration=%d\r\n", key_status->key, key_status->state, key_status->press_duration);
            key_status->ignore = true;
        }
        break;

    case KEY_STATE_LONG_LONG_LONG_PRESSED:
        if (!key_status->ignore)
        {
            LOG_LEVEL("key %02d pressed key_status=%02x duration=%d\r\n", key_status->key, key_status->state, key_status->press_duration);
            key_status->ignore = true;
        }
        break;

    case KEY_STATE_NONE:
    default:
        power_key_password_index = 0;
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool key_send_handler(ptl_frame_type_t frame_type, uint16_t param1, uint16_t param2, ptl_proc_buff_t *buff)
{
    MY_ASSERT(buff);
    uint8_t tmp[8] = {0};

    if (MCU_TO_SOC_MOD_KEY == frame_type)
    {
        switch (param1)
        {
        case FRAME_CMD_SETUP_KEY: //
        default:
            tmp[0] = param1;
            tmp[1] = param2;
            LOG_LEVEL("MCU_TO_SOC_MOD_KEY param1=%02d param2=%02d\r\n", param1, param2);
            ptl_build_frame(MCU_TO_SOC_MOD_KEY, FRAME_CMD_SETUP_KEY, tmp, 2, buff);
            return true;
        }
    }

    if (SOC_TO_MCU_MOD_KEY == frame_type)
    {
        switch (param1)
        {
        case FRAME_CMD_SETUP_KEY:
        default:
            tmp[0] = param1;
            tmp[1] = param2;
            LOG_LEVEL("SOC_TO_MCU_MOD_KEY param1=%02d param2=%02d\r\n", param1, param2);
            ptl_build_frame(SOC_TO_MCU_MOD_KEY, FRAME_CMD_SETUP_KEY, tmp, 2, buff);
            return true;
        }
    }
    return false;
}

bool key_receive_handler(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff)
{
    MY_ASSERT(payload);
    MY_ASSERT(ackbuff);
    uint8_t tmp;

    if (MCU_TO_SOC_MOD_KEY == payload->frame_type)
    {
        switch (payload->frame_cmd)
        {
        case FRAME_CMD_SETUP_UPDATE_TIME:
            tmp = 0x01;
            ptl_build_frame(SOC_TO_MCU_MOD_SETUP, FRAME_CMD_SETUP_UPDATE_TIME, &tmp, 1, ackbuff);
            return true;
        case FRAME_CMD_SETUP_SET_TIME:
            return false;
        case FRAME_CMD_SETUP_KEY:
        default:
            send_message(TASK_MODULE_IPC, MSG_OTSM_DEVICE_KEY_DOWN_EVENT, payload->data[0], payload->data[1]);
            break;
        }
    }

    if (SOC_TO_MCU_MOD_KEY == payload->frame_type)
    {
        if (payload->data[0] == KEY_STATE_RELEASED)
            key_status_received_temp.ignore = false;

        if (key_status_received_temp.key != payload->data[1])
        {
            key_status_received_temp.key = payload->data[0];
            key_status_received_temp.state = payload->data[1];
            if (key_status_received_temp.state == KEY_STATE_PRESSED)
            {
                key_status_received_temp.ignore = false;
            }
        }

        if (payload->data_len >= 2)
        {
            task_key_received_dispatcher(payload->data[0], payload->data[1]);
        }
        else
        {
            task_key_received_dispatcher(payload->frame_cmd, payload->data[0]);
        }
    }
    return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void task_key_local_dispatcher(uint8_t key, uint8_t key_status)
{
    LOG_LEVEL("key %02d key_status=%02d\r\n", key, key_status);
    switch (key)
    {
    case OCTOPUS_KEY_PAGE:
        send_message(TASK_MODULE_PTL_1, MCU_TO_SOC_MOD_KEY, key, key_status);
        break;
    }
}

void task_key_received_dispatcher(uint8_t key, uint8_t key_status)
{
    LOG_LEVEL("key %02d state %02d\r\n", key, key_status);
    switch (key)
    {
    case OCTOPUS_KEY_ZZD:
        if (key_status == KEY_STATE_PRESSED)
        {
            lt_carinfo_indicator.left_turn = !lt_carinfo_indicator.left_turn;
            lt_carinfo_indicator.right_turn = 0;
        }
        break;

    case OCTOPUS_KEY_YZD:
        if (key_status == KEY_STATE_PRESSED)
        {
            lt_carinfo_indicator.right_turn = !lt_carinfo_indicator.right_turn;
            lt_carinfo_indicator.left_turn = 0;
        }
        break;

    case OCTOPUS_KEY_SKD:
        // lt_carinfo_indicator.width_lamp = !lt_carinfo_indicator.width_lamp;
        if (key_status == KEY_STATE_PRESSED)
            lt_carinfo_indicator.horn = 1;
        else
            lt_carinfo_indicator.horn = 0;

        break;

    case OCTOPUS_KEY_DDD:
        lt_carinfo_indicator.high_beam = !lt_carinfo_indicator.high_beam;
        break;

    case OCTOPUS_KEY_PLUS:
        if (key_status == KEY_STATE_LONG_PRESSED)
        {
            lt_carinfo_indicator.high_beam = !lt_carinfo_indicator.high_beam;
            key_status_received_temp.ignore = true;
            send_message(TASK_MODULE_CAR_INFOR, MCU_TO_SOC_MOD_CARINFOR, FRAME_CMD_CARINFOR_INDICATOR, FRAME_CMD_CARINFOR_INDICATOR);
            break;
        }
        else if (key_status_received_temp.ignore)
        {
            break;
        }

    case OCTOPUS_KEY_SUBT:
        if (key_status == KEY_STATE_LONG_PRESSED)
        {
            lt_carinfo_indicator.walk_assist = !lt_carinfo_indicator.walk_assist;
            key_status_received_temp.ignore = true;
            send_message(TASK_MODULE_CAR_INFOR, MCU_TO_SOC_MOD_CARINFOR, FRAME_CMD_CARINFOR_INDICATOR, FRAME_CMD_CARINFOR_INDICATOR);
            break;
        }
        else if (key_status_received_temp.ignore)
        {
            break;
        }

    case OCTOPUS_KEY_PAGE:
        send_message(TASK_MODULE_PTL_1, MCU_TO_SOC_MOD_KEY, key, key_status);
        break;
    }
}
#endif
