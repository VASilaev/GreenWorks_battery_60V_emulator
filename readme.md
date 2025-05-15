It will work with an Arduino based on Atmega328 or a Chinese LGT8F328P clone (developed on the latter). It was possible to achieve stable operation on a 60V trimmer.
IO_PIN needs to be connected to the Ohm terminal via a 150 ohm resistor.

Эмулятор батареи на 60 вольт для инструмента Greenworks. 

Добавлена поддержка простой батареи 24В. Для ее выбора замените на соответствующий вариант

```c++
const uint8_t variant = VARIANT_24V_ONLY_PREAMBULA;
```