# CRC Selection

Tinbus will use very small frames and so an 8 bit CRC is used to provide useful data error detection without significant payload or compuational overhead.

It is invisaged that the payload will normally be less than 16 bytes (128 bits) and so a good choice of CRC would be polynomial C2, with value 0x97 (x^8 +x^5 +x^3 +x^2 + x + 1).

PyCRC can be used to generate the appropriate c code for the polynomial. Algorithmic or look up table based CRC calculation is choosen based on the particular application.