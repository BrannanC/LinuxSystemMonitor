#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization including child processes
float Process::CpuUtilization() {
  long jiffies = LinuxParser::ActiveJiffies(
      pid_);  // Total active time for process in clock ticks
  long active_time = jiffies / sysconf(_SC_CLK_TCK);  // Convert to seconds

  long uptime =
      LinuxParser::UpTime(pid_);  // Time the process has been up in seconds
  usage_ = (float)active_time / float(uptime);

  return usage_;
}

// Return the command that generated this process
string Process::Command() {
  if (command_ == "") {
    command_ = LinuxParser::Command(pid_);
  }
  return command_;
}

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// Return the user (name) that generated this process
string Process::User() {
  if (user_ == "") {
    user_ = LinuxParser::User(pid_);
  }
  return user_;
}

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const { return usage_ < a.usage_; }