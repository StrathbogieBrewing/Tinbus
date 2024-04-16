#include <string.h>

#include "format.h"

int _itoa(format_str_t str, int16_t value, format_t format)
{
  unsigned char digit;
  uint8_t enable_digits = 0;
	char* ptr;

	ptr = str;

  // do sign
  if (value < 0){
      value = -value;
      *str++ = '-';
  }

  // do 10000's
 	digit = '0';
  while (value > 9999){
      value -= 10000;
      digit++;
  }
  if ((digit != '0') || (enable_digits != 0)) {
      *str++ = digit; 
      enable_digits = 1;
  }

  // do 1000's
  digit = '0';
  while(value > 999){
      value -= 1000;
      digit++;
  }
  if((digit != '0') || (enable_digits != 0)) {
      *str++ = digit;
      enable_digits = 1;
  }

  // do 100's
  digit = '0';
  while(value > 99){
      value -= 100;
      digit++;
  }
  if (format == FORMAT_MILLI){
      if (enable_digits == 0) *str++ = '0';
      *str++ = '.';
      enable_digits = 1;
  }
  if((digit != '0') || (enable_digits != 0)) {
      *str++ = digit;
      enable_digits = 1;
  }

  // do 10's
  digit = '0';
  while(value > 9){
    value -= 10;
    digit++;
  }
  if (format == FORMAT_CENTI){
    if (enable_digits == 0) *str++ = '0';
    *str++ = '.';
    enable_digits = 1;
  }
  if((digit != '0') || (enable_digits != 0)  || (format == FORMAT_TWO_DIGITS) ) {
    *str++ = digit;
    enable_digits = 1;
  }

  // do 1's
  digit = '0' + value;
  if (format == FORMAT_DECI){
    if (enable_digits == 0) *str++ = '0';
    *str++ = '.';
  }
  *str++ = digit;
  *str = 0;

	return str - ptr;
}


/// Convert integer to ascii decimal string and send to terminal
/// @param integer value
/// @param desired ascii decimal format
// uint8_t itoa_toString(char* str, int16_t value, uint8_t format)
uint8_t format_toString(format_str_t str, int16_t value, format_t format)
{
	int16_t hours = 0;

	if(format == FORMAT_TIME){
		while (value >= 60) {++hours ; value -= 60;}
		str += _itoa(str, hours, FORMAT_TWO_DIGITS);
		*str++ = '.'; 
		_itoa(str, value, FORMAT_TWO_DIGITS);
	} else if(format == FORMAT_BOOL) {
		if(value) strcpy(str, "on"); else strcpy(str, "off");
	} else {
		_itoa(str, value, format);
	}
	return strlen(str);
}







// #include "itoa.h"

// void itoa(int val, int format, FILE *stream)
// {
//     unsigned char digit;
//     uint8_t enable_digits = 0;
//     // do sign
//     if (val < 0)
//     {
//         val = -val;
//         disp.write('-');
//     }

//     // do 10000's
//     digit = '0';
//     while (val > 9999)
//     {
//         val -= 10000;
//         digit++;
//     }
//     if ((digit != '0') || (enable_digits != 0))
//     {
//         disp.write(digit);
//         enable_digits = 1;
//     }
//     // do 1000's
//     digit = '0';
//     while (val > 999)
//     {
//         val -= 1000;
//         digit++;
//     }
//     if ((digit != '0') || (enable_digits != 0))
//     {
//         disp.write(digit);
//         enable_digits = 1;
//     }
//     // do 100's
//     digit = '0';
//     while (val > 99)
//     {
//         val -= 100;
//         digit++;
//     }
//     // if (format == Milli) {
//     //   if (enable_digits == 0)
//     //     disp.write('0');
//     //   disp.write('.');
//     //   enable_digits = 1;
//     // }
//     if ((digit != '0') || (enable_digits != 0))
//     {
//         disp.write(digit);
//         enable_digits = 1;
//     }
//     // do 10's
//     digit = '0';
//     while (val > 9)
//     {
//         val -= 10;
//         digit++;
//     }
//     if (format == Centi)
//     {
//         if (enable_digits == 0)
//             disp.write('0');
//         disp.write('.');
//         enable_digits = 1;
//     }
//     if ((digit != '0') || (enable_digits != 0) || (format == FORMAT_TWO_DIGITS))
//     {
//         disp.write(digit);
//         enable_digits = 1;
//     }
//     // do 1's
//     digit = '0' + val;
//     if (format == Deci)
//     {
//         if (enable_digits == 0)
//             disp.write('0');
//         disp.write('.');
//         enable_digits = 1;
//     }
//     if ((digit != '0') || (enable_digits != 0) || (format == Unit))
//     {
//         disp.write(digit);
//         enable_digits = 1;
//     }
// }