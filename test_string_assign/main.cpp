/**
 * @author huahang@xiaomi.com
 * @date 2014-10-10
 */

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  std::string bytes;
  bytes.assign("12345\0", 7);
  std::cout << bytes.size() << std::endl;
  return 0;
}
