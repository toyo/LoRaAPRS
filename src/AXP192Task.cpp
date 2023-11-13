#include "AXP192Task.h"

#if defined(XPOWERS_CHIP_AXP192)

#include <XPowersLib.h>

// volatile SemaphoreHandle_t AXP192Task::pmuSemaphore;

XPowersPMU axp;

bool AXP192Task::setup() {
  //! Use the Wire port
  int ret = axp.begin(Wire, AXP192_SLAVE_ADDRESS, SDA, SCL);
  if (ret == false) {
    Serial.println("AXP Power begin failed");
    while (1) delay(100);
  }

  pinMode(PMU_IRQ, INPUT_PULLUP);
  pmuSemaphore = xSemaphoreCreateBinary();
  attachInterruptArg(
      PMU_IRQ, [](void* sem) IRAM_ATTR { xSemaphoreGiveFromISR(static_cast<SemaphoreHandle_t>(sem), NULL); },
      pmuSemaphore, FALLING);

  axp.setVbusVoltageLimit(XPOWERS_AXP192_VBUS_VOL_LIM_4V);
  axp.setVbusCurrentLimit(XPOWERS_AXP192_VBUS_CUR_LIM_OFF);

  axp.setLDO2Voltage(2700);  // 2400-3700mV for +20dBm
  axp.enableLDO2();          // LDO_LORA

  axp.setDC1Voltage(2500);  // 2500mV
  axp.enableDC1();          // LDO_OLED

  axp.setLDO3Voltage(3000);  // 2700-3000-3600mV for NEO6M/NEOM8N
  axp.enableLDO3();          // LDO_GPS

  axp.setDC3Voltage(3300);  // 3000-3300-3600mV for ESP32 with flash or PSRAM
  axp.enableDC3();          // LDO_CPU

  axp.disableDC2();

  axp.disableIRQ(XPOWERS_AXP192_ALL_IRQ);
  axp.enableIRQ(XPOWERS_AXP192_PKEY_LONG_IRQ);
  axp.clearIrqStatus();

  return true;
}

void AXP192Task::task() {
  if (xSemaphoreTake(pmuSemaphore, portMAX_DELAY) == pdTRUE) {
    // Get AXP192 Interrupt Status Register
    axp.getIrqStatus();

    if (axp.isPekeyLongPressIrq()) {
      Serial.println("PowerOff.");
      axp.shutdown();
      vTaskDelay(1000 / portTICK_RATE_MS);
      // never reach here.
    }
    axp.clearIrqStatus();
    return;
  }
}

#endif
