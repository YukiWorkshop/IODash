# IODash
Lightweight C++ I/O library

## Features
-  Fast & lightweight
-  Header only
-  Uses very little memory (512KB for 10k TCP conns on x86_64)
-  No more `Boost`ed - doesn't abuse `shared_ptr`s

## Supported IO targets
-  Regular file (no async so far)
-  Socket (TCP, UDP, Unix, Pipe, SocketPair...)
-  Serial (/dev/tty*)
-  Timer (timerfd on Linux)
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

```cpp
Socket<AddressFamily::IPv6, SocketType::Stream> socket1; // IPv6 TCP

socket1.create();
socket1.listen({"[::]:8888"});

EventLoop<EventBackend::EPoll> event_loop;
event_loop.add(socket1);

event_loop.on_event(EventType::In, [](EventLoop<EventBackend::Any>& event_loop, const File& so, EventType ev, int& userdata){
    auto cur_socket = static_cast<const Socket<AddressFamily::IPv6, SocketType::Stream>&>(so);

    std::cout << "FD " << cur_socket.fd() << " In event\n";

    if (cur_socket.listening()) {
        Socket<AddressFamily::IPv6, SocketType::Stream> client_socket;
        cur_socket.accept(client_socket);
        std::cout << "New client: " << client_socket.remote_address().to_string() << "\n";
        event_loop.add(client_socket, EventType::In);
    } else {
        char buf[128];
        auto rc = cur_socket.recv(buf, 128);
        if (rc > 0)
            std::cout << "Read " << rc << " bytes from client " << cur_socket.remote_address().to_string() << "\n";
        else {
            event_loop.del(cur_socket);
            cur_socket.close();
        }
    }


});

event_loop.run();
```

## Documentation
TBD

## License
MIT