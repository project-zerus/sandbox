
#include <assert.h>
#include <signal.h>
#include <unistd.h>

#include <sofa/pbrpc/pbrpc.h>

#include "sandbox/sofa/echo/echo_service.pb.h"

class EchoServerImpl : public sofa::pbrpc::test::EchoServer
{
public:
  EchoServerImpl() {}
  virtual ~EchoServerImpl() {}

private:
  virtual void Echo(
    google::protobuf::RpcController* controller,
    const sofa::pbrpc::test::EchoRequest* request,
    sofa::pbrpc::test::EchoResponse* response,
    google::protobuf::Closure* done) {
    auto sofaController = dynamic_cast<sofa::pbrpc::RpcController*>(controller);
    assert(nullptr != sofaController);
    SLOG(
      NOTICE,
      "Echo(): request message from %s: %s",
      sofaController->RemoteAddress().c_str(),
      request->message().c_str()
    );
    response->set_message("echo message: " + request->message());
    done->Run();
  }
};

bool thread_init_func()
{
  sleep(1);
  SLOG(INFO, "Init work thread succeed");
  return true;
}

void thread_dest_func()
{
  SLOG(INFO, "Destroy work thread succeed");
}

int main()
{
  SOFA_PBRPC_SET_LOG_LEVEL(WARNING);

  // Define an rpc server.
  sofa::pbrpc::RpcServerOptions options;
  options.work_thread_init_func = sofa::pbrpc::NewPermanentExtClosure(
    &thread_init_func
  );
  options.work_thread_dest_func = sofa::pbrpc::NewPermanentExtClosure(
    &thread_dest_func
  );
  sofa::pbrpc::RpcServer rpc_server(options);

  // Start rpc server.
  if (!rpc_server.Start("0.0.0.0:12321")) {
    SLOG(ERROR, "start server failed");
    return EXIT_FAILURE;
  }

  // Register service.
  sofa::pbrpc::test::EchoServer* echo_service = new EchoServerImpl();
  if (!rpc_server.RegisterService(echo_service)) {
    SLOG(ERROR, "export service failed");
    return EXIT_FAILURE;
  }

  // Wait signal.
  rpc_server.Run();

  // Stop rpc server.
  rpc_server.Stop();

  // Delete closures.
  // Attention: should delete the closures after server stopped.
  delete options.work_thread_init_func;
  delete options.work_thread_dest_func;

  return EXIT_SUCCESS;
}
