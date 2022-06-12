#ifndef _BOARDS_H_
#define _BOARDS_H_

#if defined(ARDUINO_TTGO_LoRa32_v21new)
#define ENABLE_WIFI
#define ENABLE_U8G2
#define BUTTON1 -1

#elif defined(ARDUINO_T_Beam)
#define ENABLE_GPS
#define ENABLE_U8G2
#define BUTTON1 38
#define PMU_IRQ 35

#elif defined(ARDUINO_NRF52840_PCA10056)
#error Unsupported board.

#else
#error Unknown board.

#endif

#endif  // _BOARDS_H_
