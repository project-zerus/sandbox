#include "glog/logging.h"

void funcA() {
  char* str = NULL;
  str[0] = 'a';
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  while (true);
  funcA();
  return 0;
}
