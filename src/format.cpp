#include "format.h"

#include <string>

using std::string;
using std::to_string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  int hh = seconds / 3600;
  seconds %= 3600;

  int mm = seconds / 60;
  seconds %= 60;

  string MM = mm < 10 ? "0" + to_string(mm) : to_string(mm);
  string SS = seconds < 10 ? "0" + to_string(seconds) : to_string(seconds);
  return to_string(hh) + ":" + MM + ":" + SS;
}
