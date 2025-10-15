
/*******************************************************************************
 * INCLUDES
 */
#include "octopus_ling_hui_liion2.h"
#include "octopus_task_manager.h"   // Task Manager: handles scheduling and execution of system tasks
#include "octopus_tickcounter.h"    // Tick Counter: provides timing and delay utilities
#include "octopus_message.h"        // Message IDs: defines identifiers for inter-task communication
#include "octopus_msgqueue.h"       // Message Queue: API for sending/receiving messages between tasks
#include "octopus_uart_ptl.h"       // UART Protocol Layer: handles protocol-level UART operations
#include "octopus_uart_upf.h"       // UART Packet Framework: low-level UART packet processing

#include "octopus_vehicle.h"

/*******************************************************************************
 * DEBUG SWITCH MACROS
 */
#ifdef TASK_MANAGER_STATE_MACHINE_LING_HUI_LIION2
/*******************************************************************************
 * MACROS
 */
#define PTL_LHLL_I2C_HEADER (0x01) //(ICU -> CTRL)FRAME HEADER
#define PTL_LHLL_C2I_HEADER (0x02) //(CTRL -> ICU)FRAME HEADER
#define PTL_LHLL_FRAME_HEADER_SIZE (4)
#define PTL_LHLL_FRAME_MIN_SIZE (4)
/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * LOCAL FUNCTIONS DECLEAR
 */
uint8_t lhl2_ptl_checksum(uint8_t *data, uint8_t len);
bool lhl2_ptl_receive_handler(upf_proc_buff_t *upf_proc_buff);
void lhl2_ptl_tx_process(void);
void lhl2_ptl_proc_valid_frame(uint8_t *data, uint16_t length);

uint16_t get_wheel_radius_inch(void);
uint16_t get_geer_level(void);

void lhl2_ptl_test_carinfo_indicator(void);
/*******************************************************************************
 * GLOBAL VARIABLES
 */
upf_module_t upf_module_info_LING_HUI_LIION2 = {UPF_MODULE_ID_LING_HUI_LIION2, UPF_CHANNEL_8, UPF_CHANNEL_TYPE_BYTE};
/*******************************************************************************
 * STATIC VARIABLES
 */
static uint32_t lhl2_task_interval_ms = 0;
static uint32_t lhl2_task_tx_interval_ms = 0;

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 *  GLOBAL FUNCTIONS IMPLEMENTATION
 */

void task_lhl2_ptl_init_running(void)
{
    OTMS(TASK_MODULE_LING_HUI_LIION2, OTMS_S_INVALID);
    LOG_LEVEL("task_bafang_ptl_init_running\r\n");
}

void task_lhl2_ptl_start_running(void)
{
    LOG_LEVEL("task_bafang_ptl_start_running\r\n");
    upf_register_module(upf_module_info_LING_HUI_LIION2, lhl2_ptl_receive_handler);
    OTMS(TASK_MODULE_LING_HUI_LIION2, OTMS_S_ASSERT_RUN);
}

void task_lhl2_ptl_assert_running(void)
{
    OTMS(TASK_MODULE_LING_HUI_LIION2, OTMS_S_RUNNING);
    StartTickCounter(&lhl2_task_interval_ms);
    StartTickCounter(&lhl2_task_tx_interval_ms);

    lt_carinfo_meter.wheel_diameter = SETTING_WHEEL_20_Inch;
    lt_carinfo_meter.gear_level_max = SETTING_MAX_PAS_5_LEVEL;
    lt_carinfo_meter.speed_limit = 255;
    lt_carinfo_meter.gear = 0;

    lt_carinfo_battery.voltage = 600;
    lt_carinfo_battery.current = 175;
    // lt_carinfo_battery.range_max = 60; // UINT16_MAX;
    // lt_carinfo_battery.range = 60;

    lt_carinfo_indicator.cruise_control = true;
    lt_carinfo_indicator.start_poles = 1;
    lt_carinfo_indicator.motor_poles = 2;
    lt_carinfo_indicator.cruise_control = true;
    // lt_carinfo_indicator.horn = true;
}

void task_lhl2_ptl_running(void)
{
    static uint8_t loop = 0;
    lhl2_ptl_tx_process();

    if (GetTickCounter(&lhl2_task_interval_ms) < 30)
    {
        return;
    }
    StartTickCounter(&lhl2_task_interval_ms);

    if (loop == 0)
    {
        send_message(TASK_MODULE_CAR_INFOR, MCU_TO_SOC_MOD_CARINFOR, FRAME_CMD_CARINFOR_INDICATOR, FRAME_CMD_CARINFOR_INDICATOR);
        loop++;
    }
    else if (loop == 1)
    {
        send_message(TASK_MODULE_CAR_INFOR, MCU_TO_SOC_MOD_CARINFOR, FRAME_CMD_CARINFOR_METER, FRAME_CMD_CARINFOR_METER);
        loop++;
    }
    else if (loop == 2)
    {
        send_message(TASK_MODULE_CAR_INFOR, MCU_TO_SOC_MOD_CARINFOR, FRAME_CMD_CARINFOR_BATTERY, FRAME_CMD_CARINFOR_BATTERY);
        loop++;
    }
    else if (loop == 3)
    {
        send_message(TASK_MODULE_CAR_INFOR, MCU_TO_SOC_MOD_CARINFOR, FRAME_CMD_CARINFOR_ERROR, FRAME_CMD_CARINFOR_ERROR);
        loop = 0;
    }
    else
    {
        loop = 0;
    }
}

void task_lhl2_ptl_post_running(void)
{
    OTMS(TASK_MODULE_LING_HUI_LIION2, OTMS_S_ASSERT_RUN);
}

void task_lhl2_ptl_stop_running(void)
{
    LOG_LEVEL("_stop_running\r\n");
    OTMS(TASK_MODULE_LING_HUI_LIION2, OTMS_S_INVALID);
}

/*******************************************************************************
 * LOCAL FUNCTIONS IMPLEMENTATION
 */
uint16_t car_infor_to_lhl2_pwm(uint8_t ui_value)
{
    return (uint16_t)(((uint32_t)ui_value * 1000 + 127) / 255);
}

uint8_t lhl2_pwm_to_car_infor(uint16_t pwm_value)
{
    if (pwm_value > 1000)
        pwm_value = 1000;
    return (uint8_t)(((uint32_t)pwm_value * 255 + 500) / 1000);
}

void lhl2_ptl_test_carinfo_indicator(void)
{
    static uint8_t i = 0;
    memset(&lt_carinfo_indicator, 0, sizeof(lt_carinfo_indicator));

    uint8_t *ptr = (uint8_t *)&lt_carinfo_indicator;
    size_t total_fields = sizeof(carinfo_indicator_t) / sizeof(uint8_t);

    // for (size_t i = 0; i < total_fields; i++)
    {
        // memset(&lt_carinfo_indicator, 0, sizeof(lt_carinfo_indicator));  // 清零
        ptr[i] = 1; // 当前字段置1
        i++;
        if (i >= total_fields)
            i = 0;
    }
}

void lhl2_ptl_tx_process(void)
{
    uint8_t lu_tx_buff[32];

    if (GetTickCounter(&lhl2_task_tx_interval_ms) < 200)
        return;
    StartTickCounter(&lhl2_task_tx_interval_ms);
    // lhl2_ptl_test_carinfo_indicator();

    lu_tx_buff[0] = PTL_LHLL_I2C_HEADER;
    lu_tx_buff[1] = 20;
    lu_tx_buff[2] = 0x01;
    // 驱动方式
    lu_tx_buff[3] = lt_carinfo_indicator.drive_mode;
    // 助力档位
    lu_tx_buff[4] = get_geer_level();

    // 控制器控制设定1
    uint8_t flag = 0x00;
    if (get_geer_level() > 0)
    {
        flag |= BIT_7;
    }

    if (lt_carinfo_indicator.start_mode)
    {
        flag |= BIT_6;
    }
    if (lt_carinfo_indicator.high_beam)
    {
        flag |= BIT_5;
    } //--发送大灯

    if (task_carinfo_has_error_code())
    {
        flag |= BIT_4;
    } //--发送通讯故障

    if (lt_carinfo_indicator.horn)
    {
        flag |= BIT_3;
    } //--长按鸣笛键，鸣笛，，瑞碌

    // if (theIndicatorFlag.speed_limit) { flag |= BIT_02; }
    if (lt_carinfo_indicator.right_turn)
    {
        flag |= BIT_2;
    } //--改右转向，瑞碌

    // if (theMeterInfo.walk_assist) {flag |= BIT_01; }
    if (lt_carinfo_indicator.left_turn)
    {
        flag |= BIT_1;
    } //--改左转向，瑞碌

    if (lt_carinfo_indicator.cruise_control)
    {
        flag |= BIT_0;
    } // 定速巡航模式 true:启动定速巡航

    lu_tx_buff[5] = flag;

    // 测速磁钢数：（单位：磁钢片数）
    lu_tx_buff[6] = lt_carinfo_indicator.motor_poles;

    // 轮径:（单位:0.1英寸）
    uint16_t radius = get_wheel_radius_inch();
    lu_tx_buff[7] = MK_MSB(radius);
    lu_tx_buff[8] = MK_LSB(radius);

    // 助力灵敏度
    lu_tx_buff[9] = lt_carinfo_indicator.start_poles; // 1

    // 助力启动强度
    lu_tx_buff[10] = lt_carinfo_indicator.start_mode;

    // 内测速磁钢数
    lu_tx_buff[11] = 0;

    // 限速值:(单位：km/h)
    lu_tx_buff[12] = lt_carinfo_meter.speed_limit;

    // 控制器限流值
    lu_tx_buff[13] = lt_carinfo_battery.current_limit;

    // 控制器欠压值:(单位：0.1V)
    uint16_t vol = lt_carinfo_battery.voltage / 10;

    lu_tx_buff[14] = MK_MSB(vol);
    lu_tx_buff[15] = MK_LSB(vol);

    // 转把调速PWM
    lu_tx_buff[16] = MK_MSB(lt_carinfo_battery.throttle_pwm);
    lu_tx_buff[17] = MK_LSB(lt_carinfo_battery.throttle_pwm);

    flag = 0x00;
    // 控制器控制设定 2 +助力磁钢盘磁钢个数
    if (lt_carinfo_indicator.cruise_control)
    {
        flag |= BIT_6;
    } // 巡航状态

    if (lt_carinfo_indicator.drive_mode)
    {
        flag |= BIT_4;
    } // 0:后驱；1：双驱

    lu_tx_buff[18] = flag | 0x06; // 助力磁钢数：6

    lu_tx_buff[19] = lhl2_ptl_checksum(lu_tx_buff, 19);

    upf_send_buffer(upf_module_info_LING_HUI_LIION2, lu_tx_buff, 20);
}

void lhl2_ptl_remove_none_header_data(upf_proc_buff_t *upf_proc_buff)
{
    if (upf_proc_buff->buffer[0] == PTL_LHLL_C2I_HEADER)
    {
        return;
    }

    for (uint16_t i = 0; i < upf_proc_buff->size; i++)
    {
        if (upf_proc_buff->buffer[i] == PTL_LHLL_C2I_HEADER)
        {
            // remove data before header
            for (uint16_t j = i; j < upf_proc_buff->size; j++)
            {
                upf_proc_buff->buffer[j - i] = upf_proc_buff->buffer[j];
            }
            upf_proc_buff->size -= i;
            break;
        }
    }

    // no find A2M_PTL_HEADER,clear all;
    upf_proc_buff->size = 0;
}

/**
 * @brief Search for a valid protocol frame in the processing buffer.
 *
 * @param upf_proc_buff Pointer to the protocol processing buffer.
 * @return true if a valid frame is found and processed, false otherwise.
 */
bool lhl2_ptl_find_valid_frame(upf_proc_buff_t *upf_proc_buff)
{
    uint16_t offset = 0;            // Offset to current header candidate
    uint8_t framelen = 0;           // Length of the potential frame
    uint8_t crc;                    // Calculated CRC
    uint8_t crc_read = 0;           // CRC value read from the buffer
    bool frame_crc_ok = false;      // Flag indicating CRC validity
    bool find = false;              // Whether a valid frame is found
    uint16_t next_valid_offset = 0; // Offset to next search position
    bool header_invalid = false;    // Flag for invalid header

    for (uint16_t i = 0; i < upf_proc_buff->size; i++)
    {
        // Check for frame header
        if (upf_proc_buff->buffer[i] == PTL_LHLL_C2I_HEADER)
        {
            offset = i;

            // Ensure there is at least one more byte to read frame length
            if ((i + 1) >= upf_proc_buff->size)
                break;

            framelen = upf_proc_buff->buffer[i + 1];

            // If frame length is invalid and we're at the start, skip the minimum frame size
            if ((framelen < PTL_LHLL_FRAME_MIN_SIZE) &&
                (upf_proc_buff->size >= PTL_LHLL_FRAME_MIN_SIZE) &&
                (offset == 0))
            {
                next_valid_offset = PTL_LHLL_FRAME_MIN_SIZE;
            }
            // Frame length seems valid, and enough data remains in buffer
            else if ((framelen >= PTL_LHLL_FRAME_MIN_SIZE) &&
                     (framelen <= (upf_proc_buff->size - offset)))
            {
                // Calculate CRC over the frame excluding last byte (CRC byte)
                crc = lhl2_ptl_checksum(&upf_proc_buff->buffer[offset], framelen - 1);
                crc_read = upf_proc_buff->buffer[offset + framelen - 1];

                frame_crc_ok = (crc == crc_read);

                if (frame_crc_ok)
                {
                    // Valid frame found
                    next_valid_offset = offset + framelen;
                    find = true;
                    break;
                }
                else
                {
                    // Header is invalid; try skipping it
                    if ((offset == 0) || header_invalid)
                    {
                        header_invalid = true;
                        next_valid_offset = framelen + offset;
                    }
                }
            }
            else
            {
                // Not enough data remaining, continue search
            }
        }
    }

    // If valid frame found, process it
    if (find)
    {
        if (framelen < 13)
        {
            LOG_LEVEL("Not enough data framelen=%d\r\n", framelen);
        }
        lhl2_ptl_proc_valid_frame(upf_proc_buff->buffer + offset, framelen);
    }

    // TODO: implement timeout check, clear data if timeout

    // Remove processed or invalid data from the buffer
    if (next_valid_offset != 0)
    {
        for (uint16_t i = next_valid_offset; i < upf_proc_buff->size; i++)
        {
            upf_proc_buff->buffer[i - next_valid_offset] = upf_proc_buff->buffer[i];
        }

        upf_proc_buff->size -= next_valid_offset;
    }

    return find;
}

// 根据一圈所用时间（ms）计算车速（返回 km/h * 10）
// 参数 round_ms: 一圈用时（毫秒）
// get_wheel_radius_inch(): 返回轮毂半径，单位 0.1 英寸
uint16_t calculate_speed_kph_x10(uint16_t round_ms)
{
    if (round_ms == 0)
        return 0;

    // 常数 5748 = π × 2 × 0.0254 × 3600 × 10
    // π × 2（周长公式）→ 单位：米
    // 0.0254（英寸→米）
    // 3600（秒→小时）
    // 10（保留 1 位小数）
    uint32_t wheel_r = get_wheel_radius_inch();                        // 半径，0.1 英寸
    uint32_t speed_x10 = (wheel_r * 5748UL + round_ms / 2) / round_ms; // 四舍五入

    return (uint16_t)(speed_x10 / 10);
}

// #define  YONGJIU_WHEEL_Inch 284 //284,700C50C,700C的车圈直径为622毫米,理论直径?：622mm+(50mm×2)=722mm（约28.4英寸）
// data is payload, len is payload length
void lhl2_ptl_proc_valid_frame(uint8_t *data, uint16_t length) // RX
{
    uint8_t state1 = data[3];
    uint8_t state2 = data[4];
    // uint16_t cur = MK_WORD(data[5], data[6]);
    // uint8_t cur_pre = data[7];
    uint16_t round = MK_WORD(data[8], data[9]);
    // uint8_t soc = data[10];
    // uint8_t range = MK_WORD(data[11], data[12]);

    // LOG_BUFF_LEVEL(data, length);
#if 0
    // 实际电流数据
    uint32_t cur_tmp = cur & 0x3FFF;

    // 判断是否为1A单位
    if (1 == (cur & 0x4000))
    {
        cur_tmp = cur_tmp * 10;
    }
		
    lt_carinfo_battery.current = cur_tmp;
    lt_carinfo_battery.power = lt_carinfo_battery.voltage * lt_carinfo_battery.current; /// 100;
    lt_carinfo_battery.soc = soc;

    if (range > 0)
    {
        lt_carinfo_battery.range = range;
		if (lt_carinfo_battery.soc == 0)
		{
			lt_carinfo_battery.soc = range / lt_carinfo_battery.range_max;
		}
    }
    else 
    {
		if (lt_carinfo_battery.soc > 0)
			lt_carinfo_battery.range = (lt_carinfo_battery.range_max * soc) / 100;
		else
			lt_carinfo_battery.soc = cur_pre;
    }
#endif

    if (round <= 10 || round >= 3500) // 如果一圈的时间大于等于30S，认为车辆已经停止
    {
        lt_carinfo_meter.rpm = 0;
        lt_carinfo_meter.speed_actual = 0;
    }
    else
    {
        lt_carinfo_meter.rpm = 60.0 * 1000 / round; // 245
        // double radius = get_wheel_radius_inch();  //轮毂半径，单位：米  //0.33
        // double w = rpm * (2.0 * PI_FLOAT / 60.0); //转换角速度，单位：弧度/秒  //25
        // double v = w * radius;                    //线速度,单位：米/秒   //8.25
        // double kph = (v * 3600.0 / 1000.0) * 10;  // 297
        // double speed = kph * 1.618; //实物调试修正
        lt_carinfo_meter.speed_actual = (uint32_t)calculate_speed_kph_x10(round);
        // lt_carinfo_meter.speed_average = lt_carinfo_meter.speed_actual;
    }

    // 6KM巡航状态
    lt_carinfo_indicator.walk_assist = (state1 & BIT_7) ? 1 : 0; //------ 接收车机来的助推
    // 巡航状态
    lt_carinfo_indicator.cruise_control = (state1 & BIT_2) ? 1 : 0;
    // 车辆水平状态
    lt_carinfo_indicator.horizontal_position = (state2 & BIT_7) ? 1 : 0;
    // 断电刹把
    lt_carinfo_indicator.brake = (state2 & BIT_5) ? 1 : 0;
    // 充电状态
    lt_carinfo_battery.rel_charge_state = (state2 & BIT_3) ? 1 : 0;

    lt_carinfo_indicator.ready = (state2 & BIT_6) ? 1 : 0; //---ready��

    // mingnuo_4chin
    if (((state1 & BIT_6) == 0) && ((state1 & BIT_5) == 0) && ((state1 & BIT_4) == 0) && ((state1 & BIT_3) == 0) && ((state1 & BIT_0) == 0) && ((state2 & BIT_6) == 0) && ((state2 & BIT_4) == 0))
    {
        carinfo_add_error_code(ERROR_CODE_NORMAL, true, false);
        return;
    }

    if (state1 & BIT_6) // 霍尔传感器状态
        carinfo_add_error_code(ERROR_CODE_HALLSENSOR_ABNORMALITY, state1 & BIT_6, false);

    // 转把故障状态
    if (state1 & BIT_5)
        carinfo_add_error_code(ERROR_CODE_THROTTLE_HALLSENSOR_ABNORMALITY, state1 & BIT_5, false);

    // 控制器故障状态
    if (state1 & BIT_4)
        carinfo_add_error_code(ERROR_CODE_CONTROLLER_ABNORMALITY, state1 & BIT_4, false);

    // 欠压保护状态
    if (state1 & BIT_3)
        carinfo_add_error_code(ERROR_CODE_LOW_VOLTAGE_PROTECTION, state1 & BIT_3, false);

    // 电机缺相
    if (state1 & BIT_0)
        carinfo_add_error_code(ERROR_CODE_MOTOR_ABNORMALITY, state1 & BIT_0, false);

    // 助力传感器状态
    if (state2 & BIT_6)
    {
        if (lt_carinfo_error.fault_sensor)
            carinfo_add_error_code(ERROR_CODE_ASSIST_POWER_SENSOR_ABNORMALITY, state2 & BIT_6, false);
    }

    // 通讯故障
    if (state2 & BIT_4)
    {
        carinfo_add_error_code(ERROR_CODE_BMS_ABNORMALITY, state2 & BIT_4, false);
    }
}

uint8_t lhl2_ptl_checksum(uint8_t *data, uint8_t len)
{
    uint8_t sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum ^= data[i];
    }
    return sum;
}

uint16_t get_wheel_radius_inch(void)
{
    if (lt_carinfo_meter.wheel_diameter >= 100)
    {
        return lt_carinfo_meter.wheel_diameter;
    }

    switch (lt_carinfo_meter.wheel_diameter)
    {
    case SETTING_WHEEL_16_Inch:
        return 160;
    case SETTING_WHEEL_18_Inch:
        return 180;
    case SETTING_WHEEL_20_Inch:
        return 200;
    case SETTING_WHEEL_22_Inch:
        return 220;
    case SETTING_WHEEL_24_Inch:
        return 240;
    case SETTING_WHEEL_26_Inch:
        return 260;
    case SETTING_WHEEL_27_Inch:
        return 270;
    case SETTING_WHEEL_27_5_Inch:
        return 275;
    case SETTING_WHEEL_28_Inch:
        return 280;
    case SETTING_WHEEL_29_Inch:
        return 290;
    }

    return 260;
}

uint16_t get_geer_level(void)
{
    uint16_t geer = 0;
    // return lt_carinfo_meter.gear;
#if 1
    if (lt_carinfo_meter.gear_level_max == SETTING_MAX_PAS_3_LEVEL)
    {

        switch (lt_carinfo_meter.gear)
        {
        case 0:
            geer = 0;
            break;
        case 1:
            geer = 5;
            break;
        case 2:
            geer = 10;
            break;
        case 3:
            geer = 15;
            break;
        default:
            geer = 0;
            break;
        }
    }
    else if (lt_carinfo_meter.gear_level_max == SETTING_MAX_PAS_5_LEVEL)
    {
        switch (lt_carinfo_meter.gear)
        {
        case 0:
            geer = 0;
            break;
        case 1:
            geer = 3;
            break;
        case 2:
            geer = 6;
            break;
        case 3:
            geer = 9;
            break;
        case 4:
            geer = 12;
            break;
        case 5:
            geer = 15;
            break;
        default:
            geer = 0;
            break;
        }
    }
    else if (lt_carinfo_meter.gear_level_max == SETTING_MAX_PAS_9_LEVEL)
    {
        switch (lt_carinfo_meter.gear)
        {
        case 0:
            geer = 0;
            break;
        case 1:
            geer = 1;
            break;
        case 2:
            geer = 3;
            break;
        case 3:
            geer = 5;
            break;
        case 4:
            geer = 7;
            break;
        case 5:
            geer = 9;
            break;
        case 6:
            geer = 11;
            break;
        case 7:
            geer = 13;
            break;
        case 8:
            geer = 14;
            break;
        case 9:
            geer = 15;
            break;
        default:
            geer = 0;
            break;
        }
    }
    return geer;
#endif
}

bool lhl2_ptl_receive_handler(upf_proc_buff_t *upf_proc_buff)
{
    lhl2_ptl_remove_none_header_data(upf_proc_buff);
    return lhl2_ptl_find_valid_frame(upf_proc_buff);
}
#endif
