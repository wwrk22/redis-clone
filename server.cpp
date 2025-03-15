#include <sys/socket.h>


int main(int argc, char* argv[]) {
  // AF_INET is for IPv4. Use AF_INET6 for IPv6.
  // SOCK_STREAM is for TCP. Use SOCK_DGRAM for UDP.
  // With AF_INET and SOCK_STREAM, protocol 0 uses TCP by default.
  // With AF_INET and SOCK_DGRAM, protocol 0 uses UDP by default.
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  // Set SO_REUSEADDR to 1. This tells the OS to allow reusing the address even if it's in TIME_WAIT
  // state. A connection goes into TIME_WAIT state right after it's closed.
  // SO_REUSEADDR accepts 0 or 1.
  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  struct sockaddr_in addr = {};
  // address family tells functions how to correctly interpret the rest of the data
  addr.sin_family = AF_INET;
  // htons converts the port to a big endian byte order 16-bit
  addr.sin_port = htons(1234);
  // htonl converts the address to a big endian byte order 32-bit
  addr.sin_addr.s_addr = htonl(0); // wildcard IP 0.0.0.0
  // `struct sockaddr` is actually not used anywhere. It's just used as the param type for `bind()`.
  // Simply cast `struct sockaddr_in` to `struct sockaddr`. The size, `addrlen`, is required by `bind()`
  // because different socket info structs have different sizes. For example, `struct sockaddr_in` and
  // `struct sockaddr_in6` have different sizes and can both be passed as the `struct sockaddr*` to `bind()`.
  int rv = bind(fd, (const struct sockaddr*)&addr, sizeof(addr));
  if (rv) { die("bind()"); }
}
