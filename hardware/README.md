# Hardware design

`codex-lite.sch` is the KiCad schematic source for the Codex Light wiring.
Open it with KiCad's Schematic Editor. KiCad will offer to convert this
legacy, text-based schematic to its current `.kicad_sch` format; commit the
converted file together with any later design changes.

The schematic follows the firmware pin configuration:

- GPIO9, GPIO10 and GPIO11 drive the red, yellow and green LEDs through
  individual 330 ohm resistors.
- The 1.54-inch ST7789 SPI module uses GPIO12--15 and GPIO26--27.
- All logic uses 3.3 V. The TFT module must accept 3.3 V logic levels.
