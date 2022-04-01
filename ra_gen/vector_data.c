/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = gpt_counter_overflow_isr, /* GPT0 COUNTER OVERFLOW (Overflow) */
            [1] = spi_rxi_isr, /* SPI0 RXI (Receive buffer full) */
            [2] = spi_txi_isr, /* SPI0 TXI (Transmit buffer empty) */
            [3] = spi_tei_isr, /* SPI0 TEI (Transmission complete event) */
            [4] = spi_eri_isr, /* SPI0 ERI (Error) */
            [5] = can_error_isr, /* CAN0 ERROR (Error interrupt) */
            [6] = can_mailbox_rx_isr, /* CAN0 MAILBOX RX (Reception complete interrupt) */
            [7] = can_mailbox_tx_isr, /* CAN0 MAILBOX TX (Transmission complete interrupt) */
            [8] = can_error_isr, /* CAN1 ERROR (Error interrupt) */
            [9] = can_mailbox_rx_isr, /* CAN1 MAILBOX RX (Reception complete interrupt) */
            [10] = can_mailbox_tx_isr, /* CAN1 MAILBOX TX (Transmission complete interrupt) */
            [11] = sci_uart_rxi_isr, /* SCI7 RXI (Received data full) */
            [12] = sci_uart_txi_isr, /* SCI7 TXI (Transmit data empty) */
            [13] = sci_uart_tei_isr, /* SCI7 TEI (Transmit end) */
            [14] = sci_uart_eri_isr, /* SCI7 ERI (Receive error) */
            [15] = sci_uart_rxi_isr, /* SCI1 RXI (Received data full) */
            [16] = sci_uart_txi_isr, /* SCI1 TXI (Transmit data empty) */
            [17] = sci_uart_tei_isr, /* SCI1 TEI (Transmit end) */
            [18] = sci_uart_eri_isr, /* SCI1 ERI (Receive error) */
            [19] = spi_rxi_isr, /* SPI1 RXI (Receive buffer full) */
            [20] = spi_txi_isr, /* SPI1 TXI (Transmit buffer empty) */
            [21] = spi_tei_isr, /* SPI1 TEI (Transmission complete event) */
            [22] = spi_eri_isr, /* SPI1 ERI (Error) */
        };
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_IELS_ENUM(EVENT_GPT0_COUNTER_OVERFLOW), /* GPT0 COUNTER OVERFLOW (Overflow) */
            [1] = BSP_PRV_IELS_ENUM(EVENT_SPI0_RXI), /* SPI0 RXI (Receive buffer full) */
            [2] = BSP_PRV_IELS_ENUM(EVENT_SPI0_TXI), /* SPI0 TXI (Transmit buffer empty) */
            [3] = BSP_PRV_IELS_ENUM(EVENT_SPI0_TEI), /* SPI0 TEI (Transmission complete event) */
            [4] = BSP_PRV_IELS_ENUM(EVENT_SPI0_ERI), /* SPI0 ERI (Error) */
            [5] = BSP_PRV_IELS_ENUM(EVENT_CAN0_ERROR), /* CAN0 ERROR (Error interrupt) */
            [6] = BSP_PRV_IELS_ENUM(EVENT_CAN0_MAILBOX_RX), /* CAN0 MAILBOX RX (Reception complete interrupt) */
            [7] = BSP_PRV_IELS_ENUM(EVENT_CAN0_MAILBOX_TX), /* CAN0 MAILBOX TX (Transmission complete interrupt) */
            [8] = BSP_PRV_IELS_ENUM(EVENT_CAN1_ERROR), /* CAN1 ERROR (Error interrupt) */
            [9] = BSP_PRV_IELS_ENUM(EVENT_CAN1_MAILBOX_RX), /* CAN1 MAILBOX RX (Reception complete interrupt) */
            [10] = BSP_PRV_IELS_ENUM(EVENT_CAN1_MAILBOX_TX), /* CAN1 MAILBOX TX (Transmission complete interrupt) */
            [11] = BSP_PRV_IELS_ENUM(EVENT_SCI7_RXI), /* SCI7 RXI (Received data full) */
            [12] = BSP_PRV_IELS_ENUM(EVENT_SCI7_TXI), /* SCI7 TXI (Transmit data empty) */
            [13] = BSP_PRV_IELS_ENUM(EVENT_SCI7_TEI), /* SCI7 TEI (Transmit end) */
            [14] = BSP_PRV_IELS_ENUM(EVENT_SCI7_ERI), /* SCI7 ERI (Receive error) */
            [15] = BSP_PRV_IELS_ENUM(EVENT_SCI1_RXI), /* SCI1 RXI (Received data full) */
            [16] = BSP_PRV_IELS_ENUM(EVENT_SCI1_TXI), /* SCI1 TXI (Transmit data empty) */
            [17] = BSP_PRV_IELS_ENUM(EVENT_SCI1_TEI), /* SCI1 TEI (Transmit end) */
            [18] = BSP_PRV_IELS_ENUM(EVENT_SCI1_ERI), /* SCI1 ERI (Receive error) */
            [19] = BSP_PRV_IELS_ENUM(EVENT_SPI1_RXI), /* SPI1 RXI (Receive buffer full) */
            [20] = BSP_PRV_IELS_ENUM(EVENT_SPI1_TXI), /* SPI1 TXI (Transmit buffer empty) */
            [21] = BSP_PRV_IELS_ENUM(EVENT_SPI1_TEI), /* SPI1 TEI (Transmission complete event) */
            [22] = BSP_PRV_IELS_ENUM(EVENT_SPI1_ERI), /* SPI1 ERI (Error) */
        };
        #endif
