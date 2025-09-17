/**
 * ****************************************************************************
 * @copyright Copyright (c) XXX
 * All rights reserved.
 *
 * C file for the Octopus Task Manager module.
 * Defines macros, includes required libraries, and declares functions.
 */

/*********************************************************************
 * INCLUDES
 */

#include "octopus_sif.h"
#include "octopus_timer_hal.h"

#define HAL_TIMER_TIMIEOUT_VALUE_US (50)
// static uint8 timer_TaskID;
// static uint8_t s_testCase = 0;
// static void TimerTest(uint8_t testCase);

#ifdef PLATFORM_CST_OSAL_RTOS

static uint32_t hal_timer_id;
void hal_timer_interrupt_callback(uint8_t event);
#elif defined(PLATFORM_ITE_OPEN_RTOS)
void hal_timer_interrupt_callback(void *data);
#endif

#ifdef PLATFORM_CST_OSAL_RTOS
void hal_timer_init(uint8_t timer_id)
{
    hal_timer_id = timer_id;
    // timer_TaskID = task_id;
    // HalTimerSet(AP_TIMER_ID_6,2000000);
    // LOG("when test this case,you can uncomment comment which in timer int function\n");
    HalTimerInit(hal_timer_interrupt_callback);
    HalTimerSet(AP_TIMER_ID_5, HAL_TIMER_TIMIEOUT_VALUE_US); // 50us for sif
    /* Set Interrupt Priority */
    /// NVIC_SetPriority(NVIC_InitStruct->NVIC_IRQChannel, NVIC_InitStruct->NVIC_IRQChannelPriority);
    LOG_LEVEL("hal timer init %d\r\n", hal_timer_id);
}

void hal_timer_interrupt_callback(uint8_t event)
{
    switch (event)
    {
    case HAL_EVT_TIMER_5:
// LOG("t5\n");
#ifdef TASK_MANAGER_STATE_MACHINE_SIF
        HalTimerMaskInt(AP_TIMER_ID_5, true);
        SIF_IO_IRQHandler();
        HalTimerMaskInt(AP_TIMER_ID_5, false);
#endif
        break;
    case HAL_EVT_TIMER_6:
        LOG("t6\n");
        break;
    case HAL_EVT_WAKEUP:
        LOG("wakeup\n");
        LOG("timer will disable when sleep,so if you want it work please init it when wakeup");
        break;
    case HAL_EVT_SLEEP:
        LOG("sleep\n");
        break;
    default:
        LOG("err ");
        break;
    }
}

#elif defined(PLATFORM_ITE_OPEN_RTOS)

static uint32_t hal_timer_id;
void *hal_timer_irq_init(void *arg)
{
#if 1
    // Interrupt test
    hal_timer_id = ITH_TIMER2;
    LOG_LEVEL("hal_timer_irq_init %d\r\n", hal_timer_id);

    ITHIntr irqn = hal_timer_id <= ITH_TIMER8 ? ITH_INTR_TIMER1 + hal_timer_id : ITH_INTR_TIMER9 + hal_timer_id - ITH_TIMER9;
    ithPrintf("hal_timer_irq_init irqn(%d)\n", irqn);

    ithTimerReset(hal_timer_id);
    // Initialize Timer IRQ
    ithIntrDisableIrq(irqn);
    ithIntrClearIrq(irqn);

    // register Timer Handler to IRQ
    ithIntrRegisterHandlerIrq(irqn, hal_timer_interrupt_callback, (void *)hal_timer_id);

    // set Timer IRQ to edge trigger
    ithIntrSetTriggerModeIrq(irqn, ITH_INTR_EDGE);

    // set Timer IRQ to detect rising edge
    ithIntrSetTriggerLevelIrq(irqn, ITH_INTR_HIGH_RISING);

    // Enable Timer IRQ
    ithIntrEnableIrq(irqn);

    ithTimerSetTimeout(hal_timer_id, HAL_TIMER_TIMIEOUT_VALUE_US);
    // PalGetClock();
    ithTimerEnable(hal_timer_id);

#endif
}
void hal_timer_init(uint8_t timer_id)
{
    pthread_t task;
    pthread_attr_t attr;

    hal_timer_id = timer_id;
    pthread_create(&task, &attr, hal_timer_irq_init, NULL);
}

void hal_timer_interrupt_callback(void *data)
{
    // uint32_t timer = (uint32_t)data;
    ithTimerDisable(hal_timer_id);
    ithTimerReset(hal_timer_id);

    // SIF_IO_IRQHandler();
    LOG_LEVEL("hal_timer_interrupt_callback\r\n");

    ithTimerSetTimeout(hal_timer_id, HAL_TIMER_TIMIEOUT_VALUE_US);
    ithTimerEnable(hal_timer_id);
}

#else
void hal_timer_init(uint8_t timer_id)
{
}

void hal_timer_interrupt_callback(uint8_t event)
{
#ifdef TASK_MANAGER_STATE_MACHINE_SIF
    SIF_IO_IRQHandler();
#endif
}

void otsm_timer_init(void)
{
}
#endif
