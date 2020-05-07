# IODash
Lightweight C++ I/O library

## Features
-  Fast & lightweight
-  RAII
-  Header only
-  Uses very little memory (1MB for 10k TCP conns on x86_64)
-  No more `Boost`ed

## Supported IO targets
-  Regular file (no async so far)
-  Socket (TCP, UDP, Unix, Pipe, SocketPair...)
-  Serial (/dev/tty*)
-  Timer (timerfd on Linux) (WIP)
-  ...more to come

## ToDos
-  Documentation!!!
-  Serial port class
-  `kqueue` support for BSDs
-  `io_uring` support for newer Linux

## Usage
```cpp
#include <IODash.hpp>

using namespace IODash;
```

```cpp
SocketAddress<AddressFamily::IPv4> v4_addr("127.0.0.1:8080");
v4_addr.from_string("0.0.0.0");
v4_addr.from_string("0.0.0.0:8888");
v4_addr.port() = 8080;
std::cout << v4_addr.to_string();

SocketAddress<AddressFamily::IPv6> v6_addr("[::1]:8888");
v6_addr.from_string("::");
v6_addr.from_string("[::]:8888");
v6_addr.port() = 8080;
std::cout << v6_addr.to_string();

std::unordered_set<SocketAddress<AddressFamily::IPv4>> addrs;
addrs.insert(v4_addr);
```

```cpp
Socket<AddressFamily::IPv4, SocketType::Datagram> socket0; // IPv4 UDP
socket0.create();
socket0.sendto({"127.0.0.1:9999"}, "abcde", 5);
```

For more examples, see `test.cpp` and `http_test.cpp`.

## Documentation
TBD

## License
MIT