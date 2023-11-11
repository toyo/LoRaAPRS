#ifndef AX25UITask_h
#define AX25UITask_h

#include <list>
#include <set>

#include "AX25UI.h"
#include "Payload.h"

class AX25UITask {
  QueueHandle_t &PayloadRXQ;
  std::list<Payload> &PayloadTXQueue;
  QueueHandle_t &RXQ;

  String CallSign;
  bool digipeat;

  std::set<String> UITRACE;

 public:
  std::list<AX25UI> TXQueue;
  AX25UITask() = delete;
  AX25UITask(QueueHandle_t &_PayloadRXQ, std::list<Payload> &_TXQueue, QueueHandle_t &_RXQ)
      : PayloadRXQ(_PayloadRXQ), PayloadTXQueue(_TXQueue), RXQ(_RXQ) {}

  void setCallSign(String _CallSign, bool _digipeat = true);

  bool setup();
  void addUITRACE(String _UITRACE);

  bool loop();

  size_t TXQueueSize();
};
#endif