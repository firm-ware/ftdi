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
 *
 * @note Due to the polled nature of the USB protocol, and the callback-based interface
 *       of the UART class, it is required to regularly call the `poll()` method in
 *       order to check the FTDI device for new data in its RX buffer.
 */
class FTDI : public firm::uart::UART {
public:
    /**
     * Construct a new FTDI object
     *
     * @param rxBuffer Pointer to a buffer where received data will be stored
     * @param rxBufferSize Size of the receive buffer
     * @param log Pointer to a logger instance
     */
    FTDI(uint8_t *rxBuffer, size_t rxBufferSize, firm::logging::Logger *log)
        : rxBuffer(rxBuffer), rxBufferSize(rxBufferSize), log(log), handler(NULL), rxEnabled(false) {
        ftdi_init(&this->ctx);
    };

    ~FTDI() {
        ftdi_usb_close(&this->ctx);
        ftdi_deinit(&this->ctx);
    };

    /**
     * Establish a connection to the FTDI device with the given VID and PID.
     *
     * @param vid
     * @param pid
     * @return int
     */
    int open(uint16_t vid, uint16_t pid);

    /**
     * Configure the UART interface with the given parameters.
     *
     * @param config
     * @return int
     */
    int configure(firm::uart::Config_t config);

    /**
     * Set the baud rate of the UART interface.
     *
     * @param rate
     */
    void setBaudRate(uint32_t rate);

    /**
     * Poll the FTDI for new data or signals in the RX buffer.
     *
     * If the FTDI reports a break, frame or parity error, the `uartSignal()` method
     * of the registered handler is called with the corresponding signal type. If
     * data is available, the `uartData()` method of the handler will be called.
     *
     * If RX is disabled, this method will do nothing.
     */
    void poll();

    void enableRx(bool enabled);
    void enableTx(bool enabled);
    void sendByte(uint8_t value);
    void sendBytes(uint8_t *values, size_t count);
    void awaitTxBufferEmpty();
    void sendBreak(uint16_t durationUs);
    void registerHandler(firm::uart::Handler *handler);

private:
    struct ftdi_context ctx;

    uint8_t *rxBuffer;
    size_t rxBufferSize;

    firm::logging::Logger *log;
    firm::uart::Handler *handler;

    struct {
        ftdi_bits_type bits;
        ftdi_stopbits_type stopbits;
        ftdi_parity_type parity;
    } props;

    bool rxEnabled;
};

} // namespace ftdi
} // namespace firm
