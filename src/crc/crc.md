# CRC Selection

Tinbus uses a 16 bit CRC to provide frame error detection. For a maximum frame size of up to 135 bits a Hamming Distance of 6 can be achieved with the following polynomial (aka CRC-16-DNP)

0x3D65 = x^16^ + x^13^ + x^12^ + x^11^ + x^10^ + x^8^ + x^6^ + x^5^ + x^2^ +1

The payload will normally be less than 15 bytes (120 bits).

PyCRC can be used to generate the appropriate c code for any polynomial. The use of algorithmic or look up table based CRC calculation is based on the particular application.

Also see

http://blog.martincowen.me.uk/clearing-up-crc-terminology-and-representations-of-polynomials.html
