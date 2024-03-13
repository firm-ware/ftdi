#include "firm/ftdi/uart.hpp"
#include "firm/timing.hpp"

namespace firm {
namespace ftdi {

int FTDI::streamCallback(uint8_t *buffer, int length, FTDIProgressInfo *progress, void *userdata) {
    if (userdata == NULL) {
        return -1;
    }

    FTDI *instance = (FTDI *)userdata;
    return instance->streamCallback(buffer, length, progress);
}

int FTDI::streamCallback(uint8_t *buffer, int length, FTDIProgressInfo *progress) {
    // We can abort the stream by returning a non-zero value
    if (!this->rxEnabled) {
        return -1;
    }

    if (this->handler == NULL) {
        return -1;
    }

    // First check if there is a break condition or parity/framing error
    uint16_t status;
    int err = ftdi_poll_modem_status(&this->ctx, &status);
    if (err < 0) {
        this->log->error("[FTDI] Failed to poll modem status: %s", ftdi_get_error_string(&this->ctx));
        return -1;
    }
    if (status & (1 << (8 + 2))) {
        this->handler->uartSignal(firm::uart::ERR_PARITY);
    }
    if (status & (1 << (8 + 3))) {
        this->handler->uartSignal(firm::uart::ERR_FRAME);
    }
    if (status & (1 << (8 + 4))) {
        this->handler->uartSignal(firm::uart::BREAK);
    }

    // Pass data to downstream handler
    for (int i = 0; i < length; i++) {
        this->handler->uartData(buffer[i]);
    }

    return 0;
}

int FTDI::open(uint16_t vid, uint16_t pid) {
    return ftdi_usb_open(&this->ctx, vid, pid);
}

int FTDI::configure(firm::uart::Config_t config) {
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

    return 0;
}

void FTDI::setBaudRate(uint32_t rate) {
    int err = ftdi_set_baudrate(&this->ctx, rate);
    if (err < 0) {
        this->log->error("[FTDI] Failed to set baud rate: %s", ftdi_get_error_string(&this->ctx));
    }
}

void FTDI::enableRx(bool enabled) {
    if (!this->rxEnabled && enabled) {
        int err = ftdi_readstream(&this->ctx, FTDI::streamCallback, this, 2, 4);
        if (err < 0) {
            this->log->error("[FTDI] Failed to start read stream: %s", ftdi_get_error_string(&this->ctx));
        }
    }

    this->rxEnabled = enabled;
}

void FTDI::enableTx(bool enabled) {
    // NOOP - TX always enabled
}

void FTDI::sendByte(uint8_t value) {
    int written = ftdi_write_data(&this->ctx, &value, 1);
    if (written < 0) {
        this->log->error("[FTDI] Failed write byte: %s", ftdi_get_error_string(&this->ctx));
    }
}

void FTDI::sendBytes(uint8_t *values, size_t count) {
    int written = ftdi_write_data(&this->ctx, values, count);
    if (written < 0) {
        this->log->error("[FTDI] Failed write byte: %s", ftdi_get_error_string(&this->ctx));
    } else if (written != count) {
        this->log->error("[FTDI] Incomplete write: %d/%zu", written, count);
    }
}

void FTDI::awaitTxBufferEmpty() {
    // NOOP - writes are synchronous
}

void FTDI::sendBreak(uint16_t durationUs) {
    int err = ftdi_set_line_property2(&this->ctx, this->props.bits, this->props.stopbits, this->props.parity, BREAK_ON);
    if (err != 0) {
        this->log->error("[FTDI] Failed to set break: %s", ftdi_get_error_string(&this->ctx));
    }

    firm::timing::delay_us(durationUs);

    err = ftdi_set_line_property2(&this->ctx, this->props.bits, this->props.stopbits, this->props.parity, BREAK_OFF);
    if (err != 0) {
        this->log->error("[FTDI] Failed to set break: %s", ftdi_get_error_string(&this->ctx));
    }
}

void FTDI::registerHandler(firm::uart::Handler *handler) {
    this->handler = handler;
}

} // namespace ftdi
} // namespace firm
