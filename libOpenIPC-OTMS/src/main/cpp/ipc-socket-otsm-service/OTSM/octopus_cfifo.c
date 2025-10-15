/*******************************************************************************
 * @file    octopus_cfifo.c
 * @brief   Implementation of a circular FIFO (First-In-First-Out) buffer.
 *          Provides initialization, push, and pop operations for a circular buffer.
 *
 * @details
 * This module implements a circular FIFO data structure, allowing efficient
 * data storage and retrieval in a fixed-size memory buffer. It includes functions
 * for initialization, data pushing, and data popping, with support for detecting
 * buffer full and empty states.
 *
 * @author
 * @date
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_cfifo.h" // Header file for circular FIFO definitions and declarations

/*******************************************************************************
 * DEBUG SWITCH MACROS
 */
// (Optional) Define macros for enabling or disabling debug logs specific to this module.

/*******************************************************************************
 * MACROS
 */

/**
 * @brief Increment an index in a circular buffer.
 * @param _capacity The capacity of the buffer.
 * @param _idx The current index.
 * @return The next index in the circular buffer.
 */
#define cFifo_Inc(_capacity, _idx) ((_idx + 1) % _capacity)

/*******************************************************************************
 * TYPEDEFS
 */
// No additional type definitions in this module.

/*******************************************************************************
 * CONSTANTS
 */
// No constants defined for this module.

/*******************************************************************************
 * LOCAL FUNCTION DECLARATIONS
 */
// Add forward declarations for static (local) functions if necessary.

/*******************************************************************************
 * GLOBAL VARIABLES
 */
// No global variables defined in this module.

/*******************************************************************************
 * STATIC VARIABLES
 */
// Define static (local) variables for internal use if necessary.

/*******************************************************************************
 * EXTERNAL VARIABLES
 */
// Declare external variables used in this module if necessary.

/*******************************************************************************
 * GLOBAL FUNCTIONS IMPLEMENTATION
 */

/**
 * @brief Initialize the circular FIFO.
 * @param[out] ptFifo Pointer to the circular FIFO structure pointer.
 * @param[in] pucMem Pointer to the memory buffer to be used for the FIFO.
 * @param[in] uiMemSize Size of the memory buffer in bytes.
 */
void cFifo_Init(cFifo_t **ptFifo, uint8_t *pucMem, uint32_t uiMemSize)
{
    if (ptFifo == NULL || pucMem == NULL || uiMemSize <= sizeof(cFifo_t))
        return;
    *ptFifo = (cFifo_t *)pucMem;                       // Use the provided memory for FIFO structure
    (*ptFifo)->capacity = uiMemSize - sizeof(cFifo_t); // Adjust capacity excluding metadata
    (*ptFifo)->head = 0;
    (*ptFifo)->tail = 0;
    (*ptFifo)->buffer = pucMem + sizeof(cFifo_t);
}

/**
 * @brief Push a byte of data into the circular FIFO.
 * @param[in] a_ptFifo Pointer to the circular FIFO structure.
 * @param[in] a_u8Data The byte of data to be pushed into the FIFO.
 * @return True if the data was successfully pushed, false if the FIFO is full.
 */
bool cFifo_Push(cFifo_t *a_ptFifo, uint8_t a_u8Data)
{
    bool bRet = false;

    if (!cFifo_isFull(a_ptFifo))
    {                                                                   // Check if FIFO is not full
        a_ptFifo->buffer[a_ptFifo->tail] = a_u8Data;                    // Add data to the tail
        a_ptFifo->tail = cFifo_Inc(a_ptFifo->capacity, a_ptFifo->tail); // Update tail position
        bRet = true;
    }

    return bRet;
}

/**
 * @brief Pop a byte of data from the circular FIFO.
 * @param[in] a_ptFifo Pointer to the circular FIFO structure.
 * @param[out] a_pu8Data Pointer to store the byte of data popped from the FIFO.
 * @return True if data was successfully popped, false if the FIFO is empty.
 */
bool cFifo_Pop(cFifo_t *a_ptFifo, uint8_t *a_pu8Data)
{
    bool bRet = false;

    if (!cFifo_isEmpty(a_ptFifo))
    {                                                                   // Check if FIFO is not empty
        *a_pu8Data = a_ptFifo->buffer[a_ptFifo->head];                  // Retrieve data from the head
        a_ptFifo->head = cFifo_Inc(a_ptFifo->capacity, a_ptFifo->head); // Update head position
        bRet = true;
    }

    return bRet;
}
/**
 * @brief Check if a full line (ending with '\n') is present in the FIFO.
 * @param[in] a_ptFifo Pointer to the circular FIFO structure.
 * @return True if a full line is available, false otherwise.
 */
bool cFifo_HasLine(cFifo_t *a_ptFifo)
{
    if (cFifo_isEmpty(a_ptFifo))
    {
        return false;
    }

    uint32_t idx = a_ptFifo->head;
    while (idx != a_ptFifo->tail)
    {
        if (a_ptFifo->buffer[idx] == '\n')
        {
            return true; // Found a newline = complete line
        }
        idx = cFifo_Inc(a_ptFifo->capacity, idx);
    }

    return false; // No newline found
}
