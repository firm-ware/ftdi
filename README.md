# FTDI drivers for firm

This repo contains an implementation of the `firm::uart::UART` interface backed by [libftdi](https://www.intra2net.com/en/developer/libftdi/).

## BREAK detection of FT232RL

According to my test (N=1), the FT232RL (possibly others) does not reliably detect BREAK conditions on its RX line. This makes the device particularly unsuitable as DMX receiver or RDM controller/responder.

You can test your chip using the `break-test.cpp` contained in this repo. A cmake target with the same name exists. The test code continuously polls the `ftdi_poll_modem_status()` and checks the result for the break interrupt bit. If you connect it to a DMX controller sending full frames at the maximum frequency, it should detect about 44 breaks per second. In the sample capture below it detected 12 breaks within ~10s (won't even bother to calculate the loss ratio):

```
BREAK at iteration 94 (285007µs)
BREAK at iteration 267 (811045µs)
BREAK at iteration 387 (1171010µs)
BREAK at iteration 561 (1696968µs)
BREAK at iteration 681 (2057005µs)
BREAK at iteration 976 (2943004µs)
BREAK at iteration 1068 (3220010µs)
BREAK at iteration 1243 (3746394µs)
BREAK at iteration 1363 (4106085µs)
BREAK at iteration 1805 (5435041µs)
BREAK at iteration 2071 (6238023µs)
BREAK at iteration 2163 (6515011µs)
BREAK at iteration 2456 (7401042µs)
BREAK at iteration 2603 (7844031µs)
BREAK at iteration 2723 (8204035µs)
BREAK at iteration 3046 (9173052µs)
BREAK at iteration 3138 (9450046µs)
BREAK at iteration 3553 (10696053µs)
```

### Why so?

I don't know. The Datasheet does not provide any detail on how the break detection works. The official d2xx library and its accompanying documentation didn't reveal anything, either. So we can just assume it's a design flaw of the chip itself. Maybe the register holding the error status bits is synchronized/sampled in fixed intervals and depending on whether or not the break condition is present during the sample window, the bit is set or not. One would think that the break interrupt bit is set when the break condition is detected and cleared on read. But apparently that's not the case. Or the break detection itself just doesn't work.

### Now what?

...
