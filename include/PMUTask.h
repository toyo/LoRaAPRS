#include "boards.h"

#if defined(XPOWERS_CHIP_AXP192)


class PMUTask {
 public:
  bool setup();
  bool loop();
};

#endif
