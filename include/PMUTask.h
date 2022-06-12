#include "boards.h"

#if defined(PMU_IRQ)
#include <axp20x.h>
bool PMUIrq;
void ARDUINO_ISR_ATTR PMUIRQIntr() { PMUIrq = true; }
AXP20X_Class axp;
#endif

class PMUTask {
 public:
  bool setup() {
#if defined(PMU_IRQ)

    //! Use the Wire port
    int ret = axp.begin(Wire, AXP192_SLAVE_ADDRESS);
    if (ret == AXP_FAIL) {
      Serial.println("AXP Power begin failed");
      while (1)
        ;
    }

#define LDO_GPS AXP192_LDO3
#define LDO_LORA AXP192_LDO2
#define LDO_OLED AXP192_DCDC1
#define LDO_CPU AXP192_DCDC3

    axp.setPowerOutPut(LDO_LORA, AXP202_ON);
    axp.setPowerOutPut(LDO_OLED, AXP202_ON);
    axp.setPowerOutPut(LDO_GPS, AXP202_ON);

    pinMode(PMU_IRQ, INPUT_PULLUP);
    attachInterrupt(PMU_IRQ, PMUIRQIntr, FALLING);

    axp.clearIRQ();
    axp.enableIRQ(AXP202_PEK_LONGPRESS_IRQ, true);
#endif
    return true;
  }

  bool loop() {
#if defined(PMU_IRQ)
    if (PMUIrq) {
      axp.readIRQ();
      PMUIrq = false;
      if (axp.isPEKLongtPressIRQ()) {
        Serial.println("PowerOff.");
        axp.shutdown();
        delay(1000);
        // never reach here.
      }
      axp.clearIRQ();
    }
#endif
    return true;
  }
};