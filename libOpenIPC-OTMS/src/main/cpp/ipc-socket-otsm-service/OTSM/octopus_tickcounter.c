/** ****************************************************************************
 * @copyright Copyright (c) XXX
 * All rights reserved.
 *
 */

/*******************************************************************************
 * INCLUDES
 */

#include "octopus_tickcounter.h" // Includes the header file that defines tick counter functions
#include "octopus_platform.h"    // Include platform-specific header for hardware platform details

#if 1 // def TASK_MANAGER_STATE_MACHINE_PTL1
/*******************************************************************************
 * DEBUG SWITCH MACROS
 */

/*******************************************************************************
 * MACROS
 */
#define TICK_COUNTER_MAX ((uint32_t)-1) // Maximum value for tick counter (32-bit unsigned integer)

/*******************************************************************************
 * TYPEDEFS
 */

/**
 * \defgroup GROUP_LOCAL_FUNCTIONS OS:TIMER:LOCAL_FUNCTIONS
 * Functions for managing the system timer
 */

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * LOCAL FUNCTIONS DECLEAR
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * STATIC VARIABLES
 */
// static uint32_t OsTickCounter = 0; // Stores the current tick value

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 *  GLOBAL FUNCTIONS IMPLEMENTATION
 */

// Start the tick counter by recording the system tick time one ms == one tick
void StartTickCounter(uint32_t *timer)
{
    MY_ASSERT(timer != NULL); // Ensures the pointer is not NULL

    uint32_t system_tick_count = GetSystemTickClock(); // Get the current system tick value

    // If the tick counter is 0, initialize the timer with 1
    if (system_tick_count == 0U)
    {
        *timer = 1;
    }
    else
    {
        *timer = system_tick_count; // Set the timer to the current tick value
    }
}

// Stop the tick counter by setting the timer to 0
void StopTickCounter(uint32_t *timer)
{
    MY_ASSERT(timer != NULL); // Ensures the pointer is not NULL
    *timer = 0;               // Stop the timer
}

// Restart the tick counter by starting it again
void RestartTickCounter(uint32_t *timer)
{
    StartTickCounter(timer); // Calls StartTickCounter to restart
}

// Get the difference in ticks between the current time and the timer (in ms)
uint32_t GetTickCounter(const uint32_t *timer) // Returns time difference in milliseconds
{
    uint32_t diff;
    MY_ASSERT(timer != NULL); // Ensures the pointer is not NULL

    uint32_t system_tick_count = GetSystemTickClock(); // Get the current system tick value

    if (0 == *timer)
    {
        diff = 0U; // If timer was not started, return 0
    }
    else
    {
        // If the current tick is greater than or equal to the timer, simply subtract
        if (system_tick_count >= *timer)
        {
            diff = system_tick_count - *timer;
        }
        else
        {
            // Handle counter overflow (wraparound)
            diff = (TICK_COUNTER_MAX - *timer) + system_tick_count;
        }
    }
    return diff; // Return the time difference in ticks
}

// Check if the tick counter has started
bool IsTickCounterStart(const uint32_t *timer)
{
    return *timer != 0; // Returns true if the timer is non-zero, meaning it has started
}

// Get the current system tick (in milliseconds)
uint32_t GetSystemTickClock(void)
{
    // return GET_SYSTEM_TICK_COUNT; // Calls a macro or function to get the system tick count

    DISABLE_IRQ;
    uint32_t system_tick = GET_SYSTEM_TICK_COUNT;
    ENABLE_IRQ;
    return system_tick;
}

// Function to convert date string into tm structure (parsed from string)
#if 1
void Date2tm(struct tm *pTM, const char *pData)
{
    struct tm timeInfo;
    char *tokenPtr = NULL;
    char dataTimeTest[40] = {0}; // Buffer for date/time string
    char arrDate[20] = {0};      // Buffer for the date part
    char arrTime[20] = {0};      // Buffer for the time part

    if (NULL == pData || NULL == pTM) // Validate input parameters
        return;

    memset(&timeInfo, 0, sizeof(struct tm)); // Clear the tm structure
    strcpy(dataTimeTest, pData);             // Copy the input date-time string
    dataTimeTest[39] = 0;                    // Ensure null-termination

    // Split the string into date and time parts
    tokenPtr = strtok(dataTimeTest, " ");
    if (tokenPtr)
    {
        strcpy(arrDate, tokenPtr); // Extract the date part
        tokenPtr = strtok(NULL, " ");
        if (tokenPtr)
        {
            strcpy(arrTime, tokenPtr); // Extract the time part
        }
    }

    // Parse the date part
    tokenPtr = strtok(arrDate, ".");
    if (tokenPtr)
        timeInfo.tm_mday = atoi(tokenPtr); // Extract day
    tokenPtr = strtok(NULL, ".");
    if (tokenPtr)
        timeInfo.tm_mon = atoi(tokenPtr); // Extract month
    tokenPtr = strtok(NULL, ".");
    if (tokenPtr)
        timeInfo.tm_year = atoi(tokenPtr); // Extract year

    // Parse the time part
    tokenPtr = strtok(arrTime, ":");
    if (tokenPtr)
        timeInfo.tm_hour = atoi(tokenPtr); // Extract hour
    tokenPtr = strtok(NULL, ":");
    if (tokenPtr)
        timeInfo.tm_min = atoi(tokenPtr); // Extract minutes
    tokenPtr = strtok(NULL, ":");
    if (tokenPtr)
        timeInfo.tm_sec = atoi(tokenPtr); // Extract seconds

    *pTM = timeInfo; // Store the parsed time in the provided struct tm pointer
}
#endif
#endif
