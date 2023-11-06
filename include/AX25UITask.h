#ifndef AX25UITask_h
#define AX25UITask_h

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

  void setCallSign(String _CallSign, bool _digipeat = true);

  bool setup();
  void addUITRACE(String _UITRACE);

  bool loop();

  size_t TXQueueSize();
};
#endif