#include "AX25UITask.h"

void AX25UITask::setCallSign(String _CallSign, bool _digipeat) {
  CallSign = _CallSign;
  digipeat = _digipeat;
}

bool AX25UITask::setup() { return true; }

void AX25UITask::addUITRACE(String _UITRACE) {
  if (_UITRACE != "") {
    UITRACE.insert(_UITRACE);
  }
}

void AX25UITask::taskRX(portTickType xBlockTime) {
  Payload recvd;
  if (xQueueReceive(PayloadRXQ, &recvd, xBlockTime) == pdTRUE) {
    AX25UI ui(recvd.getData(), recvd.getLen());
    if (!ui.isNull()) {
      if (xQueueSend(AX25UIRXQ, &ui, 0) != pdTRUE) {
        Serial.println("error sending AX25 to user.");
      }
      if (digipeat) {  // work as digi.
        if (!ui.findDigiCall((CallSign + "*").c_str()) &&
            strncmp(ui.getFromCall(), CallSign.c_str(), MAXCALLSIGNLEN) != 0) {
          // my call is not in digipeated list or sender.
          int digiindex;
          if ((digiindex = ui.findNextDigiIndex()) != -1) {  // All listed calls are not digipeated.
            String nextDigi = ui.getDigiCalls(digiindex);    // Got next digipeater call
            if (nextDigi == CallSign) {                      // Next digipeater call is mine.
              AX25UI digiUi(ui);
              digiUi.setToDigiCall((CallSign + "*").c_str(), digiindex);
              xQueueSendToFront(AX25UITXQ, &digiUi, portMAX_DELAY);
            } else {
              int index = nextDigi.indexOf('-');
              if (index != -1) {                                 // Has SSID
                String digicall = nextDigi.substring(0, index);  // like "WIDE1"
                if (std::find(UITRACE.begin(), UITRACE.end(), digicall) != UITRACE.end()) {
                  //   http://www.aprs.org/fix14439.html
                  AX25UI digiUi(ui);
                  digiUi.setToDigiCall((CallSign + "*").c_str(), digiindex);  // Add digipeated.
                  int digissid = nextDigi.substring(index + 1).toInt() - 1;   // SSID decrement
                  if (digissid > 0) {
                    digiUi.addToDigiCall((digicall + "-" + digissid).c_str(), digiindex + 1);
                  }
                  xQueueSendToFront(AX25UITXQ, &digiUi, portMAX_DELAY);
                }
              }
            }
          }
        }
      }
    }
  }
}

bool AX25UITask::loopTX() {
  bool isDo = false;
  while (uxQueueMessagesWaiting(AX25UITXQ) != 0 && uxQueueMessagesWaiting(PayloadTXQ) == 0) {
    AX25UI ui;
    if (xQueueReceive(AX25UITXQ, &ui, 0) == pdTRUE) {
      if (!ui.isNull()) {
        Payload uip(ui.Encode());
        xQueueSend(PayloadTXQ, &uip, portMAX_DELAY);
      }
      isDo = true;
    }
  }
  return isDo;
}

size_t AX25UITask::TXQueueSize() { return uxQueueMessagesWaiting(AX25UITXQ) + uxQueueMessagesWaiting(PayloadTXQ); }
