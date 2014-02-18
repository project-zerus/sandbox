
#include "EchoHandler.h"

namespace sandbox { namespace echo { namespace server {

using namespace sandbox::echo::thrift;

void EchoHandler::echo(std::string& _return, const std::string& msg) {
  _return = msg;
}

}}}
