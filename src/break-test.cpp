#ifdef LIBFTDI
#include "ftdi.h"
#else
#include "ftd2xx.h"
#endif
#include <chrono>
#include <iostream>
#include <unistd.h>

using namespace std::chrono;

int main() {
#ifdef LIBFTDI
    ftdi_context *ctx = ftdi_new();
    if (ftdi_usb_open(ctx, 0x0403, 0x6001)) {
        return -1;
    }

    if (ftdi_set_baudrate(ctx, 250000)) {
        return -1;
    }

    if (ftdi_set_line_property(ctx, BITS_8, STOP_BIT_2, NONE)) {
        return -1;
    }
#else
    FT_HANDLE ctx;

    // FT_STATUS status = FT_SetVIDPID(0x0403, 0x6001);
    // if (status != FT_OK) {
    //     std::cerr << "Failed to set VID/PID" << std::endl;
    //     return -1;
    // }

    FT_STATUS status = FT_Open(0, &ctx);
    if (status != FT_OK) {
        std::cerr << "Failed to open device" << std::endl;
        return -1;
    }

    status = FT_ResetDevice(ctx);
    if (status != FT_OK) {
        std::cerr << "Failed to reset device" << std::endl;
        return -1;
    }

    status = FT_SetBaudRate(ctx, 250000);
    if (status != FT_OK) {
        std::cerr << "Failed to set baud rate" << std::endl;
        return -1;
    }

    status = FT_SetDataCharacteristics(ctx, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
    if (status != FT_OK) {
        std::cerr << "Failed to set data characteristics" << std::endl;
        return -1;
    }

    status = FT_SetFlowControl(ctx, FT_FLOW_NONE, 0, 0);
    if (status != FT_OK) {
        std::cerr << "Failed to set flow control" << std::endl;
        return -1;
    }

    status = FT_SetTimeouts(ctx, 1, 1);
    if (status != FT_OK) {
        std::cerr << "Failed to set timeouts" << std::endl;
        return -1;
    }

    status = FT_Purge(ctx, FT_PURGE_RX | FT_PURGE_TX);
    if (status != FT_OK) {
        std::cerr << "Failed to purge" << std::endl;
        return -1;
    }
#endif

    auto start = system_clock::now();
    uint64_t count = 0;
    while (true) {
#ifdef LIBFTDI
        uint16_t status;
        if (ftdi_poll_modem_status(ctx, &status)) {
            return -1;
        }
        bool breakDetected = status & ftdi_modem_status::BI;
#else
        ULONG status;
        if (FT_GetModemStatus(ctx, &status) != FT_OK) {
            std::cerr << "Failed to get modem status" << std::endl;
            return -1;
        }
        bool breakDetected = status & (0x10 << 8);
#endif

        // std::cout << "Status: 0x" << std::hex << status << std::endl;
        if (breakDetected) {
            auto delta = duration_cast<microseconds>(system_clock::now() - start);
            std::cout << "BREAK at iteration " << count << " (" << delta.count() << "µs)" << std::endl;
        }

        ++count;
    }

    return 0;
}
