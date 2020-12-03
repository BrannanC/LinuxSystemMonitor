#ifndef PROCESS_H
#define PROCESS_H

#include <string>

class Process {
 public:
  int Pid();
  std::string User();
  std::string Command();
  float CpuUtilization();
  std::string Ram();
  long int UpTime();
  bool operator<(Process const& a) const;
  Process(int pid) : pid_(pid) { CpuUtilization(); };

 private:
  int pid_;
  std::string user_{""};
  std::string command_{""};
  float usage_;
};

#endif
