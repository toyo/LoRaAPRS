#ifndef AX25UITask_h
#define AX25UITask_h

#include <set>

#include "AX25UI.h"
#include "Payload.h"

class AX25UITask {
  QueueHandle_t &PayloadRXQ;
  QueueHandle_t &PayloadTXQ;
  QueueHandle_t &AX25UIRXQ;
  QueueHandle_t &AX25UITXQ;

  String CallSign;
  bool digipeat;

  std::set<String> UITRACE;  // like "WIDE1"

 public:
  AX25UITask() = delete;
  AX25UITask(QueueHandle_t &_PayloadRXQ, QueueHandle_t &_PayloadTXQ, QueueHandle_t &_AX25UIRXQ,
             QueueHandle_t &_AX25UITXQ)
      : PayloadRXQ(_PayloadRXQ), PayloadTXQ(_PayloadTXQ), AX25UIRXQ(_AX25UIRXQ), AX25UITXQ(_AX25UITXQ) {}

  void setCallSign(String _CallSign, bool _digipeat = true);

  bool setup();
  void addUITRACE(String _UITRACE);

  void taskRX(portTickType xBlockTime);
  bool loopTX();

  size_t TXQueueSize();
};
#endif