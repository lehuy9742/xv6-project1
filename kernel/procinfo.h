#ifndef PROCINFO_H
#define PROCINFO_H

// Dùng chung cho kernel và user. Cần #include "kernel/types.h" (hoặc "types.h")
// trước file này để có kiểu uint64.
struct procinfo {
  int pid;        // Process ID
  int ppid;       // Parent process ID
  int state;      // Process state (xem enum procstate trong kernel/proc.h)
  uint64 sz;      // Size of process memory (bytes)
  char name[16];  // Process name
};

#endif