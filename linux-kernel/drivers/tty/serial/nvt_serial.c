#if defined(CONFIG_SERIAL_NVT_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/atomic.h>
#include <linux/hrtimer.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>

#include "nvt_serial.h"

#define UART_TO_NVT(uart_port)	((struct nvt_port *) uart_port)

struct nvt_port {
	struct uart_port     uart;
	char                 name[16];
	struct clk           *clk;
	unsigned int         baud;
	unsigned int         tx_loadsz;  /* transmit fifo load size */
	/*
	 * Some bits in registers are cleared on a read, so they must
	 * be saved whenever the register is read, but the bits will not
	 * be immediately processed.
	 */
	unsigned int         lsr_break_flag;
};

static void nvt_stop_rx(struct uart_port *port)
{
	nvt_masked_write(port, UART2_IER_REG, RLS_INTEN_BIT | RDA_INTEN_BIT, 0);
}

static void nvt_stop_tx(struct uart_port *port)
{
	nvt_masked_write(port, UART2_IER_REG, THR_EMPTY_INTEN_BIT, 0);
}

static void nvt_start_tx(struct uart_port *port)
{
	/*
	 * We can't control the number of start_tx in one startup cycle,
	 * which is determined by the number of SyS_write calls.
	 *
	 * If handle_tx exhausted the xmit buffer, it needs to be returned
	 * when xmit buffer is empty to prevent start_tx from hanging without
	 * data.
	 */
	if (uart_circ_empty(&port->state->xmit) || uart_tx_stopped(port)) {
		pr_debug("nvt_serial: start_tx without THR data, so return\n");
		return;
	}

	nvt_masked_write(port, UART2_IER_REG, THR_EMPTY_INTEN_BIT, THR_EMPTY_INTEN_BIT);
}

static unsigned int nvt_tx_empty(struct uart_port *port)
{
	return (nvt_read(port, UART2_LSR_REG) & TRANSMITTER_EMPTY_BIT) ? TIOCSER_TEMT : 0;
}

static void nvt_handle_rx(struct uart_port *port, unsigned int lsr_r)
{
	struct nvt_port *nvt_port = UART_TO_NVT(port);
	struct tty_port *tport = &port->state->port;
	unsigned char ch;
	char flag;

	do {
		if (likely(lsr_r & DATA_READY_BIT))
			ch = nvt_read(port, UART2_RBR_REG);
		else
			ch = 0;

		pr_debug("nvt_serial: LSR=0x%x, IER=0x%x, read ch=0x%x from RBR\n", lsr_r, nvt_read(port, UART2_IER_REG), ch);

		flag = TTY_NORMAL;
		port->icount.rx++;

		lsr_r |= nvt_port->lsr_break_flag;
		nvt_port->lsr_break_flag = 0;

		if (lsr_r & BREAK_INT_BIT) {
			port->icount.brk++;
			if (uart_handle_break(port))
				continue;
		} else if (lsr_r & PARITY_ERR_BIT)
			port->icount.parity++;
		else if (lsr_r & FRAMING_ERR_BIT)
			port->icount.frame++;

		if (lsr_r & OVERRUN_ERR_BIT)
			port->icount.overrun++;

		/*
		 * Mask off conditions which should be ignored.
		 */
		lsr_r &= port->read_status_mask;

		if (lsr_r & BREAK_INT_BIT)
			flag = TTY_BREAK;
		else if (lsr_r & PARITY_ERR_BIT)
			flag = TTY_PARITY;
		else if (lsr_r & FRAMING_ERR_BIT)
			flag = TTY_FRAME;

		if (!(uart_handle_sysrq_char(port, ch)))
			uart_insert_char(port, lsr_r, OVERRUN_ERR_BIT, ch, flag);
	} while ((lsr_r = nvt_read(port, UART2_LSR_REG)) & DATA_READY_BIT);

	/*
	 * Drop the lock here since it might end up calling
	 * uart_start(), which takes the lock.
	 */
	spin_unlock(&port->lock);
	tty_flip_buffer_push(tport);
	spin_lock(&port->lock);
}

static void nvt_handle_tx(struct uart_port *port)
{
	struct nvt_port *nvt_port = UART_TO_NVT(port);
	struct circ_buf *xmit = &port->state->xmit;
	unsigned char ch;
	int count;

	/* For software flow control, xon resume transmission, xoff pause transmission */
	if (port->x_char) {
		nvt_write(port, UART2_THR_REG, port->x_char);
		port->icount.tx++;
		port->x_char = 0;
		return;
	}

	/* If there isn't anything more to transmit, or the uart is now stopped, disable the uart and exit */
	if (uart_circ_empty(xmit) || uart_tx_stopped(port)) {
		nvt_stop_tx(port);
		return;
	}

	/* Drain the buffer by size of tx_loadsz in one cycle */
	count = nvt_port->tx_loadsz;
	do {
		ch = xmit->buf[xmit->tail];
		nvt_write(port, UART2_THR_REG, ch);
		port->icount.tx++;
		pr_debug("nvt_serial: head=%d, tail=%d, cnt_tx=%d, write ch=0x%x to THR\n", xmit->head, xmit->tail, port->icount.tx, ch);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		if (uart_circ_empty(xmit))
			break;
	} while (--count > 0);

	/*
	 * If num chars in xmit buffer are too few, ask tty layer for more.
	 * By Hard ISR to schedule processing in software interrupt part.
	 */
	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	if (uart_circ_empty(xmit))
		nvt_stop_tx(port);
}

static irqreturn_t nvt_irq(int irq, void *dev_id)
{
	struct uart_port *port = dev_id;
	unsigned int lsr_r, ier_r, iir_r;
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);

	lsr_r = nvt_read(port, UART2_LSR_REG);  /* error bits are cleared after reading */
	ier_r = nvt_read(port, UART2_IER_REG);
	iir_r = nvt_read(port, UART2_IIR_REG);  /* clear interrupt status to avoid irq storm */

	pr_debug("nvt_serial: irq stage LSR=0x%x, IER=0x%x, IIR=0x%x\n", lsr_r, ier_r, iir_r);

	/* Handle Uart Rx */
	if ((lsr_r & DATA_READY_BIT) && (ier_r & RDA_INTEN_BIT))
		nvt_handle_rx(port, lsr_r);

	/* Handle Uart Tx */
	if ((lsr_r & THR_EMPTY_BIT) && (ier_r & THR_EMPTY_INTEN_BIT))
		nvt_handle_tx(port);

	/* Print Uart Error */
	if (lsr_r & FIFO_DATA_ERR_BIT) {
		if (lsr_r & OVERRUN_ERR_BIT)
			pr_err("nvt_serial: UART P-OVR Err!\r\n");
		if (lsr_r & PARITY_ERR_BIT)
			pr_err("nvt_serial: UART P-PARITY Err!\r\n");
		if (lsr_r & FRAMING_ERR_BIT)
			pr_err("nvt_serial: UART P-FRM Err!\r\n");
		if (lsr_r & BREAK_INT_BIT)
			pr_err("nvt_serial: UART P-BREAK Err!\r\n");
	}

	/* Clear RBR when IIR stuck in CRT */
	if (((iir_r & UART2_IIR_INT_ID_MASK) == _UART2_IIR_INT_CRT) && ((lsr_r & DATA_READY_BIT) == 0))
		nvt_read(port, UART2_RBR_REG);

	spin_unlock_irqrestore(&port->lock, flags);

	return IRQ_HANDLED;
}

static unsigned int nvt_get_mctrl(struct uart_port *port)
{
	/*
	 * Pretend we have a Modem status reg and following bits are
	 *  always set, to satify the serial core state machine
	 *  (DSR) Data Set Ready
	 *  (CTS) Clear To Send
	 *  (CAR) Carrier Detect
	 */
	return TIOCM_DSR | TIOCM_CTS | TIOCM_CAR;
}

static void nvt_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	/* MCR not present */
}

static void nvt_break_ctl(struct uart_port *port, int break_ctl)
{
	if (break_ctl)
		nvt_masked_write(port, UART2_LCR_REG, SET_BREAK_BIT, SET_BREAK_BIT);
	else
		nvt_masked_write(port, UART2_LCR_REG, SET_BREAK_BIT, 0);
}

static int nvt_startup(struct uart_port *port)
{
	struct nvt_port *nvt_port = UART_TO_NVT(port);
	int ret;

	snprintf(nvt_port->name, sizeof(nvt_port->name),
		 "nvt_serial%d", port->line);

	ret = request_irq(port->irq, nvt_irq, IRQF_TRIGGER_HIGH,
			  nvt_port->name, port);
	if (unlikely(ret))
		return ret;

	return 0;
}

static void nvt_shutdown(struct uart_port *port)
{
	nvt_write(port, UART2_IER_REG, 0);  /* disable interrupts to avoid hanging in start_tx */
	free_irq(port->irq, port);
}

static void nvt_set_baud_rate(struct uart_port *port, unsigned int baud)
{
	unsigned int PSR, DLR;

	PSR = 0x01;
	DLR = port->uartclk / 16 / PSR / baud;

	if (DLR > UART_DLR_MAX) {
		DLR = UART_DLR_MAX;
		pr_err("nvt_serial: baud out of MAX range Err!\n");
	}
	if (DLR < UART_DLR_MIN) {
		DLR = UART_DLR_MIN;
		pr_err("nvt_serial: baud out of MIN range Err!\n");
	}

	nvt_write(port, UART2_DLL_REG, DLR & DLL_MASK);
	nvt_masked_write(port, UART2_DLM_REG, DLM_MASK, DLR >> 8);
	nvt_write(port, UART2_PSR_REG, PSR & PSR_MASK);
}

static void nvt_set_termios(struct uart_port *port, struct ktermios *termios,
			    struct ktermios *old)
{
	struct nvt_port *nvt_port = UART_TO_NVT(port);
	unsigned long flags;
	unsigned int lcr_val, baud;

	spin_lock_irqsave(&port->lock, flags);

	/* Set baud rate and divisor */
	if (nvt_port->baud) {
		if (tty_termios_baud_rate(termios))
			tty_termios_encode_baud_rate(termios, nvt_port->baud, nvt_port->baud);
		nvt_port->baud = 0;
	}
	baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk / 16);

	nvt_masked_write(port, UART2_LCR_REG, DLAB_BIT, DLAB_BIT);  /* set DLAB bit to access divisor */
	nvt_set_baud_rate(port, baud);
	if (tty_termios_baud_rate(termios))
		tty_termios_encode_baud_rate(termios, baud, baud);

	/* Set length */
	switch (termios->c_cflag & CSIZE) {
	case CS5:
		lcr_val |= _WL_L5;
		break;
	case CS6:
		lcr_val |= _WL_L6;
		break;
	case CS7:
		lcr_val |= _WL_L7;
		break;
	case CS8:
	default:
		lcr_val |= _WL_L8;
		break;
	}

	/* Set stop bits */
	if (termios->c_cflag & CSTOPB)
		lcr_val |= STOP_BIT;

	/* Set parity */
	if (termios->c_cflag & PARENB) {
		if (termios->c_cflag & CMSPAR) {
			if (termios->c_cflag & PARODD)
				lcr_val |= _UART2_PARITY_ONE << _UART2_LCR_PARITY_SHIFT;
			else
				lcr_val |= _UART2_PARITY_ZERO << _UART2_LCR_PARITY_SHIFT;
		} else {
			if (termios->c_cflag & PARODD)
				lcr_val |= _UART2_PARITY_ODD << _UART2_LCR_PARITY_SHIFT;
			else
				lcr_val |= _UART2_PARITY_EVEN << _UART2_LCR_PARITY_SHIFT;
		}
	}
	nvt_write(port, UART2_LCR_REG, lcr_val);  /* clear DLAB bit */

	/* Support hardware flow control */
	if (port->hw_flowctrl == 0)
		nvt_masked_write(port, UART2_MCR_REG, HW_FLOW_CTRL_BIT, 0);
	else
		nvt_masked_write(port, UART2_MCR_REG, HW_FLOW_CTRL_BIT, HW_FLOW_CTRL_BIT);

	/* Enanle receive interrupts */
	nvt_write(port, UART2_IER_REG, RLS_INTEN_BIT | RDA_INTEN_BIT);

	/* Configure status bits to ignore based on termio flags */
	port->read_status_mask = OVERRUN_ERR_BIT;
	if (termios->c_iflag & INPCK)
		port->read_status_mask |= FRAMING_ERR_BIT | PARITY_ERR_BIT;
	if (termios->c_iflag & (IGNBRK | BRKINT | PARMRK))
		port->read_status_mask |= BREAK_INT_BIT;

	uart_update_timeout(port, termios->c_cflag, baud);

	spin_unlock_irqrestore(&port->lock, flags);
}

static const char *nvt_type(struct uart_port *port)
{
	return "NVT";
}

static void nvt_release_port(struct uart_port *port)
{
}

static int nvt_request_port(struct uart_port *port)
{
	return 0;
}

static void nvt_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE)
		port->type = PORT_16550A;
}

static int nvt_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	if (unlikely(ser->type != PORT_UNKNOWN && ser->type != PORT_16550A))
		return -EINVAL;
	if (unlikely(port->irq != ser->irq))
		return -EINVAL;
	return 0;
}

static inline void wait_for_xmitr(struct uart_port *port, int bits)
{
	struct nvt_port *nvt_port = UART_TO_NVT(port);
	unsigned int status, tmout = 10000;

	/* Wait up to 10ms for the character(s) to be sent */
	for (;;) {
		status = nvt_read(port, UART2_LSR_REG);

		if (status & BREAK_INT_BIT)
			nvt_port->lsr_break_flag = BREAK_INT_BIT;

		if ((status & bits) == bits)
			break;
		if (--tmout == 0)
			break;
		udelay(1);
	}
}

#ifdef CONFIG_CONSOLE_POLL
static int nvt_poll_get_char(struct uart_port *port)
{
	unsigned char ch;

	while (!(nvt_read(port, UART2_LSR_REG) & DATA_READY_BIT))
		cpu_relax();

	ch = nvt_read(port, UART2_RBR_REG);

	return ch;
}

static void nvt_poll_put_char(struct uart_port *port, unsigned char ch)
{
	unsigned int ier;

	/* First save the IER then disable the interrupts */
	ier = nvt_read(port, UART2_IER_REG);
	nvt_write(port, UART2_IER_REG, 0);
	wait_for_xmitr(port, BOTH_EMPTY_BIT);

	/* Send the character out */
	nvt_write(port, UART2_THR_REG, ch);

	/* Finally, wait for transmitter to become empty and restore the IER */
	wait_for_xmitr(port, BOTH_EMPTY_BIT);
	nvt_write(port, UART2_IER_REG, ier);
}
#endif

static struct uart_ops nvt_uart_pops = {
	.stop_rx = nvt_stop_rx,
	.stop_tx = nvt_stop_tx,
	.start_tx = nvt_start_tx,
	.tx_empty = nvt_tx_empty,
	.get_mctrl = nvt_get_mctrl,
	.set_mctrl = nvt_set_mctrl,
	.break_ctl = nvt_break_ctl,
	.startup = nvt_startup,
	.shutdown = nvt_shutdown,
	.set_termios = nvt_set_termios,
	.type = nvt_type,
	.release_port = nvt_release_port,
	.request_port = nvt_request_port,
	.config_port = nvt_config_port,
	.verify_port = nvt_verify_port,
#ifdef CONFIG_CONSOLE_POLL
	.poll_get_char	= nvt_poll_get_char,
	.poll_put_char	= nvt_poll_put_char,
#endif
};

static struct nvt_port nvt_uart_ports[] = {
#ifdef CONFIG_SERIAL_NVT_CONSOLE
	{
		.uart = {
			.line = 0,
			.ops = &nvt_uart_pops,
			.flags = UPF_BOOT_AUTOCONF,
		/* Following items are the default values and will be covered by dts including baud and tx_loadsz */
			.uartclk = 24000000,
			.iotype = UPIO_MEM32,
			.fifosize = 16,
			.regshift = 2,
			.hw_flowctrl = 0,
		},
		.baud = 0,
		.tx_loadsz = 16,
	},
#endif
	{
		.uart = {
			.line = 1,
			.ops = &nvt_uart_pops,
			.flags = UPF_BOOT_AUTOCONF,
		/* Following items are the default values and will be covered by dts including baud and tx_loadsz */
			.uartclk = 48000000,
			.iotype = UPIO_MEM32,
			.fifosize = 32,
			.regshift = 2,
			.hw_flowctrl = 0,
		},
		.baud = 0,  /* set into termios structure when first used, and can be modified through user interface such as stty */
		.tx_loadsz = 32,  /* maximum number of characters tx can send in one handle_tx cycle (follow fifosize) */
	},
	{
		.uart = {
			.line = 2,
			.ops = &nvt_uart_pops,
			.flags = UPF_BOOT_AUTOCONF,
		/* Following items are the default values and will be covered by dts including baud and tx_loadsz */
			.uartclk = 48000000,
			.iotype = UPIO_MEM32,
			.fifosize = 32,
			.regshift = 2,
			.hw_flowctrl = 0,
		},
		.baud = 0,
		.tx_loadsz = 32,
	},
	{
		.uart = {
			.line = 3,
			.ops = &nvt_uart_pops,
			.flags = UPF_BOOT_AUTOCONF,
		/* Following items are the default values and will be covered by dts including baud and tx_loadsz */
			.uartclk = 48000000,
			.iotype = UPIO_MEM32,
			.fifosize = 32,
			.regshift = 2,
			.hw_flowctrl = 0,
		},
		.baud = 0,
		.tx_loadsz = 32,
	},
};

#define UART_NR	4

static inline struct uart_port *get_port_from_line(unsigned int line)
{
#ifdef CONFIG_SERIAL_NVT_CONSOLE
	return &nvt_uart_ports[line].uart;
#else
	return &nvt_uart_ports[line-1].uart;
#endif
}

#ifdef CONFIG_SERIAL_NVT_CONSOLE
static void nvt_console_putchar(struct uart_port *port, int ch)
{
	wait_for_xmitr(port, THR_EMPTY_BIT);
	nvt_write(port, UART2_THR_REG, ch);
}

static void nvt_console_write(struct console *co, const char *s, unsigned int count)
{
	struct uart_port *port = get_port_from_line(co->index);
	unsigned long flags;
	unsigned int ier;
	int locked = 1;

	if (port->sysrq)
		locked = 0;
	else if (oops_in_progress)
		locked = spin_trylock_irqsave(&port->lock, flags);
	else
		spin_lock_irqsave(&port->lock, flags);

	/* First save the IER then disable the interrupts */
	ier = nvt_read(port, UART2_IER_REG);
	nvt_write(port, UART2_IER_REG, 0);

	uart_console_write(port, s, count, nvt_console_putchar);

	/* Finally, wait for transmitter to become empty and restore the IER */
	wait_for_xmitr(port, BOTH_EMPTY_BIT);
	nvt_write(port, UART2_IER_REG, ier);

	if (locked)
		spin_unlock_irqrestore(&port->lock, flags);
}

static int __init nvt_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	if (unlikely(co->index >= UART_NR || co->index < 0))
		return -ENXIO;

	/*
	* The uart port backing the console (e.g. ttyNVT0) might not have been
	* init yet. If so, defer the console setup to after the port.
	*/
	port = get_port_from_line(co->index);
	if (unlikely(!port->membase))
		return -ENXIO;

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	pr_info("nvt_serial: console setup on port #%d\n", port->line);

	/*
	* Serial core will call port->ops->set_termios( )
	* which will set the baud reg.
	*/
	return uart_set_options(port, co, baud, parity, bits, flow);
}

static void nvt_serial_early_write(struct console *con, const char *s, unsigned n)
{
	struct earlycon_device *dev = con->data;

	uart_console_write(&dev->port, s, n, nvt_console_putchar);
}

static int __init nvt_serial_early_console_setup(struct earlycon_device *device, const char *opt)
{
	if (!device->port.membase)
		return -ENODEV;

	device->con->write = nvt_serial_early_write;
	return 0;
}
EARLYCON_DECLARE(nvt_serial, nvt_serial_early_console_setup);
OF_EARLYCON_DECLARE(nvt_serial, "nvt.nvt_uart", nvt_serial_early_console_setup);

static struct uart_driver nvt_uart_driver;

static struct console nvt_console = {
	.name = "ttyNVT",
	.write = nvt_console_write,
	.device = uart_console_device,
	.setup = nvt_console_setup,
	.flags = CON_PRINTBUFFER,
	.index = -1,
	.data = &nvt_uart_driver,
};

#define NVT_CONSOLE	(&nvt_console)
#else
#define NVT_CONSOLE	NULL
#endif

static struct uart_driver nvt_uart_driver = {
	.owner = THIS_MODULE,
	.driver_name = "nvt_serial",
	.dev_name = "ttyNVT",
	.nr = UART_NR,
	.cons = NVT_CONSOLE,
};

static int nvt_serial_probe(struct platform_device *pdev)
{
	struct nvt_port *nvt_port;
	struct uart_port *port;
	u32 prop;
	int irq, line;
	struct resource *resource;

	if (of_property_read_u32(pdev->dev.of_node, "uart_id", &prop)) {
		dev_err(&pdev->dev, "get uart_id failed\n");
		return -EINVAL;
	}
	line = prop;
	port = get_port_from_line(line);
	port->dev = &pdev->dev;
	nvt_port = UART_TO_NVT(port);

	nvt_port->clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	if (IS_ERR(nvt_port->clk))
		dev_warn(&pdev->dev, "%s not found, ignored\n", dev_name(&pdev->dev));

	resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(!resource)) {
		dev_err(&pdev->dev, "get address failed\n");
		return -ENXIO;
	}
	port->mapbase = resource->start;

	port->membase = devm_ioremap(port->dev, resource->start, resource_size(resource));
	if (!port->membase) {
		dev_err(&pdev->dev, "ioremap failed\n");
		return -EBUSY;
	}

	irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
	if (unlikely(irq < 0)) {
		dev_err(&pdev->dev, "get irq failed\n");
		return -ENXIO;
	}
	port->irq = irq;

	if (of_property_read_u32(pdev->dev.of_node, "clock-frequency", &prop) == 0)
		port->uartclk = prop;

	if (of_property_read_u32(pdev->dev.of_node, "reg-io-width", &prop) == 0) {
		switch (prop) {
		case 1:
			port->iotype = UPIO_MEM;
			break;
		case 4:
			port->iotype = of_device_is_big_endian(pdev->dev.of_node) ?
				       UPIO_MEM32BE : UPIO_MEM32;
			break;
		default:
			dev_warn(&pdev->dev, "unsupported reg-io-width (%d)\n", prop);
		}
	}

	if (of_property_read_u32(pdev->dev.of_node, "fifo-size", &prop) == 0) {
		port->fifosize = prop;
		nvt_port->tx_loadsz = prop;
	}

	if (of_property_read_u32(pdev->dev.of_node, "reg-shift", &prop) == 0)
		port->regshift = prop;

	if (of_property_read_u32(pdev->dev.of_node, "hw_flowctrl", &prop) == 0)
		port->hw_flowctrl = prop;

	if (of_find_property(pdev->dev.of_node, "no-loopback-test", NULL))
		port->flags |= UPF_SKIP_TEST;

	if (of_property_read_u32(pdev->dev.of_node, "baud", &prop) == 0)
		nvt_port->baud = prop;

	if (of_property_read_u32(pdev->dev.of_node, "fifo-trig-level", &prop) == 0)
		nvt_write(port, UART2_FCR_REG, FIFO_EN_BIT | (prop << _UART2_FCR_RX_TRIGGER_SHIFT));

	pr_info("nvt_serial: driver probed\n");

	platform_set_drvdata(pdev, port);

	return uart_add_one_port(&nvt_uart_driver, port);
}

static int nvt_serial_remove(struct platform_device *pdev)
{
	struct uart_port *port = platform_get_drvdata(pdev);

	uart_remove_one_port(&nvt_uart_driver, port);

	return 0;
}

static const struct of_device_id nvt_match_table[] = {
    { .compatible = "nvt.nvt_uart" },
    {},
};

static struct platform_driver nvt_serial_platform_driver = {
	.probe		= nvt_serial_probe,
	.remove		= nvt_serial_remove,
	.driver		= {
		.name	= "nvt_serial",
		.of_match_table = nvt_match_table,
	},
};

static int __init nvt_serial_init(void)
{
	int ret;

	ret = uart_register_driver(&nvt_uart_driver);
	if (unlikely(ret))
		return ret;

	ret = platform_driver_register(&nvt_serial_platform_driver);
	if (unlikely(ret)) {
		uart_unregister_driver(&nvt_uart_driver);
		return ret;
	}

	pr_info("nvt_serial: driver initialized\n");

	return ret;
}

static void __exit nvt_serial_exit(void)
{
	platform_driver_unregister(&nvt_serial_platform_driver);
	uart_unregister_driver(&nvt_uart_driver);
}

module_init(nvt_serial_init);
module_exit(nvt_serial_exit);

MODULE_AUTHOR("Shawn Chou <shawn_chou@novatek.com>");
MODULE_DESCRIPTION("Driver for nvt serial device");
MODULE_LICENSE("GPL");
