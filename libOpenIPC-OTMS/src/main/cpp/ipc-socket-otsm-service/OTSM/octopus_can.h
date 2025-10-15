// File: octopus_can.h
// Description: Header for CAN data dispatching and parsing logic
// Author: ak47
// Created: 2025-04-17

#ifndef __OCTOPUS_CAN_H__
#define __OCTOPUS_CAN_H__

#include "octopus_base.h" //  Base include file for the Octopus project.

// Default CAN data length (in bytes)
#define CAN_DATA_MAX_LENGTH 8

typedef enum
{
    CAN_OK = 0,
    CAN_ERR_FIFO_FULL,
    CAN_ERR_PARAM,
    CAN_ERR_TRANSMIT,
    CAN_ERR_TIMEOUT,
    CAN_ERR_UNKNOWN
} CAN_Status_t;

// Structure to represent a CAN message
typedef struct
{
    uint32_t StdId; /*!< Specifies the standard identifier.
                       This parameter can be a value between 0 to 0x7FF. */

    uint32_t ExtId; /*!< Specifies the extended identifier.
                       This parameter can be a value between 0 to 0x1FFFFFFF. */

    uint8_t IDE; /*!< Specifies the type of identifier for the message that
                    will be received. This parameter can be a value of
                    @ref CAN_identifier_type */

    uint8_t RTR; /*!< Specifies the type of frame for the received message.
                    This parameter can be a value of
                    @ref CAN_remote_transmission_request */

    uint8_t DLC; /*!< Specifies the length of the frame that will be received.
                    This parameter can be a value between 0 to 8 */

    uint8_t Data[8]; /*!< Contains the data to be received. It ranges from 0 to
                        0xFF. */

    uint8_t FMI; /*!< Specifies the index of the filter the message stored in
                    the mailbox passes through. This parameter can be a
                    value between 0 to 0xFF */
} CAN_Message_t;

// 发送结构体示例宏
static inline void print_can_error(uint32_t id, uint8_t ide, CAN_Status_t status)
{
    const char *id_type = ide ? "ExtID" : "StdID";
    const char *msg = "Unknown error";

    switch (status)
    {
    case CAN_OK:
        return; // 无错误，不打印
    case CAN_ERR_PARAM:
        msg = "Parameter invalid";
        break;
    case CAN_ERR_TIMEOUT:
        msg = "Transmission timeout";
        break;
    case CAN_ERR_TRANSMIT:
        msg = "Transmit mailbox error";
        break;
    default:
        msg = "Unknown error";
        break;
    }

    if (ide)
    {
        LOG_LEVEL("CAN ERROR %s: 0x%08X, %s \r\n", id_type, id, msg);
    }
    else
    {
        LOG_LEVEL("CAN ERROR %s: 0x%03X, %s \r\n", id_type, id, msg);
    }
}

#define CAN_SEND_STRUCT_STD(ID, STRUCT_PTR)                          \
    do                                                               \
    {                                                                \
        CAN_Status_t status = CAN_Send_Data((ID), 0,                 \
                                            (uint8_t *)(STRUCT_PTR), \
                                            sizeof(*(STRUCT_PTR)));  \
        if (status != CAN_OK)                                        \
        {                                                            \
            print_can_error((ID), 0, status);                        \
        }                                                            \
    } while (0)

#define CAN_SEND_STRUCT_EXT(ID, STRUCT_PTR)                          \
    do                                                               \
    {                                                                \
        CAN_Status_t status = CAN_Send_Data((ID), 1,                 \
                                            (uint8_t *)(STRUCT_PTR), \
                                            sizeof(*(STRUCT_PTR)));  \
        if (status != CAN_OK)                                        \
        {                                                            \
            print_can_error((ID), 1, status);                        \
        }                                                            \
    } while (0)

#ifdef __cplusplus
extern "C"
{
#endif
    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // ========================================================
    //                Function Declarations
    // ========================================================

    /**
     * @brief Parse a received CAN message and process it according to its ID.
     * @param msg Pointer to a CAN_Message_t structure containing received data.
     */
    void can_message_receiver(const CAN_Message_t *msg);

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    void task_can_init_running(void);
    void task_can_start_running(void);
    void task_can_assert_running(void);
    void task_can_running(void);
    void task_can_post_running(void);
    void task_can_stop_running(void);

#ifdef __cplusplus
}
#endif

#endif // OCTOPUS_CAN_H
