/*******************************************************************************
 * @file    octopus_cfifo.h
 * @brief   Header file for circular FIFO (First-In-First-Out) buffer implementation.
 *          Provides macros, type definitions, and function declarations for
 *          managing a circular FIFO.
 *
 * @details
 * This file defines the `cFifo_t` structure and its associated operations,
 * including initialization, push, pop, and utility macros for capacity and data size
 * calculations. It is designed to support efficient data management in embedded systems.
 *
 * @version  1.0.0
 * @date     2024-12-09
 * @author   Octopus Team

 ******************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_CFIFO_H__
#define __OCTOPUS_TASK_MANAGER_CFIFO_H__

#include "octopus_base.h" //  Base include file for the Octopus project.

/*******************************************************************************
 * TYPE DEFINITIONS
 */

/**
 * @brief Circular FIFO structure.
 */
#pragma pack(push, 1)
// #pragma pack(push)
// #pragma pack(1) // Ensure structure is packed for memory alignment
typedef struct
{
	volatile uint32_t head;		/**< Input index (read pointer). */
	volatile uint32_t tail;		/**< Output index (write pointer). */
	volatile uint32_t capacity; /**< FIFO capacity (size of the buffer). */
	// uint8_t aucArr[];           /**< FIFO data array. */
	uint8_t *buffer;
} cFifo_t;
#pragma pack(pop)

/*******************************************************************************
 * MACROS
 */

/**
 * @brief Calculate the size of the FIFO object based on buffer size.
 * @param _bufferSize Size of the data buffer.
 */
#define cFifo_ObjSize(_bufferSize) (sizeof(cFifo_t) + (_bufferSize))

/**
 * @brief Check if the FIFO is empty.
 * @param ptFifo Pointer to the circular FIFO structure.
 */
#define cFifo_isEmpty(ptFifo) ((ptFifo)->head == (ptFifo)->tail ? true : false)

/**
 * @brief Check if the FIFO is full.
 * @param ptFifo Pointer to the circular FIFO structure.
 */
#define cFifo_isFull(ptFifo) \
	((ptFifo)->head == (((ptFifo)->tail + 1) % (ptFifo)->capacity) ? true : false)

/**
 * @brief Clear the FIFO by resetting its head and tail pointers.
 * @param ptFifo Pointer to the circular FIFO structure.
 */
#define cFifo_Clear(ptFifo) ((ptFifo)->head = (ptFifo)->tail = 0)

/**
 * @brief Get the capacity of the FIFO.
 * @param ptFifo Pointer to the circular FIFO structure.
 */
#define cFifo_Capacity(ptFifo) ((ptFifo)->capacity)

/**
 * @brief Get the number of data bytes currently stored in the FIFO.
 * @param ptFifo Pointer to the circular FIFO structure.
 */
#define cFifo_DataSize(ptFifo) \
	(((ptFifo)->tail >= (ptFifo)->head) ? (ptFifo)->tail - (ptFifo)->head : (ptFifo)->capacity - (ptFifo)->head + (ptFifo)->tail)

/**
 * @brief Get the number of free bytes available in the FIFO.
 * @param ptFifo Pointer to the circular FIFO structure.
 */
#define cFifo_FreeSize(ptFifo) \
	(((ptFifo)->capacity) -    \
	 (((ptFifo)->tail >= (ptFifo)->head) ? (ptFifo)->tail - (ptFifo)->head : (ptFifo)->capacity - (ptFifo)->head + (ptFifo)->tail))

/*******************************************************************************
 * FUNCTION DECLARATIONS
 */

/**
 * @brief Initialize the circular FIFO.
 * @param[out] ptFifo Pointer to the circular FIFO structure pointer.
 * @param[in] pucMem Pointer to the memory buffer to be used for the FIFO.
 * @param[in] uiMemSize Size of the memory buffer in bytes.
 */
void cFifo_Init(cFifo_t **ptFifo, uint8_t *pucMem, uint32_t uiMemSize);

/**
 * @brief Push a byte of data into the circular FIFO.
 * @param[in] a_ptFifo Pointer to the circular FIFO structure.
 * @param[in] a_u8Data The byte of data to be pushed into the FIFO.
 * @return True if the data was successfully pushed, false if the FIFO is full.
 */
bool cFifo_Push(cFifo_t *a_ptFifo, uint8_t a_u8Data);

/**
 * @brief Pop a byte of data from the circular FIFO.
 * @param[in] a_ptFifo Pointer to the circular FIFO structure.
 * @param[out] a_pu8Data Pointer to store the byte of data popped from the FIFO.
 * @return True if data was successfully popped, false if the FIFO is empty.
 */
bool cFifo_Pop(cFifo_t *a_ptFifo, uint8_t *a_pu8Data);

/**
 * @brief Check if a full line (ending with '\n') is present in the FIFO.
 * @param[in] a_ptFifo Pointer to the circular FIFO structure.
 * @return True if a full line is available, false otherwise.
 */
bool cFifo_HasLine(cFifo_t *a_ptFifo);

#endif // __OCTOPUS_TASK_MANAGER_CFIFO_H__
