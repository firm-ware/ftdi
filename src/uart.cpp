#include "firm/ftdi/uart.hpp"
#include "firm/timing.hpp"

namespace firm {
namespace ftdi {

int FTDI::open(uint16_t vid, uint16_t pid) {
#ifdef LIBFTDI
    return ftdi_usb_open(&this->ctx, vid, pid);
#else
    FT_STATUS status = FT_SetVIDPID(vid, pid);
    if (status != FT_OK) {
        return -1;
    }

    status = FT_Open(0, &this->ctx);
    if (status != FT_OK) {
        return -1;
    }

    status = FT_ResetDevice(this->ctx);
    if (status != FT_OK) {
        return -1;
    }
#endif
    return 0;
}

int FTDI::configure(firm::uart::Config_t config) {
#ifdef LIBFTDI
    // Remember the configuration under this->props because we will need it later
    // when calling ftdi_set_line_property2 to set and clear the break condition
    switch (config.dataBits) {
        case firm::uart::DATA_BITS_7: this->props.bits = BITS_7; break;
        case firm::uart::DATA_BITS_8: this->props.bits = BITS_8; break;
        default: return -1;
    }
    switch (config.stopBits) {
        case firm::uart::STOP_BITS_1: this->props.stopbits = STOP_BIT_1; break;
        case firm::uart::STOP_BITS_2: this->props.stopbits = STOP_BIT_2; break;
    }
    switch (config.parity) {
        case firm::uart::PARITY_NONE: this->props.parity = NONE; break;
        case firm::uart::PARITY_ODD: this->props.parity = ODD; break;
        case firm::uart::PARITY_EVEN: this->props.parity = EVEN; break;
    }

    int err = ftdi_set_line_property(&this->ctx, this->props.bits, this->props.stopbits, this->props.parity);
    if (err < 0) {
        this->log->error("[FTDI] Failed to set line property: %s", ftdi_get_error_string(&this->ctx));
        return err;
    }

    ftdi_read_data_set_chunksize(&this->ctx, this->rxBufferSize);
    ftdi_set_latency_timer(&this->ctx, 1);
    ftdi_setflowctrl(&this->ctx, SIO_DISABLE_FLOW_CTRL);
    ftdi_set_bitmode(&this->ctx, 0xFF, BITMODE_RESET);
    ftdi_tcioflush(&this->ctx);
#else
    UCHAR dataBits;
    switch (config.dataBits) {
        case firm::uart::DATA_BITS_7: dataBits = FT_BITS_7; break;
        case firm::uart::DATA_BITS_8: dataBits = FT_BITS_8; break;
        default: return -1;
    }

    UCHAR stopBits;
    switch (config.stopBits) {
        case firm::uart::STOP_BITS_1: stopBits = FT_STOP_BITS_1; break;
        case firm::uart::STOP_BITS_2: stopBits = FT_STOP_BITS_2; break;
        default: return -1;
    }

    UCHAR parity;
    switch (config.parity) {
        case firm::uart::PARITY_NONE: parity = FT_PARITY_NONE; break;
        case firm::uart::PARITY_ODD: parity = FT_PARITY_ODD; break;
        case firm::uart::PARITY_EVEN: parity = FT_PARITY_EVEN; break;
        default: return -1;
    }

    FT_STATUS status = FT_SetDataCharacteristics(this->ctx, dataBits, stopBits, parity);
    if (status != FT_OK) {
        return -1;
    }

    status = FT_SetFlowControl(this->ctx, FT_FLOW_NONE, 0, 0);
    if (status != FT_OK) {
        return -1;
    }

    status = FT_Purge(this->ctx, FT_PURGE_RX | FT_PURGE_TX);
    if (status != FT_OK) {
        return -1;
    }

    status = FT_SetTimeouts(this->ctx, 10, 10);
    if (status != FT_OK) {
        return -1;
    }
#endif
    return 0;
}

void FTDI::setBaudRate(uint32_t rate) {
#ifdef LIBFTDI
    int err = ftdi_set_baudrate(&this->ctx, rate);
    if (err < 0) {
        this->log->error("[FTDI] Failed to set baud rate: %s", ftdi_get_error_string(&this->ctx));
    }
#else
    FT_STATUS status = FT_SetBaudRate(this->ctx, 115200);
    if (status != FT_OK) {
        this->log->error("[FTDI] Failed to set baud rate: %d", status);
    }
#endif
}

void FTDI::poll() {
    if (!this->rxEnabled) {
        return;
    }

    if (this->handler == NULL) {
        return;
    }

#ifdef LIBFTDI
    uint16_t status;
    int read = ftdi_read_data_and_status(&this->ctx, this->rxBuffer, this->rxBufferSize, &status);
    if (read < 0) {
        this->log->error("[FTDI] Failed to read data: %s", ftdi_get_error_string(&this->ctx));
        return;
    }

    // First check if there is a break condition or parity/framing error
    bool breakReceived = false;
    if (status & ftdi_modem_status::PE) {
        this->handler->uartSignal(firm::uart::ERR_PARITY);
    }
    if (status & ftdi_modem_status::FE) {
        this->handler->uartSignal(firm::uart::ERR_FRAME);
    }
    if (status & ftdi_modem_status::BI) {
        breakReceived = true;
        this->handler->uartSignal(firm::uart::BREAK);
    }

    // In case of a BREAK the buffer seems to contain a leading 0x00,
    // which we have to discard.
    int start = breakReceived ? 1 : 0;
    for (int i = start; i < read; i++) {
        this->handler->uartData(this->rxBuffer[i]);
    }
#else
    // Check for break condition
    ULONG modemStatus;
    FT_STATUS status = FT_GetModemStatus(this->ctx, &modemStatus);
    if (status != FT_OK) {
        this->log->error("[FTDI] Failed to get modem status: %d", status);
        return;
    }
    if (modemStatus & (0x10 << 8)) {
        this->handler->uartSignal(firm::uart::BREAK);
    } else if (modemStatus & (0x08 << 8)) {
        this->handler->uartSignal(firm::uart::ERR_FRAME);
    } else if (modemStatus & (0x04 << 8)) {
        this->handler->uartSignal(firm::uart::ERR_PARITY);
    }

    // Check for data
    DWORD rxBytes;
    status = FT_GetQueueStatus(this->ctx, &rxBytes);
    if (status != FT_OK) {
        this->log->error("[FTDI] Failed to get queue status: %d", status);
        return;
    }
    if (rxBytes == 0) {
        return;
    }

    // Data available, read it
    DWORD read;
    status = FT_Read(this->ctx, this->rxBuffer, this->rxBufferSize, &read);
    if (status != FT_OK) {
        this->log->error("[FTDI] Failed to read data: %d", status);
        return;
    }
    for (DWORD i = 0; i < read; i++) {
        this->handler->uartData(this->rxBuffer[i]);
    }
#endif

    return;
}

void FTDI::enableRx(bool enabled) {
    this->rxEnabled = enabled;
}

void FTDI::enableTx(bool enabled) {
    // NOOP - TX always enabled
}

void FTDI::sendByte(uint8_t value) {
#ifdef LIBFTDI
    int written = ftdi_write_data(&this->ctx, &value, 1);
    if (written < 0) {
        this->log->error("[FTDI] Failed write byte: %s", ftdi_get_error_string(&this->ctx));
    }
#else
    FT_STATUS status = FT_Write(this->ctx, &value, 1, NULL);
    if (status != FT_OK) {
        this->log->error("[FTDI] Failed write byte: %d", status);
    }
#endif
}

void FTDI::sendBytes(uint8_t *values, size_t count) {
#ifdef LIBFTDI
    int written = ftdi_write_data(&this->ctx, values, count);
    if (written < 0) {
        this->log->error("[FTDI] Failed write byte: %s", ftdi_get_error_string(&this->ctx));
    } else if (written != count) {
        this->log->error("[FTDI] Incomplete write: %d/%zu", written, count);
    }
#else
    DWORD written;
    FT_STATUS status = FT_Write(this->ctx, values, count, &written);
    if (status != FT_OK) {
        this->log->error("[FTDI] Failed write byte: %d", status);
    } else if (written != count) {
        this->log->error("[FTDI] Incomplete write: %d/%zu", written, count);
    }
#endif
}

void FTDI::awaitTxBufferEmpty() {
    // NOOP - writes are synchronous
}

void FTDI::sendBreak(uint16_t durationUs) {
#ifdef LIBFTDI
    int err = ftdi_set_line_property2(&this->ctx, this->props.bits, this->props.stopbits, this->props.parity, BREAK_ON);
    if (err != 0) {
        this->log->error("[FTDI] Failed to set break: %s", ftdi_get_error_string(&this->ctx));
    }
#else
    FT_STATUS status = FT_SetBreakOn(this->ctx);
    if (status != FT_OK) {
        this->log->error("[FTDI] Failed to set break: %d", status);
    }
#endif

    firm::timing::delay_us(durationUs);

#ifdef LIBFTDI
    err = ftdi_set_line_property2(&this->ctx, this->props.bits, this->props.stopbits, this->props.parity, BREAK_OFF);
    if (err != 0) {
        this->log->error("[FTDI] Failed to set break: %s", ftdi_get_error_string(&this->ctx));
    }
#else
    status = FT_SetBreakOff(this->ctx);
    if (status != FT_OK) {
        this->log->error("[FTDI] Failed to set break: %d", status);
    }
#endif
}

void FTDI::registerHandler(firm::uart::Handler *handler) {
    this->handler = handler;
}

} // namespace ftdi
} // namespace firm
