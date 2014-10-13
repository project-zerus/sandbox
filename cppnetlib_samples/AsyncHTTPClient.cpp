
#include <iostream>
#include <memory>
#include <thread>

#include <boost/range.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/network/protocol/http.hpp>

int main(int argc, char** argv) {
  using boost::asio::io_service;
  using boost::iterator_range;
  using boost::network::http::client;
  using boost::system::error_code;
  std::thread::id threadID = std::this_thread::get_id();
  std::cout << "Main thread #" << threadID << std::endl;
  boost::shared_ptr<io_service> ioService(new io_service());
  std::thread ioThread(
    [ioService] () {
      std::thread::id threadID = std::this_thread::get_id();
      std::cout << "IO thread #" << threadID << std::endl;
      try {
        boost::asio::io_service::work sentinel(*ioService);
        ioService->run();
      } catch (std::exception& e) {
        std::cout << "Exception in ioServiceRun: " << e.what();
      }
    }
  );
  client::options options;
  options.io_service(ioService);
  client httpClient(options);
  client::request request("http://z1.huahang.im/");
  client::response response = httpClient.get(
    request,
    [&httpClient, ioService]
    (iterator_range<char const*> const& range, error_code const& code) {
      std::thread::id threadID = std::this_thread::get_id();
      std::cout << "Callback thread #" << threadID << std::endl;
      if (code == boost::system::errc::success) {
        std::cout << ">>>>> DATA" << std::endl;
        for (uint64_t i = 0; i < range.size(); ++i) {
          std::cout << range[i];
        }
        std::cout << std::endl;
      } else if (code == boost::asio::error::misc_errors::eof) {
        std::cout << ">>>>> EOF" << std::endl;
        ioService->stop();
      } else {
        std::cout << ">>>>> ERROR " << code << std::endl;
      }
    }
  );
  auto status = boost::network::http::status(response);
  std::cout << ">>>>> REQUEST status " << status << std::endl;
  ioThread.join();
  std::cout << ">>>>> DONE" << std::endl;
  return 0;
}
