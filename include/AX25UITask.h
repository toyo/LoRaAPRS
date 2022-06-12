#include <list>
#include <set>

#include "AX25UI.h"
#include "Payload.h"

class AX25UITask {
  std::list<Payload> &PayloadRXQueue;
  std::list<Payload> &PayloadTXQueue;
  String CallSign;
  bool digipeat;

  std::set<String> UITRACE;

 public:
  std::list<AX25UI> RXQueue;
  std::list<AX25UI> TXQueue;
  AX25UITask() = delete;
  AX25UITask(std::list<Payload> &_RXQueue, std::list<Payload> &_TXQueue)
      : PayloadRXQueue(_RXQueue), PayloadTXQueue(_TXQueue) {}

  void setCallSign(String _CallSign, bool _digipeat = true) {
    CallSign = _CallSign;
    digipeat = _digipeat;
  }

  bool setup() { return true; }
  void addUITRACE(String _UITRACE) {
    if (_UITRACE != "") {
      UITRACE.insert(_UITRACE);
    }
  }

  bool loop() {
    while (!PayloadRXQueue.empty()) {
      Payload recvd = PayloadRXQueue.front();
      AX25UI ui(recvd.getData(), recvd.getLen());
      if (!ui.isNull()) {
        RXQueue.push_back(ui);
        if (digipeat) {  // work as digi.
          if (!ui.findDigiCall(CallSign + "*") && ui.getFromCall() != CallSign) {
            // my call is not in digipeated list or sender.
            int digiindex;
            if ((digiindex = ui.findNextDigiIndex()) != -1) {  // All listed calls are not digipeated.
              String nextDigi = ui.getToDigiCalls(digiindex);  // Got next digipeater call
              if (nextDigi == CallSign) {                      // Next digipeater call is mine.
                AX25UI digiUi(ui);
                digiUi.setToDigiCall(CallSign + "*", digiindex);
                TXQueue.push_front(digiUi);
              } else {
                int index = nextDigi.indexOf('-');
                if (index != -1) {                                 // Has SSID
                  String digicall = nextDigi.substring(0, index);  // like "WIDE1"
                  if (std::find(UITRACE.begin(), UITRACE.end(), digicall) != UITRACE.end()) {
                    //   http://www.aprs.org/fix14439.html
                    AX25UI digiUi(ui);
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
      PayloadRXQueue.pop_front();
    }

    while (!TXQueue.empty() && PayloadTXQueue.empty()) {
      AX25UI ui = TXQueue.front();
      if (!ui.isNull()) {
        // PayloadTXQueue.push_back(Payload(ui.Encode()));
        PayloadTXQueue.push_back(ui.Encode());
      }
      TXQueue.pop_front();
    }
    return true;
  }

  size_t TXQueueSize() { return TXQueue.size() + PayloadTXQueue.size(); }
};