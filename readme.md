# avr-platformio-template

Simple PlatformIO project for an ATmega328 (Arduino Uno) that demonstrates
a basic UART driver: initializing the UART, printing strings, and printing
a value in binary.

## Project structure

- `src/main.c` — entry point, initializes UART and prints a startup message
- `lib/uart` — minimal UART driver (`uart_init`, `uart_write`/`uart_read`,
  `uart_print`/`uart_println`, `uart_print_uint`/`uart_print_int`/`uart_print_bin`)
- `lib/millis` — Timer0-based millisecond counter (`millis_init`, `millis`)
- `platformio.ini` — PlatformIO config (`atmelavr`, `uno` board)

## Building

```bash
pio run
```

## uart print

`lib/uart` provides simple helpers for sending data over the UART, as used
in `src/main.c`:

```c
#include <avr/io.h>
#include <uart.h>

int main(void)
{
    uart_init(9600);
    uart_println("Program started");

    uart_print_bin(1 << 0);
}
```

## millis

`lib/millis` provides an Arduino-style `millis()` counter driven by Timer0,
useful for non-blocking timing instead of `_delay_ms`. Here it's used to
print a message over UART once a second:

```c
#include <avr/io.h>
#include <uart.h>
#include <millis.h>

int main(void)
{
    uart_init(9600);
    millis_init();

    uint32_t last = 0;

    while (1) {
        uint32_t now = millis();

        if (now - last >= 1000) {
            last = now;
            uart_println("tick");
        }
    }
}
```

## spi
`lib/spi` provides an hardware spi.

```c
#include <avr/io.h>
#include <util/delay.h>
#include <spi.h>

#define LATCH_PIN_DISPLAY  PB2
#define LATCH_PIN_BUTTONS  PB1

const uint8_t digit_map[10] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111  // 9
};

void display_digit(uint8_t digit);
uint8_t read_shift_register();

int main() {
  DDRB |= (1 << LATCH_PIN_DISPLAY) | (1 << LATCH_PIN_BUTTONS);
  spi_init();
  while (1) {
    uint8_t input = read_shift_register();

    if (bit_is_clear(input, 0)) {
      display_digit(1);
    } else if (bit_is_clear(input, 1)) {
      display_digit(2);
    } else if (bit_is_clear(input, 2)) {
      display_digit(3);
    } else {
      display_digit(0);
    }

    _delay_ms(50);
  }
}

void display_digit(uint8_t digit) {
  PORTB &= ~(1 << LATCH_PIN_DISPLAY);
  spi_transfer(digit_map[digit]);
  PORTB |= (1 << LATCH_PIN_DISPLAY);
}

uint8_t read_shift_register() {
  uint8_t data;

  PORTB &= ~(1 << LATCH_PIN_BUTTONS);
  _delay_us(1); 
  PORTB |= (1 << LATCH_PIN_BUTTONS);

  data = spi_read();

  return data;
}

```

## max72
```c
#include <avr/io.h>
#include <util/delay.h>
#include <spi.h>

#define MAX7219_DECODE_MODE_REG 0x09
#define MAX7219_INTENSITY_REG 0x0a
#define MAX7219_SCAN_LIMIT_REG 0x0b
#define MAX7219_SHUTDOWN_REG 0x0c
#define MAX7219_DISPLAY_TEST_REG 0x0f

#define LATCH_PIN PB2

void send_to_max7219(uint8_t addr, uint8_t data);
void setup_max7219_led_matrix(void);


uint8_t Cat[8] = {
  0b10001000,
  0b11111000,
  0b10101000,
  0b01110001,
  0b00100001,
  0b01111001,
  0b01111101,
  0b10111110
};


int main(void) {
  DDRB |= 1 << LATCH_PIN;

  spi_init();
  setup_max7219_led_matrix();

  for (int i = 0; i < 8; i++) {
    send_to_max7219(i + 1, Cat[i]);
  }
}

void send_to_max7219(uint8_t addr, uint8_t data) {
  spi_transfer(addr);
  spi_transfer(data);

  PORTB |= 1 << LATCH_PIN;
  _delay_us(1);
  PORTB &= ~(1 << LATCH_PIN);
  _delay_us(1);
}


void setup_max7219_led_matrix(void) {
  send_to_max7219(MAX7219_DECODE_MODE_REG, 0);
  send_to_max7219(MAX7219_SCAN_LIMIT_REG, 0xf);
  send_to_max7219(MAX7219_INTENSITY_REG, 0x07);
  send_to_max7219(MAX7219_DISPLAY_TEST_REG, 0);
  send_to_max7219(MAX7219_SHUTDOWN_REG, 1);
}
```


## uart
```c
#include <avr/io.h>
#include <util/delay.h>
#include <uart.h>

#define LED PB5

void blinkNTimes(uint8_t n);

int main() {
    DDRB |= (1 << LED);

    uart_init(9600);

    uart_println("Enter from 1 to 9:");

    while (1) {
        char receivedChar = uart_read();

        if (receivedChar >= '1' && receivedChar <= '9') {
            uint8_t count = receivedChar - '0';

            uart_print("Blinking ");
            uart_print_uint(count);
            uart_println(" times");

            blinkNTimes(count);
        } else {
            uart_println("Not valid command");
        }
    }
}

void blinkNTimes(uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
        PORTB |= (1 << LED);
        _delay_ms(250);

        PORTB &= ~(1 << LED);
        _delay_ms(250);
    }
}
```

## SimulIDE

The circuit can be simulated without any hardware using [SimulIDE](https://simulide.com/).

![ATmega328 UART interface scheme](assets/uart-scheme.png)

The scheme above is a simple ATmega328 UART interface: the chip's `D0` (RX)
and `D1` (TX) pins are cross-wired to a serial terminal component's `Rx`/`Tx`
pins. Load the built firmware (`.hex`) onto the `mega328` part, open the
serial terminal, and start the simulation — the terminal should print
`Program started` followed by the binary output.
