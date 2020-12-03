#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// read data from the filesystem and return name of OS
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// read data from the filesystem and return Kernel identifier
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Returns line starting with name
string ParseForLine(string name, string dir) {
  string line;
  std::ifstream stream(dir);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      if (line.compare(0, name.length(), name) == 0) {
        return line;
      }
    }
  }
  return "";
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line, memtotal_name, memtotal_value;
  string memfree_name, memfree_value;
  line = ParseForLine("MemTotal", kProcDirectory + kMeminfoFilename);
  if (line.length()) {
    std::istringstream linestream(line);
    linestream >> memtotal_name >> memtotal_value;
  }

  line = ParseForLine("MemFree", kProcDirectory + kMeminfoFilename);
  if (line.length()) {
    std::istringstream linestream(line);
    linestream >> memfree_name >> memfree_value;
  }
  return stof(memfree_value) / stof(memtotal_value);
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  string result{"0"};
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> result;
  }
  return stol(result);
}

// Read and return the number of jiffies (clock ticks) for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  string line, value;
  long total{0};
  getline(stream, line);
  std::istringstream linestream(line);
  for (int i = 0; i < 22; i++) {
    linestream >> value;
    if (i >= 13 && i <= 16) {
      total += stol(value);
    }
  }
  return total;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpu_util = LinuxParser::CpuUtilization();
  long total{0};

  // add up all jiffies that aren't idle or guest
  // user + nice + system + irq + steal
  for (size_t i = 0; i < cpu_util.size(); i++) {
    if (i != 0 && i != 4 && i != 5 && i < 8) {
      total += stol(cpu_util[i]);
    }
  }
  return total;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> utilization = LinuxParser::CpuUtilization();
  return stol(utilization[4]) + stol(utilization[5]);  // idle + iowait
}

// Read and return CPU utilization line as vector of strings
// https://man7.org/linux/man-pages/man5/proc.5.html <- /proc/[pid]/stat
vector<string> LinuxParser::CpuUtilization() {
  string line = ParseForLine("cpu", kProcDirectory + kStatFilename);
  vector<string> result;
  std::istringstream linestream(line);
  string value;
  while (linestream >> value) {
    result.push_back(value);
  }
  return result;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, name, value;
  line = ParseForLine("processes", kProcDirectory + kStatFilename);
  if (line.length()) {
    std::istringstream linestream(line);
    linestream >> name >> value;
    return stoi(value);
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line, name, value;
  line = ParseForLine("procs_running", kProcDirectory + kStatFilename);
  if (line.length()) {
    std::istringstream linestream(line);
    linestream >> name >> value;
    return stoi(value);
  }
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  string line{""};
  if (stream.is_open()) {
    getline(stream, line);
  }
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string vm, size{""};
  string line =
      ParseForLine("VmSize", kProcDirectory + to_string(pid) + kStatusFilename);
  if (line.length()) {
    std::istringstream linestream(line);
    std::stringstream ss;
    linestream >> vm >> size;
    ss << std::fixed << std::setprecision(2) << stof(size) / 1000;
    size = ss.str();
  }
  return size;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string word, uid{""};
  string line =
      ParseForLine("Uid", kProcDirectory + to_string(pid) + kStatusFilename);
  if (line.length()) {
    std::istringstream linestream(line);
    linestream >> word >> uid;
  }
  return uid;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);
  string line, x, value, name;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> name >> x >> value;
      if (value == uid) {
        return name;
      }
    }
  }
  return string();
}

// Read and return the uptime of a process in seconds
long LinuxParser::UpTime(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    getline(stream, line);
    int c{0};
    string value;
    std::istringstream linestream(line);
    while (linestream >> value) {
      if (c == 21) {
        return LinuxParser::UpTime() - (stol(value)) / sysconf(_SC_CLK_TCK);
      }
      c++;
    }
  }
  return 0;
}
