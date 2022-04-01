/* generated vector header file - do not edit */
#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H
/* Number of interrupts allocated */
#ifndef VECTOR_DATA_IRQ_COUNT
#define VECTOR_DATA_IRQ_COUNT    (23)
#endif
/* ISR prototypes */
void gpt_counter_overflow_isr(void);
void spi_rxi_isr(void);
void spi_txi_isr(void);
void spi_tei_isr(void);
void spi_eri_isr(void);
void can_error_isr(void);
void can_mailbox_rx_isr(void);
void can_mailbox_tx_isr(void);
void sci_uart_rxi_isr(void);
void sci_uart_txi_isr(void);
void sci_uart_tei_isr(void);
void sci_uart_eri_isr(void);

/* Vector table allocations */
#define VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW ((IRQn_Type) 0) /* GPT0 COUNTER OVERFLOW (Overflow) */
#define VECTOR_NUMBER_SPI0_RXI ((IRQn_Type) 1) /* SPI0 RXI (Receive buffer full) */
#define VECTOR_NUMBER_SPI0_TXI ((IRQn_Type) 2) /* SPI0 TXI (Transmit buffer empty) */
#define VECTOR_NUMBER_SPI0_TEI ((IRQn_Type) 3) /* SPI0 TEI (Transmission complete event) */
#define VECTOR_NUMBER_SPI0_ERI ((IRQn_Type) 4) /* SPI0 ERI (Error) */
#define VECTOR_NUMBER_CAN0_ERROR ((IRQn_Type) 5) /* CAN0 ERROR (Error interrupt) */
#define VECTOR_NUMBER_CAN0_MAILBOX_RX ((IRQn_Type) 6) /* CAN0 MAILBOX RX (Reception complete interrupt) */
#define VECTOR_NUMBER_CAN0_MAILBOX_TX ((IRQn_Type) 7) /* CAN0 MAILBOX TX (Transmission complete interrupt) */
#define VECTOR_NUMBER_CAN1_ERROR ((IRQn_Type) 8) /* CAN1 ERROR (Error interrupt) */
#define VECTOR_NUMBER_CAN1_MAILBOX_RX ((IRQn_Type) 9) /* CAN1 MAILBOX RX (Reception complete interrupt) */
#define VECTOR_NUMBER_CAN1_MAILBOX_TX ((IRQn_Type) 10) /* CAN1 MAILBOX TX (Transmission complete interrupt) */
#define VECTOR_NUMBER_SCI7_RXI ((IRQn_Type) 11) /* SCI7 RXI (Received data full) */
#define VECTOR_NUMBER_SCI7_TXI ((IRQn_Type) 12) /* SCI7 TXI (Transmit data empty) */
#define VECTOR_NUMBER_SCI7_TEI ((IRQn_Type) 13) /* SCI7 TEI (Transmit end) */
#define VECTOR_NUMBER_SCI7_ERI ((IRQn_Type) 14) /* SCI7 ERI (Receive error) */
#define VECTOR_NUMBER_SCI1_RXI ((IRQn_Type) 15) /* SCI1 RXI (Received data full) */
#define VECTOR_NUMBER_SCI1_TXI ((IRQn_Type) 16) /* SCI1 TXI (Transmit data empty) */
#define VECTOR_NUMBER_SCI1_TEI ((IRQn_Type) 17) /* SCI1 TEI (Transmit end) */
#define VECTOR_NUMBER_SCI1_ERI ((IRQn_Type) 18) /* SCI1 ERI (Receive error) */
#define VECTOR_NUMBER_SPI1_RXI ((IRQn_Type) 19) /* SPI1 RXI (Receive buffer full) */
#define VECTOR_NUMBER_SPI1_TXI ((IRQn_Type) 20) /* SPI1 TXI (Transmit buffer empty) */
#define VECTOR_NUMBER_SPI1_TEI ((IRQn_Type) 21) /* SPI1 TEI (Transmission complete event) */
#define VECTOR_NUMBER_SPI1_ERI ((IRQn_Type) 22) /* SPI1 ERI (Error) */
typedef enum IRQn {
	Reset_IRQn = -15,
	NonMaskableInt_IRQn = -14,
	HardFault_IRQn = -13,
	MemoryManagement_IRQn = -12,
	BusFault_IRQn = -11,
	UsageFault_IRQn = -10,
	SecureFault_IRQn = -9,
	SVCall_IRQn = -5,
	DebugMonitor_IRQn = -4,
	PendSV_IRQn = -2,
	SysTick_IRQn = -1,
	GPT0_COUNTER_OVERFLOW_IRQn = 0, /* GPT0 COUNTER OVERFLOW (Overflow) */
	SPI0_RXI_IRQn = 1, /* SPI0 RXI (Receive buffer full) */
	SPI0_TXI_IRQn = 2, /* SPI0 TXI (Transmit buffer empty) */
	SPI0_TEI_IRQn = 3, /* SPI0 TEI (Transmission complete event) */
	SPI0_ERI_IRQn = 4, /* SPI0 ERI (Error) */
	CAN0_ERROR_IRQn = 5, /* CAN0 ERROR (Error interrupt) */
	CAN0_MAILBOX_RX_IRQn = 6, /* CAN0 MAILBOX RX (Reception complete interrupt) */
	CAN0_MAILBOX_TX_IRQn = 7, /* CAN0 MAILBOX TX (Transmission complete interrupt) */
	CAN1_ERROR_IRQn = 8, /* CAN1 ERROR (Error interrupt) */
	CAN1_MAILBOX_RX_IRQn = 9, /* CAN1 MAILBOX RX (Reception complete interrupt) */
	CAN1_MAILBOX_TX_IRQn = 10, /* CAN1 MAILBOX TX (Transmission complete interrupt) */
	SCI7_RXI_IRQn = 11, /* SCI7 RXI (Received data full) */
	SCI7_TXI_IRQn = 12, /* SCI7 TXI (Transmit data empty) */
	SCI7_TEI_IRQn = 13, /* SCI7 TEI (Transmit end) */
	SCI7_ERI_IRQn = 14, /* SCI7 ERI (Receive error) */
	SCI1_RXI_IRQn = 15, /* SCI1 RXI (Received data full) */
	SCI1_TXI_IRQn = 16, /* SCI1 TXI (Transmit data empty) */
	SCI1_TEI_IRQn = 17, /* SCI1 TEI (Transmit end) */
	SCI1_ERI_IRQn = 18, /* SCI1 ERI (Receive error) */
	SPI1_RXI_IRQn = 19, /* SPI1 RXI (Receive buffer full) */
	SPI1_TXI_IRQn = 20, /* SPI1 TXI (Transmit buffer empty) */
	SPI1_TEI_IRQn = 21, /* SPI1 TEI (Transmission complete event) */
	SPI1_ERI_IRQn = 22, /* SPI1 ERI (Error) */
} IRQn_Type;
#endif /* VECTOR_DATA_H */
