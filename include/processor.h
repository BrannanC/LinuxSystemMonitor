#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  unsigned long long prev_idle_{0};
  unsigned long long idle_{0};
  unsigned long long prev_non_idle_{0};
  unsigned long long non_idle_{0};
  unsigned long long prev_total_{0};
  unsigned long long total_{0};
};

#endif
