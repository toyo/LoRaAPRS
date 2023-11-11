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

bool AX25UITask::loop() {
  bool isDo = false;

  Payload recvd((const uint8_t*)"", 0);
  while (xQueueReceive(PayloadRXQ, &recvd, 0) == pdPASS) {
    AX25UI* ui = new AX25UI(recvd.getData(), recvd.getLen());
    if (!ui->isNull()) {
      if (xQueueSend(RXQ, ui, 0) != pdTRUE) {
        Serial.println("error sending AX25 to user.");
      }
      if (digipeat) {  // work as digi.
        if (!ui->findDigiCall(CallSign + "*") && ui->getFromCall() != CallSign) {
          // my call is not in digipeated list or sender.
          int digiindex;
          if ((digiindex = ui->findNextDigiIndex()) != -1) {  // All listed calls are not digipeated.
            String nextDigi = ui->getToDigiCalls(digiindex);  // Got next digipeater call
            if (nextDigi == CallSign) {                       // Next digipeater call is mine.
              AX25UI digiUi(*ui);
              digiUi.setToDigiCall(CallSign + "*", digiindex);
              TXQueue.push_front(digiUi);
            } else {
              int index = nextDigi.indexOf('-');
              if (index != -1) {                                 // Has SSID
                String digicall = nextDigi.substring(0, index);  // like "WIDE1"
                if (std::find(UITRACE.begin(), UITRACE.end(), digicall) != UITRACE.end()) {
                  //   http://www.aprs.org/fix14439.html
                  AX25UI digiUi(*ui);
                  digiUi.setToDigiCall(CallSign + "*", digiindex);           // Add digipeated.
                  int digissid = nextDigi.substring(index + 1).toInt() - 1;  // SSID decrement
                  if (digissid > 0) {
                    digiUi.addToDigiCall(digicall + "-" + digissid, digiindex + 1);
                  }
                  TXQueue.push_front(digiUi);
                }
              }
            }
          }
        }
      }
    }
    isDo = true;

    // delete ui;
  }

  while (!TXQueue.empty() && PayloadTXQueue.empty()) {
    AX25UI ui(TXQueue.front());
    if (!ui.isNull()) {
      PayloadTXQueue.push_back(ui.Encode());
    }
    TXQueue.pop_front();
    isDo = true;
  }
  return isDo;
}

size_t AX25UITask::TXQueueSize() { return TXQueue.size() + PayloadTXQueue.size(); }
