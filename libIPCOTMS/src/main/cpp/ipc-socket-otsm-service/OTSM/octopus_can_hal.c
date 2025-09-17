/******************************************************************************/
/* Header file contains */
#include "octopus_can_hal.h"

#ifdef TASK_MANAGER_STATE_MACHINE_CAN
void Can_Init(void)
{
}

void Can_Deinit(void)
{
}

bool Can_SendMsg(uint8_t mailBox, uint32_t id, uint8_t *data, uint8_t len)
{
    return false;
}
#endif
/******************************** endfile @ dio ******************************/
