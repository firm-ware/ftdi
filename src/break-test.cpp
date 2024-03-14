#include "ftdi.h"
#include <chrono>
#include <iostream>
#include <unistd.h>

using namespace std::chrono;

int main() {
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

    auto start = system_clock::now();
    uint16_t status;
    uint64_t count = 0;
    while (true) {
        if (ftdi_poll_modem_status(ctx, &status)) {
            return -1;
        }

        if (status & ftdi_modem_status::BI) {
            auto delta = duration_cast<microseconds>(system_clock::now() - start);
            std::cout << "BREAK at iteration " << count << " (" << delta.count() << "µs)" << std::endl;
        }
        ++count;
    }

    return 0;
}
