
#include <unistd.h>
#include <iostream>

#include <sofa/pbrpc/pbrpc.h>

#include "glog/logging.h"

int main() {
  SOFA_PBRPC_SET_LOG_LEVEL(DEBUG);
  SLOG(DEBUG, "debug");
  SLOG(TRACE, "trace");
  SLOG(INFO, "info");
  SLOG(NOTICE, "notice");
  SLOG(WARNING, "warning");
  SLOG(ERROR, "error");
  LOG(ERROR) << "error";
  LOG(WARNING) << "warning";
  LOG(INFO) << "info";
  return EXIT_SUCCESS;
}
