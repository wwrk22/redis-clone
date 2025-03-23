#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>


static void msg(const char *msg) {
  fprintf(stderr, "%s\n", msg);
}

static void die(const char* msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

static void do_something(int connfd) {
  char rbuf[64] = {};
  ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    msg("read() error");
    return;
  }
  fprintf(stderr, "client says: %s\n", rbuf);

  char wbuf[] = "world";
  write(connfd, wbuf, strlen(wbuf));
}

int main(int argc, char* argv[]) {
  // AF_INET is for IPv4. Use AF_INET6 for IPv6.
  // SOCK_STREAM is for TCP. Use SOCK_DGRAM for UDP.
  // With AF_INET and SOCK_STREAM, protocol 0 uses TCP by default.
  // With AF_INET and SOCK_DGRAM, protocol 0 uses UDP by default.
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket()");
  }

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
  if (rv) {
    die("bind()");
  }

  // Socket is actually created after call to listen()
  // Now the socket is setup to passively listen so that it can accept incoming connections
  // A queue is created for incoming connection requests from clients
  rv = listen(fd, SOMAXCONN);
  if (rv) {
    die("listen()");
  }

  while (true) {
    struct sockaddr_in client_addr = {};
    socklen_t addrlen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr*)&client_addr, &addrlen);
    if (connfd < 0) {
      continue;
    }

    do_something(connfd);
    close(connfd);
  }
}
