/* generated HAL source file - do not edit */
#include "hal_data.h"

flash_hp_instance_ctrl_t g_mcuflash_ctrl;
const flash_cfg_t g_mcuflash_cfg = { .data_flash_bgo = false, .p_callback =
		mcu_flash_callback, .p_context = NULL,
#if defined(VECTOR_NUMBER_FCU_FRDYI)
    .irq                 = VECTOR_NUMBER_FCU_FRDYI,
#else
		.irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_FCU_FIFERR)
    .err_irq             = VECTOR_NUMBER_FCU_FIFERR,
#else
		.err_irq = FSP_INVALID_VECTOR,
#endif
		.err_ipl = (BSP_IRQ_DISABLED), .ipl = (BSP_IRQ_DISABLED), };
/* Instance structure to use this module. */
const flash_instance_t g_mcuflash = { .p_ctrl = &g_mcuflash_ctrl, .p_cfg =
		&g_mcuflash_cfg, .p_api = &g_flash_on_flash_hp };
spi_instance_ctrl_t g_cssdSpi_ctrl;

/** SPI extended configuration for SPI HAL driver */
const spi_extended_cfg_t g_cssdSpi_ext_cfg = { .spi_clksyn =
		SPI_SSL_MODE_CLK_SYN, .spi_comm = SPI_COMMUNICATION_FULL_DUPLEX,
		.ssl_polarity = SPI_SSLP_LOW, .ssl_select = SPI_SSL_SELECT_SSL0,
		.mosi_idle = SPI_MOSI_IDLE_VALUE_FIXING_DISABLE, .parity =
				SPI_PARITY_MODE_DISABLE, .byte_swap = SPI_BYTE_SWAP_DISABLE,
		.spck_div = {
		/* Actual calculated bitrate: 200000. */.spbr = 249, .brdv = 0 },
		.spck_delay = SPI_DELAY_COUNT_1,
		.ssl_negation_delay = SPI_DELAY_COUNT_1, .next_access_delay =
				SPI_DELAY_COUNT_1 };

/** SPI configuration for SPI HAL driver */
const spi_cfg_t g_cssdSpi_cfg = { .channel = 1,

#if defined(VECTOR_NUMBER_SPI1_RXI)
    .rxi_irq             = VECTOR_NUMBER_SPI1_RXI,
#else
		.rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SPI1_TXI)
    .txi_irq             = VECTOR_NUMBER_SPI1_TXI,
#else
		.txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SPI1_TEI)
    .tei_irq             = VECTOR_NUMBER_SPI1_TEI,
#else
		.tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SPI1_ERI)
    .eri_irq             = VECTOR_NUMBER_SPI1_ERI,
#else
		.eri_irq = FSP_INVALID_VECTOR,
#endif

		.rxi_ipl = (12), .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),

		.operating_mode = SPI_MODE_MASTER,

		.clk_phase = SPI_CLK_PHASE_EDGE_EVEN, .clk_polarity =
				SPI_CLK_POLARITY_HIGH,

		.mode_fault = SPI_MODE_FAULT_ERROR_DISABLE, .bit_order =
				SPI_BIT_ORDER_MSB_FIRST, .p_transfer_tx =
				g_cssdSpi_P_TRANSFER_TX, .p_transfer_rx =
				g_cssdSpi_P_TRANSFER_RX, .p_callback = cssd_spi_callback,

		.p_context = NULL, .p_extend = (void*) &g_cssdSpi_ext_cfg, };

/* Instance structure to use this module. */
const spi_instance_t g_cssdSpi = { .p_ctrl = &g_cssdSpi_ctrl, .p_cfg =
		&g_cssdSpi_cfg, .p_api = &g_spi_on_spi };
qspi_instance_ctrl_t g_qspi0_ctrl;

static const spi_flash_erase_command_t g_qspi0_erase_command_list[] = {
#if 2048 > 0
		{ .command = 0x20, .size = 2048 },
#endif
#if 32768 > 0
		{ .command = 0x52, .size = 32768 },
#endif
#if 65536 > 0
		{ .command = 0xD8, .size = 65536 },
#endif
#if 0xC7 > 0
		{ .command = 0xC7, .size = SPI_FLASH_ERASE_SIZE_CHIP_ERASE },
#endif
		};
static const qspi_extended_cfg_t g_qspi0_extended_cfg = {
		.min_qssl_deselect_cycles = QSPI_QSSL_MIN_HIGH_LEVEL_8_QSPCLK,
		.qspclk_div = QSPI_QSPCLK_DIV_2, };
const spi_flash_cfg_t g_qspi0_cfg = { .spi_protocol =
		SPI_FLASH_PROTOCOL_EXTENDED_SPI, .read_mode =
		SPI_FLASH_READ_MODE_FAST_READ_QUAD_OUTPUT, .address_bytes =
		SPI_FLASH_ADDRESS_BYTES_3, .dummy_clocks =
		SPI_FLASH_DUMMY_CLOCKS_DEFAULT, .page_program_address_lines =
		SPI_FLASH_DATA_LINES_1, .page_size_bytes = 2048, .page_program_command =
		0x02, .write_enable_command = 0x06, .status_command = 0x05,
		.write_status_bit = 0, .xip_enter_command = 0x20, .xip_exit_command =
				0xFF, .p_erase_command_list = &g_qspi0_erase_command_list[0],
		.erase_command_list_length = sizeof(g_qspi0_erase_command_list)
				/ sizeof(g_qspi0_erase_command_list[0]), .p_extend =
				&g_qspi0_extended_cfg, };
/** This structure encompasses everything that is needed to use an instance of this interface. */
const spi_flash_instance_t g_qspi0 = { .p_ctrl = &g_qspi0_ctrl, .p_cfg =
		&g_qspi0_cfg, .p_api = &g_qspi_on_spi_flash, };
sci_uart_instance_ctrl_t g_uart1_ctrl;

baud_setting_t g_uart1_baud_setting = {
/* Baud rate calculated with 0.469% error. */.abcse = 0, .abcs = 0, .bgdm = 1,
		.cks = 0, .brr = 53, .mddr = (uint8_t) 256, .brme = false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_uart_extended_cfg_t g_uart1_cfg_extend = {
		.clock = SCI_UART_CLOCK_INT, .rx_edge_start =
				SCI_UART_START_BIT_FALLING_EDGE, .noise_cancel =
				SCI_UART_NOISE_CANCELLATION_DISABLE, .rx_fifo_trigger =
				SCI_UART_RX_FIFO_TRIGGER_MAX, .p_baud_setting =
				&g_uart1_baud_setting,
		.flow_control = SCI_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_0xFF_PIN_0xFF,
                #else
		.flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
		};

/** UART interface configuration */
const uart_cfg_t g_uart1_cfg = { .channel = 1, .data_bits = UART_DATA_BITS_8,
		.parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
				uart_callback, .p_context = NULL, .p_extend =
				&g_uart1_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
		.p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
		.p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
		.rxi_ipl = (12), .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),
#if defined(VECTOR_NUMBER_SCI1_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI1_RXI,
#else
		.rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI1_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI1_TXI,
#else
		.txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI1_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI1_TEI,
#else
		.tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI1_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI1_ERI,
#else
		.eri_irq = FSP_INVALID_VECTOR,
#endif
		};

/* Instance structure to use this module. */
const uart_instance_t g_uart1 = { .p_ctrl = &g_uart1_ctrl,
		.p_cfg = &g_uart1_cfg, .p_api = &g_uart_on_sci };
sci_uart_instance_ctrl_t g_uart0_ctrl;

baud_setting_t g_uart0_baud_setting = {
/* Baud rate calculated with 0.469% error. */.abcse = 0, .abcs = 0, .bgdm = 1,
		.cks = 0, .brr = 53, .mddr = (uint8_t) 256, .brme = false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_uart_extended_cfg_t g_uart0_cfg_extend = {
		.clock = SCI_UART_CLOCK_INT, .rx_edge_start =
				SCI_UART_START_BIT_FALLING_EDGE, .noise_cancel =
				SCI_UART_NOISE_CANCELLATION_DISABLE, .rx_fifo_trigger =
				SCI_UART_RX_FIFO_TRIGGER_MAX, .p_baud_setting =
				&g_uart0_baud_setting,
		.flow_control = SCI_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_0xFF_PIN_0xFF,
                #else
		.flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
		};

/** UART interface configuration */
const uart_cfg_t g_uart0_cfg = { .channel = 7, .data_bits = UART_DATA_BITS_8,
		.parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
				uart_callback, .p_context = NULL, .p_extend =
				&g_uart0_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
		.p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
		.p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
		.rxi_ipl = (12), .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),
#if defined(VECTOR_NUMBER_SCI7_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI7_RXI,
#else
		.rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI7_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI7_TXI,
#else
		.txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI7_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI7_TEI,
#else
		.tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI7_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI7_ERI,
#else
		.eri_irq = FSP_INVALID_VECTOR,
#endif
		};

/* Instance structure to use this module. */
const uart_instance_t g_uart0 = { .p_ctrl = &g_uart0_ctrl,
		.p_cfg = &g_uart0_cfg, .p_api = &g_uart_on_sci };
#ifndef CAN1_BAUD_SETTINGS_OVERRIDE
#define CAN1_BAUD_SETTINGS_OVERRIDE  (0)
#endif
#if CAN1_BAUD_SETTINGS_OVERRIDE
can_bit_timing_cfg_t g_can1_bit_timing_cfg =
{
    .baud_rate_prescaler = 1,
    .time_segment_1 = 4,
    .time_segment_2 = 2,
    .synchronization_jump_width = 1
};
#else
can_bit_timing_cfg_t g_can1_bit_timing_cfg =
		{
				/* Actual bitrate: 500000 Hz. Actual Bit Time Ratio: 75 %. */.baud_rate_prescaler =
						1 + 2 /* Division value of baud rate prescaler */,
				.time_segment_1 = 11, .time_segment_2 = 4,
				.synchronization_jump_width = 4, };
#endif

uint32_t g_can1_mailbox_mask[CAN_NO_OF_MAILBOXES_g_can1 / 4] = { 0,
#if CAN_NO_OF_MAILBOXES_g_can1 > 4
0,
#endif
#if CAN_NO_OF_MAILBOXES_g_can1 > 8
0,
0,
#endif
#if CAN_NO_OF_MAILBOXES_g_can1 > 16
0,
0,
0,
0,
#endif
		};

can_mailbox_t g_can1_mailbox[CAN_NO_OF_MAILBOXES_g_can1] = { { .mailbox_id = 0,
		.id_mode = CAN_ID_MODE_EXTENDED, .mailbox_type = CAN_MAILBOX_TRANSMIT,
		.frame_type = CAN_FRAME_TYPE_DATA }, { .mailbox_id = 1, .id_mode =
		CAN_ID_MODE_EXTENDED, .mailbox_type = CAN_MAILBOX_RECEIVE, .frame_type =
		CAN_FRAME_TYPE_DATA }, { .mailbox_id = 2, .id_mode =
		CAN_ID_MODE_EXTENDED, .mailbox_type = CAN_MAILBOX_RECEIVE, .frame_type =
		CAN_FRAME_TYPE_DATA, }, { .mailbox_id = 3, .id_mode =
		CAN_ID_MODE_EXTENDED, .mailbox_type = CAN_MAILBOX_RECEIVE, .frame_type =
		CAN_FRAME_TYPE_DATA },
#if CAN_NO_OF_MAILBOXES_g_can1 > 4
    {
        .mailbox_id              =  4,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  5,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  6,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  7,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
#endif
#if CAN_NO_OF_MAILBOXES_g_can1 > 8
    {
        .mailbox_id              =  8,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  9,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  10,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  11,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  12,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA,
    },
    {
        .mailbox_id              =  13,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  14,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  15,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
#endif
#if CAN_NO_OF_MAILBOXES_g_can1 > 16
    {
        .mailbox_id              =  16,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },

    {
        .mailbox_id              =  17,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  18,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  19,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  20,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  21,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  22,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA,
    },
    {
        .mailbox_id              =  23,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  24,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  25,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  26,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  27,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  28,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  29,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  30,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  31,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    }
#endif
		};

const can_extended_cfg_t g_can1_extended_cfg = { .clock_source =
		CAN_CLOCK_SOURCE_CANMCLK, .p_mailbox_mask = g_can1_mailbox_mask,
		.p_mailbox = g_can1_mailbox, .global_id_mode =
				CAN_GLOBAL_ID_MODE_EXTENDED, .mailbox_count =
				CAN_NO_OF_MAILBOXES_g_can1, .message_mode =
				CAN_MESSAGE_MODE_OVERWRITE, };

can_instance_ctrl_t g_can1_ctrl;
const can_cfg_t g_can1_cfg = { .channel = 1, .p_bit_timing =
		&g_can1_bit_timing_cfg, .p_callback = can_callback, .p_extend =
		&g_can1_extended_cfg, .p_context = NULL, .ipl = (2),
#if defined(VECTOR_NUMBER_CAN1_MAILBOX_TX)
    .tx_irq             = VECTOR_NUMBER_CAN1_MAILBOX_TX,
#else
		.tx_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_CAN1_MAILBOX_RX)
    .rx_irq             = VECTOR_NUMBER_CAN1_MAILBOX_RX,
#else
		.rx_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_CAN1_ERROR)
    .error_irq             = VECTOR_NUMBER_CAN1_ERROR,
#else
		.error_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const can_instance_t g_can1 = { .p_ctrl = &g_can1_ctrl, .p_cfg = &g_can1_cfg,
		.p_api = &g_can_on_can };
#ifndef CAN0_BAUD_SETTINGS_OVERRIDE
#define CAN0_BAUD_SETTINGS_OVERRIDE  (0)
#endif
#if CAN0_BAUD_SETTINGS_OVERRIDE
can_bit_timing_cfg_t g_can0_bit_timing_cfg =
{
    .baud_rate_prescaler = 1,
    .time_segment_1 = 4,
    .time_segment_2 = 2,
    .synchronization_jump_width = 1
};
#else
can_bit_timing_cfg_t g_can0_bit_timing_cfg =
		{
				/* Actual bitrate: 500000 Hz. Actual Bit Time Ratio: 75 %. */.baud_rate_prescaler =
						1 + 2 /* Division value of baud rate prescaler */,
				.time_segment_1 = 11, .time_segment_2 = 4,
				.synchronization_jump_width = 4, };
#endif

uint32_t g_can0_mailbox_mask[CAN_NO_OF_MAILBOXES_g_can0 / 4] = { 0x00,
#if CAN_NO_OF_MAILBOXES_g_can0 > 4
0,
#endif
#if CAN_NO_OF_MAILBOXES_g_can0 > 8
0,
0,
#endif
#if CAN_NO_OF_MAILBOXES_g_can0 > 16
0,
0,
0,
0,
#endif
		};

can_mailbox_t g_can0_mailbox[CAN_NO_OF_MAILBOXES_g_can0] = { { .mailbox_id = 0,
		.id_mode = CAN_ID_MODE_EXTENDED, .mailbox_type = CAN_MAILBOX_TRANSMIT,
		.frame_type = CAN_FRAME_TYPE_DATA }, { .mailbox_id = 1, .id_mode =
		CAN_ID_MODE_EXTENDED, .mailbox_type = CAN_MAILBOX_RECEIVE, .frame_type =
		CAN_FRAME_TYPE_DATA }, { .mailbox_id = 2, .id_mode =
		CAN_ID_MODE_EXTENDED, .mailbox_type = CAN_MAILBOX_RECEIVE, .frame_type =
		CAN_FRAME_TYPE_DATA, }, { .mailbox_id = 3, .id_mode =
		CAN_ID_MODE_EXTENDED, .mailbox_type = CAN_MAILBOX_RECEIVE, .frame_type =
		CAN_FRAME_TYPE_DATA },
#if CAN_NO_OF_MAILBOXES_g_can0 > 4
    {
        .mailbox_id              =  4,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  5,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  6,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  7,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
#endif
#if CAN_NO_OF_MAILBOXES_g_can0 > 8
    {
        .mailbox_id              =  8,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  9,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  10,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  11,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  12,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA,
    },
    {
        .mailbox_id              =  13,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  14,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  15,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
#endif
#if CAN_NO_OF_MAILBOXES_g_can0 > 16
    {
        .mailbox_id              =  16,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },

    {
        .mailbox_id              =  17,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  18,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  19,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  20,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  21,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  22,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA,
    },
    {
        .mailbox_id              =  23,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  24,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  25,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  26,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  27,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  28,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  29,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  30,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  31,
        .id_mode                 =  CAN_ID_MODE_EXTENDED,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    }
#endif
		};

const can_extended_cfg_t g_can0_extended_cfg = { .clock_source =
		CAN_CLOCK_SOURCE_CANMCLK, .p_mailbox_mask = g_can0_mailbox_mask,
		.p_mailbox = g_can0_mailbox, .global_id_mode =
				CAN_GLOBAL_ID_MODE_EXTENDED, .mailbox_count =
				CAN_NO_OF_MAILBOXES_g_can0, .message_mode =
				CAN_MESSAGE_MODE_OVERWRITE, };

can_instance_ctrl_t g_can0_ctrl;
const can_cfg_t g_can0_cfg = { .channel = 0, .p_bit_timing =
		&g_can0_bit_timing_cfg, .p_callback = can_callback, .p_extend =
		&g_can0_extended_cfg, .p_context = NULL, .ipl = (2),
#if defined(VECTOR_NUMBER_CAN0_MAILBOX_TX)
    .tx_irq             = VECTOR_NUMBER_CAN0_MAILBOX_TX,
#else
		.tx_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_CAN0_MAILBOX_RX)
    .rx_irq             = VECTOR_NUMBER_CAN0_MAILBOX_RX,
#else
		.rx_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_CAN0_ERROR)
    .error_irq             = VECTOR_NUMBER_CAN0_ERROR,
#else
		.error_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const can_instance_t g_can0 = { .p_ctrl = &g_can0_ctrl, .p_cfg = &g_can0_cfg,
		.p_api = &g_can_on_can };
dtc_instance_ctrl_t g_transfer1_ctrl;

transfer_info_t g_transfer1_info = { .dest_addr_mode =
		TRANSFER_ADDR_MODE_INCREMENTED, .repeat_area =
		TRANSFER_REPEAT_AREA_DESTINATION, .irq = TRANSFER_IRQ_END, .chain_mode =
		TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
		.size = TRANSFER_SIZE_1_BYTE, .mode = TRANSFER_MODE_NORMAL, .p_dest =
				(void*) NULL, .p_src = (void const*) NULL, .num_blocks = 0,
		.length = 0, };
const dtc_extended_cfg_t g_transfer1_cfg_extend = { .activation_source =
		VECTOR_NUMBER_SPI0_RXI, };
const transfer_cfg_t g_transfer1_cfg = { .p_info = &g_transfer1_info,
		.p_extend = &g_transfer1_cfg_extend, };

/* Instance structure to use this module. */
const transfer_instance_t g_transfer1 = { .p_ctrl = &g_transfer1_ctrl, .p_cfg =
		&g_transfer1_cfg, .p_api = &g_transfer_on_dtc };
dtc_instance_ctrl_t g_transfer0_ctrl;

transfer_info_t g_transfer0_info = { .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
		.repeat_area = TRANSFER_REPEAT_AREA_SOURCE, .irq = TRANSFER_IRQ_END,
		.chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode =
				TRANSFER_ADDR_MODE_INCREMENTED, .size = TRANSFER_SIZE_1_BYTE,
		.mode = TRANSFER_MODE_NORMAL, .p_dest = (void*) NULL, .p_src =
				(void const*) NULL, .num_blocks = 0, .length = 0, };
const dtc_extended_cfg_t g_transfer0_cfg_extend = { .activation_source =
		VECTOR_NUMBER_SPI0_TXI, };
const transfer_cfg_t g_transfer0_cfg = { .p_info = &g_transfer0_info,
		.p_extend = &g_transfer0_cfg_extend, };

/* Instance structure to use this module. */
const transfer_instance_t g_transfer0 = { .p_ctrl = &g_transfer0_ctrl, .p_cfg =
		&g_transfer0_cfg, .p_api = &g_transfer_on_dtc };
spi_instance_ctrl_t g_w5500spi_ctrl;

/** SPI extended configuration for SPI HAL driver */
const spi_extended_cfg_t g_w5500spi_ext_cfg = { .spi_clksyn =
		SPI_SSL_MODE_CLK_SYN, .spi_comm = SPI_COMMUNICATION_FULL_DUPLEX,
		.ssl_polarity = SPI_SSLP_LOW, .ssl_select = SPI_SSL_SELECT_SSL0,
		.mosi_idle = SPI_MOSI_IDLE_VALUE_FIXING_DISABLE, .parity =
				SPI_PARITY_MODE_DISABLE, .byte_swap = SPI_BYTE_SWAP_DISABLE,
		.spck_div = {
		/* Actual calculated bitrate: 16666667. */.spbr = 2, .brdv = 0 },
		.spck_delay = SPI_DELAY_COUNT_1,
		.ssl_negation_delay = SPI_DELAY_COUNT_1, .next_access_delay =
				SPI_DELAY_COUNT_1 };

/** SPI configuration for SPI HAL driver */
const spi_cfg_t g_w5500spi_cfg = { .channel = 0,

#if defined(VECTOR_NUMBER_SPI0_RXI)
    .rxi_irq             = VECTOR_NUMBER_SPI0_RXI,
#else
		.rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SPI0_TXI)
    .txi_irq             = VECTOR_NUMBER_SPI0_TXI,
#else
		.txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SPI0_TEI)
    .tei_irq             = VECTOR_NUMBER_SPI0_TEI,
#else
		.tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SPI0_ERI)
    .eri_irq             = VECTOR_NUMBER_SPI0_ERI,
#else
		.eri_irq = FSP_INVALID_VECTOR,
#endif

		.rxi_ipl = (12), .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),

		.operating_mode = SPI_MODE_MASTER,

		.clk_phase = SPI_CLK_PHASE_EDGE_ODD, .clk_polarity =
				SPI_CLK_POLARITY_LOW,

		.mode_fault = SPI_MODE_FAULT_ERROR_DISABLE, .bit_order =
				SPI_BIT_ORDER_MSB_FIRST, .p_transfer_tx =
				g_w5500spi_P_TRANSFER_TX, .p_transfer_rx =
				g_w5500spi_P_TRANSFER_RX, .p_callback = w5500_spi_callback,

		.p_context = NULL, .p_extend = (void*) &g_w5500spi_ext_cfg, };

/* Instance structure to use this module. */
const spi_instance_t g_w5500spi = { .p_ctrl = &g_w5500spi_ctrl, .p_cfg =
		&g_w5500spi_cfg, .p_api = &g_spi_on_spi };
gpt_instance_ctrl_t g_mlosTaskElapsedTimer_ctrl;
#if 0
const gpt_extended_pwm_cfg_t g_mlosTaskElapsedTimer_pwm_extend =
{
    .trough_ipl          = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT1_COUNTER_UNDERFLOW)
    .trough_irq          = VECTOR_NUMBER_GPT1_COUNTER_UNDERFLOW,
#else
    .trough_irq          = FSP_INVALID_VECTOR,
#endif
    .poeg_link           = GPT_POEG_LINK_POEG0,
    .output_disable      =  GPT_OUTPUT_DISABLE_NONE,
    .adc_trigger         =  GPT_ADC_TRIGGER_NONE,
    .dead_time_count_up  = 0,
    .dead_time_count_down = 0,
    .adc_a_compare_match = 0,
    .adc_b_compare_match = 0,
    .interrupt_skip_source = GPT_INTERRUPT_SKIP_SOURCE_NONE,
    .interrupt_skip_count  = GPT_INTERRUPT_SKIP_COUNT_0,
    .interrupt_skip_adc    = GPT_INTERRUPT_SKIP_ADC_NONE,
    .gtioca_disable_setting = GPT_GTIOC_DISABLE_PROHIBITED,
    .gtiocb_disable_setting = GPT_GTIOC_DISABLE_PROHIBITED,
};
#endif
const gpt_extended_cfg_t g_mlosTaskElapsedTimer_extend = { .gtioca = {
		.output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW }, .gtiocb = {
		.output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW },
		.start_source = (gpt_source_t)(GPT_SOURCE_NONE), .stop_source =
				(gpt_source_t)(GPT_SOURCE_NONE), .clear_source = (gpt_source_t)(
				GPT_SOURCE_NONE), .count_up_source = (gpt_source_t)(
				GPT_SOURCE_NONE), .count_down_source = (gpt_source_t)(
				GPT_SOURCE_NONE), .capture_a_source = (gpt_source_t)(
				GPT_SOURCE_NONE), .capture_b_source = (gpt_source_t)(
				GPT_SOURCE_NONE), .capture_a_ipl = (BSP_IRQ_DISABLED),
		.capture_b_ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT1_CAPTURE_COMPARE_A)
    .capture_a_irq       = VECTOR_NUMBER_GPT1_CAPTURE_COMPARE_A,
#else
		.capture_a_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_GPT1_CAPTURE_COMPARE_B)
    .capture_b_irq       = VECTOR_NUMBER_GPT1_CAPTURE_COMPARE_B,
#else
		.capture_b_irq = FSP_INVALID_VECTOR,
#endif
		.capture_filter_gtioca = GPT_CAPTURE_FILTER_NONE,
		.capture_filter_gtiocb = GPT_CAPTURE_FILTER_NONE,
#if 0
    .p_pwm_cfg                   = &g_mlosTaskElapsedTimer_pwm_extend,
#else
		.p_pwm_cfg = NULL,
#endif
		};
const timer_cfg_t g_mlosTaskElapsedTimer_cfg = { .mode = TIMER_MODE_PERIODIC,
/* Actual period: 0.065535 seconds. Actual duty: 50%. */.period_counts =
		(uint32_t) 0x63ff9c, .duty_cycle_counts = 0x31ffce, .source_div =
		(timer_source_div_t) 0, .channel = 1, .p_callback = NULL, .p_context =
		NULL, .p_extend = &g_mlosTaskElapsedTimer_extend, .cycle_end_ipl =
		(BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT1_COUNTER_OVERFLOW)
    .cycle_end_irq       = VECTOR_NUMBER_GPT1_COUNTER_OVERFLOW,
#else
		.cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const timer_instance_t g_mlosTaskElapsedTimer = { .p_ctrl =
		&g_mlosTaskElapsedTimer_ctrl, .p_cfg = &g_mlosTaskElapsedTimer_cfg,
		.p_api = &g_timer_on_gpt };
gpt_instance_ctrl_t g_mlosClockTimer_ctrl;
#if 0
const gpt_extended_pwm_cfg_t g_mlosClockTimer_pwm_extend =
{
    .trough_ipl          = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT0_COUNTER_UNDERFLOW)
    .trough_irq          = VECTOR_NUMBER_GPT0_COUNTER_UNDERFLOW,
#else
    .trough_irq          = FSP_INVALID_VECTOR,
#endif
    .poeg_link           = GPT_POEG_LINK_POEG0,
    .output_disable      =  GPT_OUTPUT_DISABLE_NONE,
    .adc_trigger         =  GPT_ADC_TRIGGER_NONE,
    .dead_time_count_up  = 0,
    .dead_time_count_down = 0,
    .adc_a_compare_match = 0,
    .adc_b_compare_match = 0,
    .interrupt_skip_source = GPT_INTERRUPT_SKIP_SOURCE_NONE,
    .interrupt_skip_count  = GPT_INTERRUPT_SKIP_COUNT_0,
    .interrupt_skip_adc    = GPT_INTERRUPT_SKIP_ADC_NONE,
    .gtioca_disable_setting = GPT_GTIOC_DISABLE_PROHIBITED,
    .gtiocb_disable_setting = GPT_GTIOC_DISABLE_PROHIBITED,
};
#endif
const gpt_extended_cfg_t g_mlosClockTimer_extend = { .gtioca = {
		.output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW }, .gtiocb = {
		.output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW },
		.start_source = (gpt_source_t)(GPT_SOURCE_NONE), .stop_source =
				(gpt_source_t)(GPT_SOURCE_NONE), .clear_source = (gpt_source_t)(
				GPT_SOURCE_NONE), .count_up_source = (gpt_source_t)(
				GPT_SOURCE_NONE), .count_down_source = (gpt_source_t)(
				GPT_SOURCE_NONE), .capture_a_source = (gpt_source_t)(
				GPT_SOURCE_NONE), .capture_b_source = (gpt_source_t)(
				GPT_SOURCE_NONE), .capture_a_ipl = (BSP_IRQ_DISABLED),
		.capture_b_ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT0_CAPTURE_COMPARE_A)
    .capture_a_irq       = VECTOR_NUMBER_GPT0_CAPTURE_COMPARE_A,
#else
		.capture_a_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_GPT0_CAPTURE_COMPARE_B)
    .capture_b_irq       = VECTOR_NUMBER_GPT0_CAPTURE_COMPARE_B,
#else
		.capture_b_irq = FSP_INVALID_VECTOR,
#endif
		.capture_filter_gtioca = GPT_CAPTURE_FILTER_NONE,
		.capture_filter_gtiocb = GPT_CAPTURE_FILTER_NONE,
#if 0
    .p_pwm_cfg                   = &g_mlosClockTimer_pwm_extend,
#else
		.p_pwm_cfg = NULL,
#endif
		};
const timer_cfg_t g_mlosClockTimer_cfg = { .mode = TIMER_MODE_PERIODIC,
/* Actual period: 0.001 seconds. Actual duty: 50%. */.period_counts =
		(uint32_t) 0x186a0, .duty_cycle_counts = 0xc350, .source_div =
		(timer_source_div_t) 0, .channel = 0,
		.p_callback = mlos_clock_timer_isr, .p_context = NULL, .p_extend =
				&g_mlosClockTimer_extend, .cycle_end_ipl = (0),
#if defined(VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW)
    .cycle_end_irq       = VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW,
#else
		.cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const timer_instance_t g_mlosClockTimer = { .p_ctrl = &g_mlosClockTimer_ctrl,
		.p_cfg = &g_mlosClockTimer_cfg, .p_api = &g_timer_on_gpt };
void g_hal_init(void) {
	g_common_init();
}
