#pragma once

#include "firm/logging/logger.hpp"
#include "firm/uart.hpp"
#include "ftdi.h"

namespace firm {
namespace ftdi {

/**
 * FTDI is an implementation of the UART interface for FTDI USB-serial adapters.
 *
 * @note Before using any other methods of this class, first, the `open()` and then
 *       the `configure()` method must be called to set the initial configuration.
 */
class FTDI : public firm::uart::UART {
public:
    FTDI(firm::logging::Logger *log) : log(log), handler(NULL), rxEnabled(false) {
        ftdi_init(&this->ctx);
    };
    ~FTDI() {
        ftdi_usb_close(&this->ctx);
        ftdi_deinit(&this->ctx);
    };
    int open(uint16_t vid, uint16_t pid);
    int configure(firm::uart::Config_t config);
    void setBaudRate(uint32_t rate);
    void enableRx(bool enabled);
    void enableTx(bool enabled);
    void sendByte(uint8_t value);
    void sendBytes(uint8_t *values, size_t count);
    void awaitTxBufferEmpty();
    void sendBreak(uint16_t durationUs);
    void registerHandler(firm::uart::Handler *handler);

    static int streamCallback(uint8_t *buffer, int length, FTDIProgressInfo *progress, void *userdata);

private:
    struct ftdi_context ctx;
    firm::logging::Logger *log;
    firm::uart::Handler *handler;

    struct {
        ftdi_bits_type bits;
        ftdi_stopbits_type stopbits;
        ftdi_parity_type parity;
    } props;

    bool rxEnabled;

    /**
     * This method is called by `static streamCallback()`. Do not call manually.
     *
     * @param buffer
     * @param length
     * @param progress
     * @param userdata
     * @return int
     */
    int streamCallback(uint8_t *buffer, int length, FTDIProgressInfo *progress);
};

} // namespace ftdi
} // namespace firm
