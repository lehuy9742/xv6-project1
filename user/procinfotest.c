#include "kernel/types.h"
#include "kernel/procinfo.h"
#include "user/user.h"

// Thứ tự phải khớp với enum procstate trong kernel/proc.h
static char *states[] = { "UNUSED", "USED", "SLEEPING", "RUNNABLE", "RUNNING", "ZOMBIE" };

static char *
statename(int s)
{
  if(s < 0 || s >= (int)(sizeof(states) / sizeof(states[0])))
    return "???";
  return states[s];
}

static int failures = 0;

static void
check(int ok, char *msg)
{
  printf("%s: %s\n", ok ? "PASS" : "FAIL", msg);
  if(!ok)
    failures++;
}

int
main(void)
{
  struct procinfo info;
  int pid = getpid();
  int before, c, st;
  failures = 0; 

  // 1. Tiến trình hiện tại
  check(procinfo(pid, &info) == 0, "procinfo(self) returns 0");
  printf("  pid=%d ppid=%d state=%s sz=%d name=%s\n",
         info.pid, info.ppid, statename(info.state), (int)info.sz, info.name);
  check(info.pid == pid, "pid matches getpid()");
  check(strcmp(statename(info.state), "RUNNING") == 0, "state of caller is RUNNING");
  check(info.sz > 0, "sz > 0");
  check(strcmp(info.name, "procinfotest") == 0, "name is procinfotest");

  // 2. sz tăng sau khi sbrk
  before = (int)info.sz;
  sbrk(4096);
  procinfo(pid, &info);
  check((int)info.sz == before + 4096, "sz grows by 4096 after sbrk(4096)");

  // 3. ppid của tiến trình con phải là pid của cha
  c = fork();
  if(c < 0){
    printf("fork failed\n");
    exit(1);
  }
  if(c == 0){
    struct procinfo ci;
    int ok = (procinfo(getpid(), &ci) == 0) && (ci.ppid == pid);
    exit(ok ? 0 : 1);
  }
  wait(&st);
  check(st == 0, "child sees ppid == parent's pid");

  // 4. Con đã exit nhưng cha chưa wait => ZOMBIE
  c = fork();
  if(c < 0){
    printf("fork failed\n");
    exit(1);
  }
  if(c == 0)
    exit(0);
  sleep(5);
  check(procinfo(c, &info) == 0 && strcmp(statename(info.state), "ZOMBIE") == 0,
        "exited-but-not-waited child is ZOMBIE");
  wait(0);
  check(procinfo(c, &info) == -1, "reaped child is no longer found");

  // 5. Tham số sai
  check(procinfo(99999, &info) == -1, "nonexistent pid returns -1");
  check(procinfo(pid, 0) == -1, "NULL pointer returns -1");
  check(procinfo(pid, (struct procinfo *)0x80000000L) == -1, "invalid address returns -1");

  if(failures == 0)
    printf("procinfotest: all tests passed\n");
  else
    printf("procinfotest: %d test(s) FAILED\n", failures);
  exit(failures ? 1 : 0);
}