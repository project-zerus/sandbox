#ifndef _sandbox_echo_echo_server_cpp_EchoHandler_h_
#define _sandbox_echo_echo_server_cpp_EchoHandler_h_

#include <iostream>
#include <string>

#include "sandbox/echo/echo-thrift-api/Echo.h"

namespace sandbox { namespace echo { namespace server {

using namespace sandbox::echo::thrift;

class EchoHandler : virtual public EchoIf {

public:
  void echo(std::string& _return, const std::string& msg);

};

}}}

#endif // _sandbox_echo_echo_server_cpp_EchoHandler_h_
