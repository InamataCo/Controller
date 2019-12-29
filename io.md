# IO

## Analog Pins on the ESP32 DevKit (57mm Variant)

The following pins can be used to perform analog voltage measurements:

Pin Name | Pin #
---------|------
D32      |32
D33      |33
D34      |34
D35      |35
VN       |36
VP       |39

When powering the board with USB, connect the ground cable to the GND pin. This acts to unify the ground voltages on the board and of the analog signals.

⚠️ **Warning:** ensure that the maximum value of the analog signal is not greater than the supply current (3.3V).
