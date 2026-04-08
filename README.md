# Qualia ESP32-S3 480x480 RGB Bring-Up

This repository is a minimal ESP-IDF display bring-up for the Adafruit Qualia ESP32-S3 RGB display board using the fixed board pin map below and the ESP-IDF native RGB LCD driver.

## Source of board assumptions

- The Qualia board GPIO assignments and PCA9554 usage in this repo come from the fixed pin map provided for this project.
- The default 480x480 panel profile in [`main/panel_480x480.c`](/Users/bchelf/display-tests/main/panel_480x480.c) is based on Adafruit's published TL034WVS05 3.4" 480x480 setup and timings from the Adafruit Learn guide:
  - https://learn.adafruit.com/adafruit-qualia-esp32-s3-for-rgb666-displays/qualia-rgb666-with-tl034-3-4-480x480-square-display
  - https://learn.adafruit.com/adafruit-qualia-esp32-s3-for-rgb666-displays/pinouts
- ESP-IDF RGB panel usage follows the official `esp_lcd` RGB panel API:
  - https://docs.espressif.com/projects/esp-idf/en/v5.1.1/esp32s3/api-reference/peripherals/lcd.html

## Known vs assumed

- Known:
  - Qualia board pin routing
  - PCA9554 address and bit assignments
  - The panel command side is driven over an expander-backed 3-wire, 9-bit command/data bus
  - A working 480x480 init sequence and timing set exists for the TL034WVS05 panel
- Assumed:
  - `BACKLIGHT_BIT` is active high
  - The RGB bus should be driven as RGB565 over the board's 16 routed data lines
  - The default panel in this repo is the Adafruit TL034WVS05-B1477A profile unless you replace it

## Where to edit panel-specific values

- Panel init sequence: [`main/panel_480x480.c`](/Users/bchelf/display-tests/main/panel_480x480.c)
- Panel RGB timing values: [`main/panel_480x480.c`](/Users/bchelf/display-tests/main/panel_480x480.c)
- Board pin map and PCA9554 bit definitions: [`main/board_qualia_s3.h`](/Users/bchelf/display-tests/main/board_qualia_s3.h)

## Build

```sh
idf.py set-target esp32s3
idf.py build
idf.py flash monitor
```

This workspace did not have `idf.py` on `PATH` during implementation, so the project files were created for a normal ESP-IDF environment but not validated with a local build in this session.
