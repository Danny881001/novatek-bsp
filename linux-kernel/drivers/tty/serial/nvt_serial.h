//-------------------------------------------------------
//  UART registers
//-------------------------------------------------------

// 0x00, Receive Buffer Register (RBR), when DLAB=0 read
//       Transmit Holding Register (THR), when DLAB=0 write
//       Baud Rate Divisor Latch LSB, when DLAB=1 (DLL)
#define UART2_RBR_REG                    0x00            // Receive Data Port

#define UART2_THR_REG                    0x00            // Transmit Data Port

#define UART2_DLL_REG                    0x00
#define DLL_MASK                         GENMASK(7, 0)   // Baud Rate Divisor Latch Least Significant Byte

#define UART_DLR_MAX                     0xFFFF          // Divisor Latch Register Max Value
#define UART_DLR_MIN                     0x0001          // Divisor Latch Register Min Value

// 0x04, Interrupt Enable Register, when DLAB=0 (IER)
//       Baud Rate Divisor Latch MSB, when DLAB=1 (DLM)
#define UART2_IER_REG                    0x04
#define RDA_INTEN_BIT                    BIT(0)          // Received Data Available Interrupt enable
#define THR_EMPTY_INTEN_BIT              BIT(1)          // Transmitter Holding Register Empty Interrupt enable
#define RLS_INTEN_BIT                    BIT(2)          // Receiver Line Status Interrupt enable

#define UART2_DLM_REG                    0x04
#define DLM_MASK                         GENMASK(7, 0)   // Baud Rate Divisor Latch Most Significant Byte

// 0x08, Interrupt ID Register (IIR), when DLAB=0 read
//       FIFO Control Register (FCR), when DLAB=0 write
//       Prescaler Register (PSR), when DLAB=1
#define UART2_IIR_REG                    0x08
#define UART2_IIR_INT_ID_MASK            GENMASK(3, 0)   // Interrupt ID mask
#define _UART2_IIR_INT_NONE              0x1             // None
#define _UART2_IIR_INT_RLS               0x6             // Receiver Line Status
#define _UART2_IIR_INT_RDR               0x4             // Received Data Ready
#define _UART2_IIR_INT_CRT               0xC             // Character Reception Timeout
#define _UART2_IIR_INT_THRE              0x2             // Transmitter Holding Register Empty
#define FIFO_MODE_ENABLED_BIT            BIT(7)          // This bit is set when FCR[0] is set as 1

#define UART2_FCR_REG                    0x08
#define FIFO_EN_BIT                      BIT(0)          // FIFO enable (Do not change this bit when TX_DMA_EN or RX_DMA_EN bit is set)
#define RX_FIFO_RESET_BIT                BIT(1)          // RX FIFO RESET
#define TX_FIFO_RESET_BIT                BIT(2)          // TX FIFO RESET
#define RXFIFO_TRGL_MASK                 GENMASK(7, 6)   // Trigger level for the Rx FIFO interrupt (PIO Only)
#define _UART2_FCR_RX_TRIGGER_SHIFT      6
#define _UART2_FCR_RX_TRIGGER_LEVEL_1    0x0             // level=1
#define _UART2_FCR_RX_TRIGGER_LEVEL_8    0x1             // level=8
#define _UART2_FCR_RX_TRIGGER_LEVEL_16   0x2             // level=16
#define _UART2_FCR_RX_TRIGGER_LEVEL_28   0x3             // level=28

#define UART2_PSR_REG                    0x08
#define PSR_MASK                         GENMASK(4, 0)   // Prescaler Register

// 0x0C, Line Control Register (LCR)
#define UART2_LCR_REG                    0x0C
#define _WL_L5                           0x0             // Char Length 5
#define _WL_L6                           0x1             // Char Length 6
#define _WL_L7                           0x2             // Char Length 7
#define _WL_L8                           0x3             // Char Length 8
#define STOP_BIT                         BIT(2)          // Stop Bits
#define _UART2_LCR_PARITY_SHIFT          3               // LCR PARITY SHIFT BIT
#define _UART2_PARITY_NONE               0x0             // No parity checking
#define _UART2_PARITY_ODD                0x1             // Odd parity
#define _UART2_PARITY_EVEN               0x3             // Even parity
#define _UART2_PARITY_ONE                0x5             // One parity
#define _UART2_PARITY_ZERO               0x7             // Zero parity
#define SET_BREAK_BIT                    BIT(6)          // Causes a break condition to be transmitted (Only Tx)
#define DLAB_BIT                         BIT(7)          // Divisor Latch Access Bit (DLAB)

// 0x14, Line Status Register (LSR), when read
//       Testing Register (TST), when write
#define UART2_LSR_REG                    0x14
#define DATA_READY_BIT                   BIT(0)          // data ready
#define OVERRUN_ERR_BIT                  BIT(1)          // overrun error (PIO Only)
#define PARITY_ERR_BIT                   BIT(2)          // parity error
#define FRAMING_ERR_BIT                  BIT(3)          // framing error
#define BREAK_INT_BIT                    BIT(4)          // break interrupt
#define THR_EMPTY_BIT                    BIT(5)          // THR empty
#define TRANSMITTER_EMPTY_BIT            BIT(6)          // transmitter empty
#define BOTH_EMPTY_BIT                   (THR_EMPTY_BIT | TRANSMITTER_EMPTY_BIT)
#define FIFO_DATA_ERR_BIT                BIT(7)          // FIFO data error

#define UART2_TST_REG                    0x14
#define TEST_PAR_ERR_BIT                 BIT(0)          // generates incorrect parity during UART transmission
#define TEST_FRM_ERR_BIT                 BIT(1)          // generates a logic 0 STOP bit during UART transmission
#define TEST_BAUDGEN_BIT                 BIT(2)          // Just to improve baud rate generator toggle rate

// 0x1C, Scratch Pad Register (SPR)
#define UART2_SPR_REG                    0x1C
#define USER_DATA_MASK                   GENMASK(7, 0)   // used by programmer to hold data temporarily

//-------------------------------------------------------
//  UART2 registers
//-------------------------------------------------------

// 0x10, MODEN Control Register
#define UART2_MCR_REG                    0x10
#define LOOP_BIT                         BIT(4)
#define HW_FLOW_CTRL_BIT                 BIT(6)          // Hardware Flow Control

// 0x80, Tx DMA Starting Address Register
#define UART2_TX_DMA_ADDR_REG            0x80
#define TX_DMA_ADDRESS_MASK              GENMASK(30, 0)

// 0x84, Tx DMA Starting SIZE Register
#define UART2_TX_DMA_SIZE_REG            0x84
#define TX_DMA_SIZE_MASK                 GENMASK(19, 0)

// 0x88, Tx DMA Current Address Register
#define UART2_TX_CUR_ADDR_REG            0x88
#define TX_CUR_ADDRESS_MASK              GENMASK(30, 0)

// 0x8C, Tx DMA Control Register
#define UART2_TX_DMA_CTRL_REG            0x8C
#define TX_DMA_EN_BIT                    BIT(0)

// 0x90, Rx DMA Starting Address Register
#define UART2_RX_DMA_ADDR_REG            0x90
#define RX_DMA_ADDRESS_MASK              GENMASK(30, 0)

// 0x94, Rx DMA Starting SIZE Register
#define UART2_RX_DMA_SIZE_REG            0x94
#define RX_DMA_SIZE_MASK                 GENMASK(19, 0)

// 0x98, Rx DMA Current Address Register
#define UART2_RX_CUR_ADDR_REG            0x98
#define RX_CUR_ADDRESS_MASK              GENMASK(30, 0)

// 0x9C, Rx DMA Control Register
#define UART2_RX_DMA_CTRL_REG            0x9C
#define RX_DMA_EN_BIT                    BIT(0)

// 0xA0, DMA Interrupt Control Register
#define UART2_DMA_INT_CTRL_REG           0xA0
#define TX_DMA_DONE_INTEN_BIT            BIT(0)
#define RX_DMA_DONE_INTEN_BIT            BIT(8)
#define RX_DMA_ERR_INTEN_BIT             BIT(9)

// 0xA4, DMA Interrupt Status Register
#define UART2_DMA_INT_STS_REG            0xA4
#define TX_DMA_DONE_BIT                  BIT(0)
#define RX_DMA_DONE_BIT                  BIT(8)
#define RX_DMA_ERR_BIT                   BIT(9)
#define DMA_OVERRUN_ERR_BIT              BIT(16)         // DMA Only
#define DMA_PARITY_ERR_BIT               BIT(17)         // DMA Only
#define DMA_FRAMING_ERR_BIT              BIT(18)         // DMA Only
#define DMA_BREAK_ERR_BIT                BIT(19)         // DMA Only

static inline void nvt_write(struct uart_port *port, unsigned int off, unsigned int val)
{
	writel_relaxed(val, port->membase + off);
}

static inline unsigned int nvt_read(struct uart_port *port, unsigned int off)
{
	return readl_relaxed(port->membase + off);
}

static inline void nvt_masked_write(struct uart_port *port, unsigned int off, unsigned int mask, unsigned int val)
{
	unsigned int tmp = nvt_read(port, off);  //need to be careful that some registers are cleared after reading, such as RBR

	tmp &= ~mask;
	tmp |= val & mask;
	writel_relaxed(tmp, port->membase + off);
}
