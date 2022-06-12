#include <AX25UI.h>

AX25UI::AX25UI(const uint8_t* data, const size_t len) {
  char buffer[257];
  memcpy(buffer, data, len);
  buffer[len] = 0;

  char *saveptr, *retval;

  saveptr = nullptr;
  retval = strtok_r(buffer, ">", &saveptr);
  if (retval == nullptr) {
    return;
  }
  FromCall = retval;

  retval = strtok_r(nullptr, ":", &saveptr);
  if (retval == nullptr) {
    return;
  }

  char* p = retval;
  message = saveptr;

  saveptr = nullptr;
  while ((retval = strtok_r(p, ",", &saveptr)) != nullptr) {
    ToDigiCalls.push_back(retval);
    p = nullptr;
  }
}

AX25UI::AX25UI(String msg, String from, String to, String digi) {
  ToDigiCalls.erase(ToDigiCalls.begin(), ToDigiCalls.end());
  FromCall = from;
  ToDigiCalls.push_front(to);
  if (digi != "") {
    ToDigiCalls.push_back(digi);
  }
  message = msg;
}

String AX25UI::Encode() const {
  String s;
  if (FromCall != "") {
    s += FromCall + ">";

    for (std::deque<String>::const_iterator ite = ToDigiCalls.begin(); ite != ToDigiCalls.end(); ++ite) {
      s += *ite + ",";
    }
    s.remove(s.length() - 1);
    s += ":" + message;
  }

  return s;
}