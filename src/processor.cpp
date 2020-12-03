#include "processor.h"

#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
  prev_idle_ = idle_;
  prev_non_idle_ = non_idle_;
  prev_total_ = total_;

  idle_ = LinuxParser::IdleJiffies();
  non_idle_ = LinuxParser::ActiveJiffies();
  total_ = idle_ + non_idle_;

  long change_total = total_ - prev_total_;
  long change_non_idle = non_idle_ - prev_non_idle_;
  return (float)change_non_idle / (float)change_total;
}
