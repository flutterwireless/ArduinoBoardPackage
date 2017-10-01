#ifndef BoardGpioPinMap_h
#define BoardGpioPinMap_h
static const GpioPinMap_t GpioPinMap[] = {
  GPIO_PIN(A, 3),  // D0 TWD0 - SDA1
  GPIO_PIN(A, 4),  // D1 TWCK0 - SCL1
  GPIO_PIN(A, 5),  // D2 RXD0
  GPIO_PIN(B, 6),  // D3 TXD0
  GPIO_PIN(B, 4),  // D4 TDI
  GPIO_PIN(B, 5),  // D5 TDO
  GPIO_PIN(B, 7),  // D6 TCK
  GPIO_PIN(B, 6),  // D6 TCK
  GPIO_PIN(A, 1),  // D8
  GPIO_PIN(A, 2),  // D9
  GPIO_PIN(A, 7),  // D10
  GPIO_PIN(A, 8),  // D11 S0
  GPIO_PIN(A, 20), // D12 S1 RGB_R
  GPIO_PIN(A, 0),  // D13 S2 RGB_B
  GPIO_PIN(A, 16), // D14 S3 RGB_G
  GPIO_PIN(A, 15), // D15 S4 Radio GPIO0
  GPIO_PIN(A, 11), // D16 S5 (Radio GPIO3)
  GPIO_PIN(A, 10), // D17 S6 (Radio
  GPIO_PIN(A, 9),  // D18 S7 CS
  GPIO_PIN(A, 13), // D19 S8 MOSI
  GPIO_PIN(A, 12), // D20 S9 MISO
  GPIO_PIN(A, 14), // D21 S10 SCLK
  GPIO_PIN(B, 2),  // D22 AD0
  GPIO_PIN(B, 3),  // D23 AD1
  GPIO_PIN(A, 17), // D24 AD2
  GPIO_PIN(A, 19), // D25 AD3
  GPIO_PIN(B, 0),  // D26 AD4
  GPIO_PIN(B, 1),  // D27 AD5
  GPIO_PIN(A, 18)  // D28 B2
};
#endif  // BoardGpioPinMap_h
