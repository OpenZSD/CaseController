Author: Jerry Z.

Contained are the arduino sketches for an assortment of components for a custom computer case. These include a fan and LED control system, a mini oLED display driver and an animated power/reset button controller. The power button sketch relies on reading a voltage difference of the HDD and PWR led to determine animation between on, sleep and off. Additionaly there is one pin available in the event the system doesn't have a blink on sleep indication to signal. It's recommended to have a pull down resistor on this pin.

Future note: some Qt programs may also be included to provide a UI control to those components.
