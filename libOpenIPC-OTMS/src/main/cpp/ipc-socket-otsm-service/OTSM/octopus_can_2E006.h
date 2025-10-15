
#ifndef __OCTOPUS_CAN_FUNCTION_2E006__
#define __OCTOPUS_CAN_FUNCTION_2E006__


#include "octopus_base.h" //  Base include file for the Octopus project.
#include "octopus_can_queue.h"
#include "octopus_tickcounter.h"
#include "octopus_vehicle.h"

#ifdef TASK_MANAGER_STATE_MACHINE_CAN
// -------- BMS --------
#pragma pack(push, 1)
typedef struct
{
    uint8_t soc_percent;          // 电池SOC百分比 [%]
    int16_t max_discharge_A;      // 最大允许放电电流 [A]
    uint16_t remain_capacity_mAh; // 剩余容量 [mAh]
    int16_t realtime_current_A;   // 实时电流 [A]
    uint8_t fault_code;           // 故障代码
} BMS_Task_H_001_t;

typedef struct
{
    uint16_t voltage_V; // 总电压 [0.1V/bit]
    int16_t current_A;  // 总电流 [0.1A/bit, 偏移 -400A]
    int8_t temp_min_C;  // 最小温度 [°C]
    int8_t temp_max_C;  // 最大温度 [°C]
} BMS_Task_H_002_t;

// -------- MCU --------
// MCU_Task_H_001 CAN 报文 (ID: 0x170, DLC=8)
typedef struct
{
    uint16_t motor_speed_rpm; // 电机转速 [RPM] (Byte0~1)

    // 控制器工作信息 (Byte2~3)
    __packed union
    {
        uint16_t raw;
        __packed struct
        {
            uint16_t pReady : 1;      // 待机状态
            uint16_t sideStand : 1;   // 边撑状态
            uint16_t cruise : 1;      // 巡航状态
            uint16_t charge : 1;      // 充电状态
            uint16_t forwardGear : 2; // 前进档位 (00无档 01低档 10高档)
            uint16_t antiRob : 1;     // 防盗状态
            uint16_t brake : 1;       // 制动状态

            uint16_t ebs : 1;      // EBS能量回馈
            uint16_t seat : 1;     // 座桶状态
            uint16_t comm : 1;     // 通信状态
            uint16_t workMode : 2; // 工作模式 (00转矩 01转速 10PWM)
            uint16_t pwmState : 1; // PWM波状态
            uint16_t reserved : 2; // 保留
        } bits;
    } info; // Byte2~3

    // 故障状态 (Byte4~7)
    __packed union
    {
        uint32_t raw;
        __packed struct
        {
            uint32_t mosFault : 1;       // MOS故障
            uint32_t overCurrent : 1;    // 过流
            uint32_t overVoltage : 1;    // 过压
            uint32_t underVoltage : 1;   // 欠压
            uint32_t throttleFault : 1;  // 转把故障
            uint32_t pLockFault : 1;     // 电机堵转
            uint32_t motorHallFault : 1; // 电机霍尔故障
            uint32_t angleFault : 1;     // 角度传感器故障

            uint32_t otherFault : 1;     // 其它故障
            uint32_t derating : 1;       // 降额状态
            uint32_t mcuOverTemp1 : 1;   // MCU过温1
            uint32_t mcuOverTemp2 : 1;   // MCU过温2
            uint32_t motorOverTemp1 : 1; // 电机过温1
            uint32_t motorOverTemp2 : 1; // 电机过温2
            uint32_t reserved : 18;      // 保留
        } bits;
    } fault_state; // Byte4~7

} MCU_Task_H_001_t;

typedef struct
{
    int8_t mcu_temp;       // 控制器温度 [°C] (raw-40)
    int8_t motor_temp;     // 电机温度 [°C] (raw-40)
    int16_t bus_current;   // 母线电流 [0.1A] (raw-1950)
    int16_t phase_current; // 相电流 [0.1A] (raw-4000)
    uint8_t bus_voltage;   // 母线电压 [V]
    uint8_t throttle_volt; // 油门电压 [0.5V/bit]
} MCU_Task_H_002_t;        // 0x171 报文

// -------- VCU --------
typedef struct
{
    uint8_t gear;       // 档位
    uint8_t work_mode;  // 工作模式
    uint8_t state_bits; // 驱动/辅助状态位
} VCU_Task_L_001_t;

typedef struct
{
    uint8_t power_limit;   // 功率限制 [%]
    uint8_t regen_level;   // 能量回收等级
    uint8_t alive_counter; // Alive计数器
} VCU_Task_L_002_t;

// -------- Charger --------
typedef struct
{
    uint16_t output_voltage_V; // 输出电压 [0.01V/bit]
    uint16_t output_current_A; // 输出电流 [0.05A/bit]
    int8_t temp_C;             // 温度 [°C, 偏移 -40]
    uint8_t alive_counter;     // Alive计数器
} Charger_Task_L_001_t;

typedef struct
{
    uint8_t charger_state; // 充电机状态
    uint8_t fault_code;    // 故障代码
    uint8_t alive_counter; // Alive计数器
} Charger_Task_L_002_t;

// -------- ABS --------
typedef struct
{
    uint8_t abs_state;        // ABS 状态
    uint16_t front_speed_rpm; // 前轮转速 [RPM]
    uint16_t rear_speed_rpm;  // 后轮转速 [RPM]
    uint8_t dtc;              // 故障代码
} ABS_Task_L_001_t;

// -------- EBS --------
typedef struct
{
    uint8_t ebs_state;       ///< EBS state (Byte0)
    uint16_t front_pressure; ///< Front brake pressure [0.1MPa/bit] (Byte1~2)
    uint16_t rear_pressure;  ///< Rear brake pressure [0.1MPa/bit] (Byte3~4)
    uint8_t fault_code;      ///< Fault code (Byte5)
    uint8_t alive_counter;   ///< Alive counter (Byte6)
    uint8_t reserved;        ///< Reserved (Byte7)
} EBS_Task_L_001_t;
#pragma pack(pop)

// ================= CAN Message ID define =================
#define CAN_ID_BMS_TASK_H_001 0x150
#define CAN_ID_BMS_TASK_H_002 0x151

#define CAN_ID_VCU_TASK_L_001 0x160
#define CAN_ID_VCU_TASK_L_002 0x201

#define CAN_ID_MCU_TASK_H_001 0x170
#define CAN_ID_MCU_TASK_H_002 0x171

#define CAN_ID_CHARGER_TASK_L_001 0x182
#define CAN_ID_CHARGER_TASK_L_002 0x202

#define CAN_ID_ABS_TASK_L_001 0x300
#define CAN_ID_EBS_LEVEL 0x180

// ========== BMS ==========
void CAN_Parse_BMS_Task_H_001(const CanQueueMsg_t *msg);
void CAN_Parse_BMS_Task_H_002(const CanQueueMsg_t *msg);

// ========== VCU ==========
void CAN_Parse_VCU_Task_L_001(const CanQueueMsg_t *msg);
void CAN_Parse_VCU_Task_L_002(const CanQueueMsg_t *msg);

// ========== MCU ==========
bool CAN_Parse_MCU_Task_H_001(const CanQueueMsg_t *msg);
bool CAN_Parse_MCU_Task_H_002(const CanQueueMsg_t *msg);

// ========== Charger ==========
void CAN_Parse_Charger_Task_L_001(const CanQueueMsg_t *msg);
void CAN_Parse_Charger_Task_L_002(const CanQueueMsg_t *msg);

// ========== ABS / EBS ==========
void CAN_Parse_ABS_Task_L_001(const CanQueueMsg_t *msg);
void CAN_Parse_EBS_Level(const CanQueueMsg_t *msg);

/* ========================================
 * Battery Management System (BMS) Faults
 * ======================================== */
#define BMS_FAULT_SHORT_CIRCUIT 0x01     // 短路故障
#define BMS_FAULT_OVER_TEMPERATURE 0x02  // 过温故障
#define BMS_FAULT_UNDER_TEMPERATURE 0x03 // 低温故障
#define BMS_FAULT_OVER_VOLTAGE 0x04      // 过压故障
#define BMS_FAULT_UNDER_VOLTAGE 0x05     // 欠压故障
#define BMS_FAULT_OVER_CURRENT 0x06      // 过流故障

/* ========================================
 * Motor Control Unit (MCU) Faults
 * ======================================== */
#define MCU_FAULT_ROTOR_LOCKED 0x10     // 电机堵转
#define MCU_FAULT_HALL_ERROR 0x11       // 电机霍尔故障
#define MCU_FAULT_ANGLE_ERROR 0x12      // 电机角度异常
#define MCU_FAULT_OTHER 0x13            // 其他故障
#define MCU_FAULT_DERATING 0x14         // 降额运行
#define MCU_FAULT_OVER_TEMP1 0x15       // 控制器过温限流
#define MCU_FAULT_OVER_TEMP2 0x16       // 控制器过温关断
#define MCU_FAULT_MOTOR_OVER_TEMP1 0x17 // 电机过温限流
#define MCU_FAULT_MOTOR_OVER_TEMP2 0x18 // 电机过温关断

/* ========================================
 * Charger Faults
 * ======================================== */
#define CHARGER_FAULT_HARDWARE 0x20  // 硬件故障
#define CHARGER_FAULT_COMM 0x21      // 通信故障
#define CHARGER_FAULT_SHORT 0x22     // 短路保护
#define CHARGER_FAULT_OPEN 0x23      // 开路故障
#define CHARGER_FAULT_INPUT_UV 0x24  // 输入欠压
#define CHARGER_FAULT_OUTPUT_OC 0x25 // 输出过流
#define CHARGER_FAULT_OUTPUT_OV 0x26 // 输出过压
#define CHARGER_FAULT_OUTPUT_UV 0x27 // 输出欠压
#define CHARGER_FAULT_INPUT_OV 0x28  // 输入过压
#define CHARGER_FAULT_OVER_TEMP 0x29 // 温度过高

/* ========================================
 * ABS Faults
 * ======================================== */
#define ABS_FAULT_SENSOR_SUPPLY 0x0B /// 传感器供电故障
#define ABS_FAULT_COIL_OPEN1 0x15    /// 线圈断路
#define ABS_FAULT_COIL_OPEN2 0x16
#define ABS_FAULT_COIL_OPEN3 0x17
#define ABS_FAULT_COIL_OPEN4 0x18
#define ABS_FAULT_FRONT_SENSOR_ERR 0x1F  /// 前轮传感器断路或线束反接
#define ABS_FAULT_BACK_SENSOR_ERR 0x20   /// 后轮传感器断路或线束反接
#define ABS_FAULT_SENSOR_FRONT_OPEN 0x29 /// 前轮传感器断路
#define ABS_FAULT_SENSOR_REAR_OPEN 0x2A  /// 后轮传感器断路
#define ABS_FAULT_FRONT_SIGNAL_LOST 0x33 /// 前轮速信号丢失
#define ABS_FAULT_REAR_SIGNAL_LOST 0x34  /// 后轮速信号丢失
#define ABS_FAULT_MOTOR_OPEN 0x3D        /// ABS电机断路
#define ABS_FAULT_VALVE_NO_POWER 0x3E    /// ABS电磁阀无12V电源
#define ABS_FAULT_FRONT_SIGNAL_ABN 0x47  /// 前轮信号异常增多
#define ABS_FAULT_REAR_SIGNAL_ABN 0x48   /// 后轮信号异常增多

#define EBS_FAULT_FRONT_SENSOR_ERR 0x50 ///< Front brake pressure sensor error
#define EBS_FAULT_REAR_SENSOR_ERR 0x51  ///< Rear brake pressure sensor error
#define EBS_FAULT_COMM_ERROR 0x52       ///< Communication error
#define EBS_FAULT_OVER_TEMP 0x53        ///< Over-temperature protection
#define EBS_FAULT_LOW_VOLTAGE 0x54      ///< Low supply voltage
#define EBS_FAULT_HIGH_VOLTAGE 0x55     ///< High supply voltage

#define VCU_FAULT_GEAR_SIGNAL_LOST 0x60  ///< Gear signal lost
#define VCU_FAULT_MODE_SWITCH_ERROR 0x61 ///< Mode switching error
#define VCU_FAULT_COMM_ERROR 0x62        ///< Communication error
#define VCU_FAULT_SENSOR_ERROR 0x63      ///< Sensor input error
#define VCU_FAULT_INTERNAL_ERROR 0x64    ///< Internal logic error

/**
 * @brief Dispatch a CAN message to appropriate handler based on its ID.
 * @this function may be used for routing logic in receive ISR or task.
 * @param msg Pointer to a CAN_Message_t structure.
 */
void can_message_dispatcher(const CanQueueMsg_t *queue_msg);
void can_message_sender(const uint16_t message_id);

#endif
#endif //__OCTOPUS_CAN_FUNCTION_2E006__
