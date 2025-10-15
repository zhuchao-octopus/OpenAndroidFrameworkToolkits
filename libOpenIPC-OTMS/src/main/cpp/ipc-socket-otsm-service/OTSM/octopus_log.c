/*******************************************************************************
 * @file     octopus_log.c
 * @brief    Implements UART logging functions to provide logging capabilities
 *           with various log levels (DEBUG, INFO, WARNING, ERROR). Supports
 *           formatted log messages and buffer outputs.
 *
 * This file contains functions that support logging in the system via UART.
 * It allows logging of messages with variable arguments, formatted output,
 * and buffer printing. The log levels are configurable to control the verbosity
 * of logs, making it useful for debugging and tracking system status.
 *
 * The `dbg_log_printf` functions allow formatted messages to be printed to
 * UART, while the `dbg_log_printf_level` function includes the log level and
 * function name in the log output.
 *
 * The logging level can be dynamically set using `dbg_log_set_level`, and
 * specific buffer data can be logged using `dbg_log_printf_buffer` or
 * `dbg_log_printf_buffer_level`.
 *
 * @version  1.0.0
 * @date     2024-12-09
 * @author   Octopus Team
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_log.h" // Include logging functions for debugging
#include "octopus_uart_hal.h"
#include "octopus_platform.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define OTSM_DEBUG_MODE

uint8_t OTSM_DEBUG_UART_CHANNEL = 3;

// #define USE_MY_PRINTF
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef OTSM_DEBUG_MODE
DBG_LOG_LEVEL current_log_level = LOG_LEVEL_DEBUG;
#else
DBG_LOG_LEVEL current_log_level = LOG_LEVEL_NONE;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef USE_MY_PRINTF

#if defined(TASK_MANAGER_STATE_MACHINE_MCU) && defined(OTSM_DEBUG_MODE) && !defined(USE_MY_PRINTF)
/**
 * @brief  Retargets the C library printf function to the USART.
 * @param  None
 * @retval None
 */
PUTCHAR_PROTOTYPE
{
#if 0
	uint32_t Timeout = 0;
	FlagStatus Status;
	USART_SendData(DEBUG_UART, (uint8_t)ch);
	do
	{
		Status = USART_GetFlagStatus(DEBUG_UART, USART_FLAG_TXE);
		Timeout++;
	} while ((Status == RESET) && (Timeout != 0xFFFF));

	return (ch);
#endif
	uint8_t data[1] = {0};
	data[0] = (uint8_t)ch;
	switch (OTSM_DEBUG_UART_CHANNEL)
	{
	case 0:
		hal_com_uart0_send_buffer(data, 1);
		break;
	case 1:
		hal_com_uartl_send_buffer(data, 1);
		break;
	case 2:
		hal_com_uart2_send_buffer(data, 1);
		break;
	case 3:
		hal_com_uart3_send_buffer(data, 1);
		break;
	case 4:
		hal_com_uart4_send_buffer(data, 1);
		break;
	case 5:
		hal_com_uart5_send_buffer(data, 1);
		break;
	case 6:
		hal_com_uart6_send_buffer(data, 1);
		break;
	case 7:
		hal_com_uart7_send_buffer(data, 1);
		break;
	case 8:
		hal_com_uart8_send_buffer(data, 1);
		break;
	case 9:
		hal_com_uart9_send_buffer(data, 1);
		break;
	}
	return (ch);
}
#endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LOG_DEFAULT_MAX_WIDTH 28
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_MY_PRINTF

#define ZEROPAD 1  // Pad with zero
#define SIGN 2	   // Unsigned/signed long
#define PLUS 4	   // Show plus
#define SPACE 8	   // Space if plus
#define LEFT 16	   // Left justified
#define SPECIAL 32 // 0x
#define LARGE 64   // Use 'ABCDEF' instead of 'abcdef'
#define is_digit(c) ((c) >= '0' && (c) <= '9')
static const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char *upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static size_t _strnlen(const char *s, size_t count)
{
	const char *sc;
	for (sc = s; *sc != '\0' && count--; ++sc)
		;
	return sc - s;
}
static int skip_atoi(const char **s)
{
	int i = 0;
	while (is_digit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}
static void number(std_putc putc, long num, int base, int size, int precision, int type)
{
	char c, sign, tmp[66];
	const char *dig = digits;
	int i;
	char tmpch;

	if (type & LARGE)
		dig = upper_digits;
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return;

	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN)
	{
		if (num < 0)
		{
			sign = '-';
			num = -num;
			size--;
		}
		else if (type & PLUS)
		{
			sign = '+';
			size--;
		}
		else if (type & SPACE)
		{
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL)
	{
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else
	{
		while (num != 0)
		{
			tmp[i++] = dig[((unsigned long)num) % (unsigned)base];
			num = ((unsigned long)num) / (unsigned)base;
		}
	}
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type & (ZEROPAD | LEFT)))
	{
		while (size-- > 0)
		{
			tmpch = ' ';
			putc(&tmpch, 1);
		}
	}
	if (sign)
	{
		putc(&sign, 1);
	}

	if (type & SPECIAL)
	{
		if (base == 8)
		{
			tmpch = '0';
			putc(&tmpch, 1);
		}
		else if (base == 16)
		{
			tmpch = '0';
			putc(&tmpch, 1);
			tmpch = digits[33];
			putc(&tmpch, 1);
		}
	}
	if (!(type & LEFT))
	{
		while (size-- > 0)
		{
			putc(&c, 1);
		}
	}
	while (i < precision--)
	{
		tmpch = '0';
		putc(&tmpch, 1);
	}
	while (i-- > 0)
	{
		tmpch = tmp[i];
		putc(&tmpch, 1);
	}
	while (size-- > 0)
	{
		tmpch = ' ';
		putc(&tmpch, 1);
	}
}

static void vsprintf__(std_putc putc, const char *fmt, va_list args)
{
	int len;
	unsigned long num;
	int base;
	char *s;
	int flags;		 // Flags to number()
	int field_width; // Width of output field
	int precision;	 // Min. # of digits for integers; max number of chars for from string
	int qualifier;	 // 'h', 'l', or 'L' for integer fields
	char *tmpstr = NULL;
	int tmpstr_size = 0;
	char tmpch;
	for (; *fmt; fmt++)
	{

		if (*fmt != '%')
		{
			if (tmpstr == NULL)
			{
				tmpstr = (char *)fmt;
				tmpstr_size = 0;
			}
			tmpstr_size++;
			continue;
		}
		else if (tmpstr_size)
		{
			putc(tmpstr, tmpstr_size);
			tmpstr = NULL;
			tmpstr_size = 0;
		}

		// Process flags
		flags = 0;
	repeat:
		fmt++; // This also skips first '%'
		switch (*fmt)
		{
		case '-':
			flags |= LEFT;
			goto repeat;
		case '+':
			flags |= PLUS;
			goto repeat;
		case ' ':
			flags |= SPACE;
			goto repeat;
		case '#':
			flags |= SPECIAL;
			goto repeat;
		case '0':
			flags |= ZEROPAD;
			goto repeat;
		}

		// Get field width
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*')
		{
			fmt++;
			field_width = va_arg(args, int);
			if (field_width < 0)
			{
				field_width = -field_width;
				flags |= LEFT;
			}
		}
		// Get the precision
		precision = -1;
		if (*fmt == '.')
		{
			++fmt;
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*')
			{
				++fmt;
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}
		// Get the conversion qualifier
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
		{
			qualifier = *fmt;
			fmt++;
		}
		// Default base
		base = 10;
		switch (*fmt)
		{
		case 'c':
			if (!(flags & LEFT))
			{
				while (--field_width > 0)
				{
					tmpch = ' ';
					putc(&tmpch, 1);
				}
			}
			tmpch = (unsigned char)va_arg(args, int);
			putc(&tmpch, 1);

			while (--field_width > 0)
			{
				tmpch = ' ';
				putc(&tmpch, 1);
			}
			continue;
		case 's':
			s = va_arg(args, char *);
			if (!s)
				s = "<NULL>";
			len = _strnlen(s, precision);
			if (!(flags & LEFT))
			{
				while (len < field_width--)
				{
					tmpch = ' ';
					putc(&tmpch, 1);
				}
			}
			putc(s, len);
			while (len < field_width--)
			{
				tmpch = ' ';
				putc(&tmpch, 1);
			}
			continue;
		case 'p':
			if (field_width == -1)
			{
				field_width = 2 * sizeof(void *);
				flags |= ZEROPAD;
			}
			number(putc, (unsigned long)va_arg(args, void *), 16, field_width, precision, flags);
			continue;
		case 'n':
			continue;
		case 'A':
			continue;
			// Integer number formats - set up the flags and "break"
		case 'o':
			base = 8;
			break;
		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;
		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;
		default:
			if (*fmt != '%')
			{
				tmpch = '%';
				putc(&tmpch, 1);
			}
			if (*fmt)
			{
				tmpch = *fmt;
				putc(&tmpch, 1);
			}
			else
			{
				--fmt;
			}
			continue;
		}
		if (qualifier == 'l')
			num = va_arg(args, unsigned long);
		else if (qualifier == 'h')
		{
			if (flags & SIGN)
				num = va_arg(args, int);
			else
				num = va_arg(args, unsigned int);
		}
		else if (flags & SIGN)
			num = va_arg(args, int);
		else
			num = va_arg(args, unsigned int);
		number(putc, num, base, field_width, precision, flags);
	}
	if (tmpstr_size)
	{
		putc(tmpstr, tmpstr_size);
		tmpstr = NULL;
		tmpstr_size = 0;
	}
}

static void native_uart_putc(char *data, uint16_t size)
{
#ifdef OTSM_DEBUG_USART1
	UART1_Send_Buffer((uint8_t *)data, size);
#elif defined(OTSM_DEBUG_USART2)
	UART2_Send_Buffer((uint8_t *)data, size);
#elif defined(OTSM_DEBUG_USART4)
	UART4_Send_Buffer((uint8_t *)data, size);
#elif defined(PLATFORM_CST_OSAL_RTOS)
	HalUartSendBuf(UART0, (uint8_t *)data, size);
#else
#endif
}

#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Print a formatted log message.
 * @param format The format string.
 * @param ... The variable arguments.
 */
void dbg_log_printf(const char *format, ...)
{
	// Skip logging if log level is set to NONE or format is NULL
	if (current_log_level == LOG_LEVEL_NONE || !format)
	{
		return;
	}
	va_list args;
	va_start(args, format); // Initialize the va_list to process the variable arguments
#ifdef USE_MY_PRINTF
	vsprintf__(native_uart_putc, format, args); // Call formatted print function
#else
	vprintf(format, args);
#endif
	va_end(args); // Clean up the va_list after use
}

/**
 * @brief Print a log message with level, function name, file, and line number.
 * @param function_name The name of the function calling the log.
 * @param format The format string.
 * @param ... The variable arguments.
 */
void dbg_log_printf_level(const char *function_name, const char *format, ...)
{
	// Skip logging if log level is set to NONE or format is NULL
	if (current_log_level == LOG_LEVEL_NONE || !format)
	{
		return;
	}

	// Map log level to string representation
	const char *level_str;
	switch (current_log_level)
	{
	case LOG_LEVEL_DEBUG:
		level_str = "DEBUG";
		break;
	case LOG_LEVEL_INFO:
		level_str = "INFO";
		break;
	case LOG_LEVEL_WARNING:
		level_str = "WARNING";
		break;
	case LOG_LEVEL_ERROR:
		level_str = "ERROR";
		break;
	default:
		level_str = "";
		break;
	}

	// Print log header with timestamp, level, function name
#ifdef USE_MY_PRINTF
	// dbg_log_printf("[%s][%28s] ", level_str, function_name);
	dbg_log_printf("[%s][%*.*s] ", LOG_DEFAULT_MAX_WIDTH, LOG_DEFAULT_MAX_WIDTH, level_str, function_name);
#else
	// printf("[%s][%28s] ", level_str, function_name);
	printf("[%s][%*.*s] ", level_str, LOG_DEFAULT_MAX_WIDTH, LOG_DEFAULT_MAX_WIDTH, function_name);
#endif

	if (format == NULL || format[0] == '\0')
	{
		return;
	}

	va_list args;
	va_start(args, format);
// Print formatted log message
#ifdef USE_MY_PRINTF
	vsprintf__(native_uart_putc, format, args);
#else
	vprintf(format, args);
#endif
	va_end(args);
}

/**
 * @brief Print a buffer of bytes in hexadecimal format.
 * @param buff The buffer to print.
 * @param length The length of the buffer.
 */
void dbg_log_printf_buffer(const uint8_t *buff, uint16_t length)
{
	if (current_log_level == LOG_LEVEL_NONE)
	{
		return;
	}
	for (int i = 0; i < length; i++)
	{
#ifdef USE_MY_PRINTF
		dbg_log_printf("%02x ", buff[i]);
#else
		printf("%02x ", buff[i]);
#endif
	}
#ifdef USE_MY_PRINTF
	dbg_log_printf("\r\n");
#else
	printf("\r\n");
#endif
}

/**
 * @brief Print a buffer of bytes in hexadecimal format with log level and function name.
 * @param function_name The name of the function calling the log.
 * @param buff The buffer to print.
 * @param length The length of the buffer.
 */
void dbg_log_printf_buffer_level(const char *function_name, const uint8_t *buff, uint16_t length)
{
	if (buff == NULL || length == 0 || current_log_level == LOG_LEVEL_NONE)
	{
		return;
	}
	dbg_log_printf_level(function_name, "");
	for (int i = 0; i < length; i++)
	{
#ifdef USE_MY_PRINTF
		dbg_log_printf("%02x ", buff[i]);
#else
		printf("%02x ", buff[i]);
#endif
	}
#ifdef USE_MY_PRINTF
	dbg_log_printf("\r\n");
#else
	printf("\r\n");
#endif
}

/**
 * @brief Set the current log level.
 * @param level The log level to set (DEBUG, INFO, WARNING, ERROR, or NONE).
 */
void dbg_log_set_level(DBG_LOG_LEVEL level)
{
	current_log_level = level; // Update the log level
}

void dbg_log_set_channel(uint8_t channel)
{
	OTSM_DEBUG_UART_CHANNEL = channel; // Update the log level
}
