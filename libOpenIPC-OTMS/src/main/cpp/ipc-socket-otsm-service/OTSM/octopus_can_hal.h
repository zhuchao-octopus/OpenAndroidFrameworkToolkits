
/******************************************************************************/
#ifndef __CAN_HAL_H__
#define __CAN_HAL_H__

#include "octopus_base.h" //  Base include file for the Octopus project.

#ifdef __cplusplus
extern "C"
{
#endif

    /*********************************************************************
     * CONSTANTS
     */

#define TX_MSGBOX_START (1UL)
#define TX_MSGBOX_NUMS (4UL)

    /*********************************************************************
     * TYPEDEFS
     */

    /*********************************************************************
     * GLOBAL VARIABLES
     */

    // CAN设备初始化
    void Can_Init(void);
    // CAN设备卸载
    void Can_Deinit(void);

    void Can_StartReceive(void);

    bool Can_SendMsg(uint8_t mailBox, uint32_t id, uint8_t *data, uint8_t len);

    /*********************************************************************
    *********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
