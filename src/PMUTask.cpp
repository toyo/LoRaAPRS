#include "PMUTask.h"

#if defined(XPOWERS_CHIP_AXP192)

#include "XPowersLib.h"

bool PMUIrq = false;
void ARDUINO_ISR_ATTR PMUIRQIntr() { PMUIrq = true; }
XPowersPMU axp;

bool PMUTask::setup() {
  //! Use the Wire port
  int ret = axp.begin(Wire, AXP192_SLAVE_ADDRESS, SDA, SCL);
  if (ret == false) {
    Serial.println("AXP Power begin failed");
    while (1) delay(100);
  }

  axp.setVbusVoltageLimit(XPOWERS_AXP192_VBUS_VOL_LIM_4V);
  axp.setVbusCurrentLimit(XPOWERS_AXP192_VBUS_CUR_LIM_OFF);

  axp.setDC1Voltage(2500);   // 2500mV
  axp.enableDC1();           // LDO_OLED
  axp.setDC3Voltage(3300);   // 3000-3300-3600mV for ESP32 with flash or PSRAM
  axp.enableDC3();           // LDO_CPU
  axp.setLDO2Voltage(2700);  // 2400-3700mV for +20dBm
  axp.enableLDO2();          // LDO_LORA
  axp.setLDO3Voltage(3000);  // 2700-3000-3600mV for NEO6M/NEOM8N
  axp.enableLDO3();          // LDO_GPS

  pinMode(PMU_IRQ, INPUT_PULLUP);
  attachInterrupt(PMU_IRQ, PMUIRQIntr, FALLING);

  axp.disableIRQ(XPOWERS_AXP192_ALL_IRQ);
  axp.clearIrqStatus();
  axp.enableIRQ(XPOWERS_AXP192_PKEY_LONG_IRQ);

  return true;
}

bool PMUTask::loop() {
  if (PMUIrq) {
    // Get PMU Interrupt Status Register
    axp.getIrqStatus();

    PMUIrq = false;

    if (axp.isPekeyLongPressIrq()) {
      Serial.println("PowerOff.");
      axp.shutdown();
      delay(1000);
      // never reach here.
    }
    axp.clearIrqStatus();
  }
  return true;
}

#endif
