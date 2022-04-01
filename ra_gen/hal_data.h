/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_flash_hp.h"
#include "r_flash_api.h"
#include "r_spi.h"
#include "r_qspi.h"
#include "r_spi_flash_api.h"
#include "r_sci_uart.h"
#include "r_uart_api.h"
#include "r_can.h"
#include "r_can_api.h"
#include "r_dtc.h"
#include "r_transfer_api.h"
#include "r_gpt.h"
#include "r_timer_api.h"
FSP_HEADER
/* Flash on Flash HP Instance */
extern const flash_instance_t g_mcuflash;

/** Access the Flash HP instance using these structures when calling API functions directly (::p_api is not used). */
extern flash_hp_instance_ctrl_t g_mcuflash_ctrl;
extern const flash_cfg_t g_mcuflash_cfg;

#ifndef mcu_flash_callback
void mcu_flash_callback(flash_callback_args_t *p_args);
#endif
/** SPI on SPI Instance. */
extern const spi_instance_t g_cssdSpi;

/** Access the SPI instance using these structures when calling API functions directly (::p_api is not used). */
extern spi_instance_ctrl_t g_cssdSpi_ctrl;
extern const spi_cfg_t g_cssdSpi_cfg;

/** Callback used by SPI Instance. */
#ifndef cssd_spi_callback
void cssd_spi_callback(spi_callback_args_t *p_args);
#endif

#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
#define g_cssdSpi_P_TRANSFER_TX (NULL)
#else
    #define g_cssdSpi_P_TRANSFER_TX (&RA_NOT_DEFINED)
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
#define g_cssdSpi_P_TRANSFER_RX (NULL)
#else
    #define g_cssdSpi_P_TRANSFER_RX (&RA_NOT_DEFINED)
#endif
#undef RA_NOT_DEFINED
extern const spi_flash_instance_t g_qspi0;
extern qspi_instance_ctrl_t g_qspi0_ctrl;
extern const spi_flash_cfg_t g_qspi0_cfg;
/** UART on SCI Instance. */
extern const uart_instance_t g_uart1;

/** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
extern sci_uart_instance_ctrl_t g_uart1_ctrl;
extern const uart_cfg_t g_uart1_cfg;
extern const sci_uart_extended_cfg_t g_uart1_cfg_extend;

#ifndef uart_callback
void uart_callback(uart_callback_args_t *p_args);
#endif
/** UART on SCI Instance. */
extern const uart_instance_t g_uart0;

/** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
extern sci_uart_instance_ctrl_t g_uart0_ctrl;
extern const uart_cfg_t g_uart0_cfg;
extern const sci_uart_extended_cfg_t g_uart0_cfg_extend;

#ifndef uart_callback
void uart_callback(uart_callback_args_t *p_args);
#endif
/** CAN on CAN Instance. */
extern const can_instance_t g_can1;
/** Access the CAN instance using these structures when calling API functions directly (::p_api is not used). */
extern can_instance_ctrl_t g_can1_ctrl;
extern const can_cfg_t g_can1_cfg;
extern const can_extended_cfg_t g_can1_cfg_extend;

#ifndef can_callback
void can_callback(can_callback_args_t *p_args);
#endif
#define CAN_NO_OF_MAILBOXES_g_can1 (32)
/** CAN on CAN Instance. */
extern const can_instance_t g_can0;
/** Access the CAN instance using these structures when calling API functions directly (::p_api is not used). */
extern can_instance_ctrl_t g_can0_ctrl;
extern const can_cfg_t g_can0_cfg;
extern const can_extended_cfg_t g_can0_cfg_extend;

#ifndef can_callback
void can_callback(can_callback_args_t *p_args);
#endif
#define CAN_NO_OF_MAILBOXES_g_can0 (32)
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer1;

/** Access the DTC instance using these structures when calling API functions directly (::p_api is not used). */
extern dtc_instance_ctrl_t g_transfer1_ctrl;
extern const transfer_cfg_t g_transfer1_cfg;
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer0;

/** Access the DTC instance using these structures when calling API functions directly (::p_api is not used). */
extern dtc_instance_ctrl_t g_transfer0_ctrl;
extern const transfer_cfg_t g_transfer0_cfg;
/** SPI on SPI Instance. */
extern const spi_instance_t g_w5500spi;

/** Access the SPI instance using these structures when calling API functions directly (::p_api is not used). */
extern spi_instance_ctrl_t g_w5500spi_ctrl;
extern const spi_cfg_t g_w5500spi_cfg;

/** Callback used by SPI Instance. */
#ifndef w5500_spi_callback
void w5500_spi_callback(spi_callback_args_t *p_args);
#endif

#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == g_transfer0)
    #define g_w5500spi_P_TRANSFER_TX (NULL)
#else
#define g_w5500spi_P_TRANSFER_TX (&g_transfer0)
#endif
#if (RA_NOT_DEFINED == g_transfer1)
    #define g_w5500spi_P_TRANSFER_RX (NULL)
#else
#define g_w5500spi_P_TRANSFER_RX (&g_transfer1)
#endif
#undef RA_NOT_DEFINED
/** Timer on GPT Instance. */
extern const timer_instance_t g_mlosTaskElapsedTimer;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_mlosTaskElapsedTimer_ctrl;
extern const timer_cfg_t g_mlosTaskElapsedTimer_cfg;

#ifndef NULL
void NULL(timer_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_mlosClockTimer;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_mlosClockTimer_ctrl;
extern const timer_cfg_t g_mlosClockTimer_cfg;

#ifndef mlos_clock_timer_isr
void mlos_clock_timer_isr(timer_callback_args_t *p_args);
#endif
void hal_entry(void);
void g_hal_init(void);
FSP_FOOTER
#endif /* HAL_DATA_H_ */
