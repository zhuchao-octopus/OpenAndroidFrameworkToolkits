/**
 * ****************************************************************************
 * @copyright Copyright (c) XXX
 * All rights reserved.
 *
 * C file for the Octopus Task Manager module.
 * Defines macros, includes required libraries, and declares functions.
 */

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_ble_hal.h"
#include "octopus_flash.h"
/*******************************************************************************
 * DEBUG SWITCH MACROS
 */

/*******************************************************************************
 * LOCAL FUNCTIONS DECLEAR
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 *  GLOBAL FUNCTIONS IMPLEMENTATION
 */
#ifdef PLATFORM_CST_OSAL_RTOS
uint8_t hal_set_pairing_mode_onoff(bool ono_ff, uint8_t current_pairing_mode)
{
	uint8_t pairMode = GAPBOND_PAIRING_MODE_NO_PAIRING;
	if (ono_ff)
	{
		if (current_pairing_mode != GAPBOND_PAIRING_MODE_WAIT_FOR_REQ)
		{
			pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
			GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE, sizeof(uint8_t), &pairMode);
			LOG_LEVEL("GAPBondMgr_SetParameter to DEFAULT_PAIRING_MODE\r\n");
		}
		else
			pairMode = current_pairing_mode;
	}
	else
	{
		if (current_pairing_mode != GAPBOND_PAIRING_MODE_NO_PAIRING)
		{
			pairMode = GAPBOND_PAIRING_MODE_NO_PAIRING;
			GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE, sizeof(uint8_t), &pairMode);
			LOG_LEVEL("GAPBondMgr_SetParameter to GAPBOND_PAIRING_MODE_NO_PAIRING\r\n");
		}
		else
			pairMode = current_pairing_mode;
	}
	return pairMode;
}

void hal_enable_bLe_pair_mode(void)
{
	uint8_t pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
	GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE, sizeof(uint8_t), &pairMode);
}

void hal_disable_bLe_pair_mode(void)
{
	uint8_t pairMode = GAPBOND_PAIRING_MODE_NO_PAIRING;
	GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE, sizeof(uint8_t), &pairMode);
}

uint8_t hal_get_ble_rssi(uint8_t connection_id)
{
	int8 rssi = 0;
	LL_ReadRssi(connection_id, &rssi);
	return rssi;
}
#else
uint8_t hal_set_pairing_mode_onoff(bool ono_ff, uint8_t current_pairing_mode)
{
	return current_pairing_mode;
}

void hal_enable_bLe_pair_mode(void)
{
}

void hal_disable_bLe_pair_mode(void)
{
}
#endif
