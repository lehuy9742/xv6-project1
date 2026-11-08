#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

// Tách một dòng thành các từ (theo khoảng trắng), nối vào sau phần
// tham số của lệnh, rồi fork + exec + wait.
void
runline(char **args, int base, char *line)
{
  int k = base;
  char *p = line;

  while(*p){
    while(*p == ' ' || *p == '\t')
      *p++ = 0;
    if(*p == 0)
      break;
    if(k >= MAXARG - 1){
      fprintf(2, "xargs: too many arguments\n");
      exit(1);
    }
    args[k++] = p;
    while(*p && *p != ' ' && *p != '\t')
      p++;
  }
  args[k] = 0;          // argv của exec phải kết thúc bằng con trỏ 0

  if(k == base)         // dòng rỗng: bỏ qua
    return;

  int pid = fork();
  if(pid < 0){
    fprintf(2, "xargs: fork failed\n");
    exit(1);
  }
  if(pid == 0){
    exec(args[0], args);
    fprintf(2, "xargs: exec %s failed\n", args[0]);
    exit(1);
  }
  wait(0);
}

int
main(int argc, char *argv[])
{
  char *args[MAXARG];
  char line[512];
  char c;
  int n = 0;
  int start = 1;
  int base = 0;
  int i;

  // Bỏ qua tùy chọn "-n 1" (xv6 xargs luôn chạy mỗi dòng một lần)
  if(argc > 2 && strcmp(argv[1], "-n") == 0)
    start = 3;

  if(argc <= start){
    fprintf(2, "usage: xargs [-n 1] command [args...]\n");
    exit(1);
  }

  for(i = start; i < argc; i++){
    if(base >= MAXARG - 1){
      fprintf(2, "xargs: too many arguments\n");
      exit(1);
    }
    args[base++] = argv[i];
  }

  // Đọc từng ký tự cho tới '\n'
  while(read(0, &c, 1) == 1){
    if(c == '\n'){
      line[n] = 0;
      runline(args, base, line);
      n = 0;
    } else if(n < (int)sizeof(line) - 1){
      line[n++] = c;
    } else {
      fprintf(2, "xargs: line too long\n");
      exit(1);
    }
  }

  // Dòng cuối không có '\n'
  if(n > 0){
    line[n] = 0;
    runline(args, base, line);
  }

  exit(0);
}