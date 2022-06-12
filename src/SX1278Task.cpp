#include "SX1278Task.h"

#include "boards.h"

int SX1278Task::numOfHandle;
bool* SX1278Task::Handles[8];

bool SX1278Task::setup(float _freq, float _bw, uint8_t _sf, uint8_t _cr, int8_t _power, uint8_t _gain,
                       uint8_t _dutyPercent, uint16_t _preambleLength) {
  freq = _freq;
  bw = _bw;
  sf = _sf;
  cr = _cr;
  power = _power;
  preambleLength = _preambleLength;
  gain = _gain;

  Handles[numOfHandle++] = &Handle;

#if defined(ARDUINO_TTGO_LoRa32_v21new)
  SPI.begin(SCK, MISO, MOSI);
  static Module module(LORA_CS, LORA_IRQ, RADIOLIB_NC, RADIOLIB_NC, SPI);
#elif defined(ARDUINO_T_Beam)
  SPI.begin(SCK, MISO, MOSI);
  static Module module(LORA_CS, LORA_IRQ, LORA_RST, LORA_IO1, SPI);
#elif defined(ARDUINO_NRF52840_PCA10056)
  SPI.begin(SCK, MISO, MOSI);
  static Module module(RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC, SPI);
#else
#error Unknown board.
#endif

  sx = new SX1278(&module);
  super::setup(sx);

  Serial.print(F("Init Freq: "));
  Serial.print(freq);
  Serial.print(F("MHz,"));

  Serial.print(F(" BandWidth: "));
  Serial.print(bw);
  Serial.print(F("kHz,"));

  Serial.print(F(" SpreadingFactor: "));
  Serial.print(sf);
  Serial.print(F(","));

  Serial.print(F(" CodingRate:4/"));
  Serial.print(cr);
  Serial.print(F(","));

  Serial.print(F(" SyncWord:0x"));
  Serial.print(syncWord, HEX);
  Serial.print(F(","));

  if (enableTX) {
    Serial.print(F(" Power: "));
    Serial.print(pow(10, (float)power / 10));
    Serial.print(F("mW,"));

    Serial.print(F(" PreambleLen: "));
    Serial.print(preambleLength);
    Serial.print(F(","));
  }
  if (enableRX) {
    Serial.print(F(" RXGain: "));
    if (gain != 0) {
      Serial.print(gain);
      Serial.print(F(","));
    } else {
      Serial.print(F("Auto,"));
    }
  }
  Serial.println();

  int16_t state;
  if (bw < 7.8) {
    state = sx->beginFSK(freq, 1.2, bw, 2.7, power);
  } else {
    state = sx->begin(freq, bw, sf, cr, syncWord, power, preambleLength, gain);
  }
  switch (state) {
    case RADIOLIB_ERR_NONE:
      break;
    case RADIOLIB_ERR_INVALID_FREQUENCY:
      Serial.printf("SX1278 init failed, The supplied frequency value (%fMHz) is invalid for this module.\n", freq);
      enableRX = false;
      enableTX = false;
      break;
    case RADIOLIB_ERR_INVALID_BANDWIDTH:
      Serial.printf(
          "SX1278 init failed, The supplied bandwidth value (%fkHz) is invalid for this module. Should be 7.8, 10.4, "
          "15.6, 20.8, 31.25, 41.7 ,62.5, 125, 250, 500.\n",
          bw);
      enableRX = false;
      enableTX = false;
      break;
    case RADIOLIB_ERR_INVALID_SPREADING_FACTOR:
      Serial.printf("SX1278 init failed, The supplied spreading factor value (%d) is invalid for this module.\n", sf);
      enableRX = false;
      enableTX = false;
      break;
    case RADIOLIB_ERR_INVALID_CODING_RATE:
      Serial.printf("SX1278 init failed, The supplied coding rate value (%d) is invalid for this module.\n", cr);
      enableRX = false;
      enableTX = false;
      break;
    case RADIOLIB_ERR_INVALID_OUTPUT_POWER:
      Serial.printf("SX1278 init failed, The supplied output power value (%d) is invalid for this module.\n", power);
      enableTX = false;
      break;
    case RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH:
      Serial.printf("SX1278 init failed, The supplied preamble length (%d) is invalid.\n", preambleLength);
      enableTX = false;
      break;
    case RADIOLIB_ERR_INVALID_GAIN:
      Serial.printf("SX1278 init failed, The supplied gain value (%d) is invalid.\n", gain);
      enableRX = false;
      break;
    default:
      Serial.print(F("SX1278 Init failed, code "));
      Serial.println(state);
      return false;
  }
  if (bw < 7.8) {
    state = sx->transmitDirect((freq * 1000000.0) / sx->getFreqStep());
    if (state != RADIOLIB_ERR_NONE) {
      Serial.println(String("transmitDirect") + state);
    }
    while (true) {
      if (bw == 0) {
        Serial.println("N0N");
      } else {
        Serial.println("F2N");
      }
      if (power > 17) {
        Serial.println(
            F("The duty cycle of transmission at +20 dBm is limited to 1%.\n"
              "Please set a output power below 17dBm."));
      }
      delay(10000);
    }
  }

  sx->setRfSwitchPins(RADIOLIB_NC, RADIOLIB_NC);

  state = sx->setCRC(true);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("setCRC failed, code "));
    Serial.println(state);
    return false;
  }

  Serial.println(F("Setting done."));

  sx->setDio0Action([]() {
    for (int i = 0; i < numOfHandle; i++) {
      *Handles[i] = true;
    }
  });

  if (enableRX) {
    state = sx->startReceive();
    if (state != RADIOLIB_ERR_NONE) {
      Serial.print(F("startReceive failed, code "));
      Serial.println(state);
      return false;
    }
  }

  if (power >= 20) {
    dutyPercent = 1;  // pp.84 in datasheet.
  } else {
    dutyPercent = _dutyPercent;
  }

  return true;
}

bool SX1278Task::loop() {
  bool resp = super::TXloop();
  if (!resp) {
    return resp;
  }

  if (Handle) {
    int irqflags = sx->getIRQFlags();

    if (irqflags & RADIOLIB_SX127X_CLEAR_IRQ_FLAG_RX_DONE) {
      RXEnd = millis();

      size_t len = sx->getPacketLength();
      uint8_t buffer[256];
      int state = sx->readData(buffer, len);

      if (buffer[0] == '<' && buffer[1] == '\xff' && buffer[2] == '\x01') {
        uint8_t* data = buffer;

        data += 3;
        len -= 3;

        if (data[len - 1] == '\r') {
          data[len - 1] = '\0';
          len--;
        }

        Payload pl(data, len);

        //

        LoRaRXPayload pkt(pl, sx->getRSSI(), sx->getSNR(), sx->getFrequencyError(false));

        switch (state) {
          case RADIOLIB_ERR_NONE:
            pkt.setCRCErr((irqflags & RADIOLIB_SX127X_CLEAR_IRQ_FLAG_PAYLOAD_CRC_ERROR) != 0);
            break;
          case RADIOLIB_ERR_LORA_HEADER_DAMAGED:
          case RADIOLIB_ERR_CRC_MISMATCH:
            pkt.setCRCErr(true);
            break;
        }

        Serial.print("LORA->:" + pkt.toString());
        Serial.print(String(", ") + pkt.getRSSI() + "dBm, " + pkt.getSNR() + "dB, " + pkt.getFrequencyError() + "Hz");
        if (pkt.getCRCErr()) {
          Serial.println(F("[CRCErr]"));
        } else {
          AX25UI_RXQueue.push_back(pkt);
          Serial.println();
        }
      }

      if (!startReceive()) {  // RXCONTINUOUS not work properly.
        return false;
      }

      RXCarrierDetected = 0;
    }

    if (irqflags & RADIOLIB_SX127X_CLEAR_IRQ_FLAG_TX_DONE) {
      if (!TXDone()) {
        return false;
      }
    }
    Handle = false;
  }

  return true;
}

bool SX1278Task::startReceive() const {
  if (enableRX) {
    uint16_t state = sx->startReceive();
    if (state != RADIOLIB_ERR_NONE) {
      Serial.print(F("LoRa startReceive failed, code "));
      Serial.println(state);
      return false;
    }
  }
  return true;
}