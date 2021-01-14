/*
    This file is part of IODash.
    Copyright (C) 2020 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "IODash.hpp"

#include <iostream>
#include <unordered_set>
#include <cassert>

using namespace IODash;

const char some_static_data[] = "aaaaa";

int main() {
//	IOService iosvc;

	Socket<AddressFamily::IPv4, SocketType::Datagram> socket00;
	socket00.create();

//	iosvc.async_read(socket00, IODash::Buffer(1024), [](int err, size_t bytes_read, const Buffer& buf){
//
//	});

	auto iobuf0 = IODash::Buffer::reference(some_static_data);
	std::vector<uint8_t> some_generated_data(16);
	auto iobuf1 = IODash::Buffer::steal(some_generated_data);
	char *some_volatile_pod_type_data = (char *)alloca(16);
	auto iobuf2 = IODash::Buffer::copy(some_volatile_pod_type_data, 16);



	// It's easy
	SocketAddress<AddressFamily::IPv4> s("127.0.0.1:8080");

	assert(s.family() == AddressFamily::IPv4);

	// Some maliciously crafted strings
	s.from_string("1:6");
	s.from_string(":6");
	s.from_string("::::::");
	s.from_string("");
	s.from_string(":");
	s.from_string("1.:");

	// Shortcuts
	assert(s.as_ipv6()->port() == s.port());

	// Easy
	std::cout << s.to_string() << "\n";

	// Copy
	auto s2 = s;

	// Put them into a container that requires hash
	std::unordered_set<SocketAddress<AddressFamily::IPv4>> sss;

	sss.insert(s);
	sss.insert(s2);

	// Like the above
	SocketAddress<AddressFamily::IPv6> t;
	auto t2 = t;

	std::unordered_set<SocketAddress<AddressFamily::IPv6>> ttt;

	ttt.insert(t);

	t.from_string("");
	t.from_string("[");
	t.from_string("]");
	t.from_string("]:");
	t.from_string("[]::::::1]:1");
	t.from_string("[:");
	t.from_string("f");
	t.from_string("z");

	std::cout << t.to_string() << "\n";

	// socketpair
	auto sp = socket_pair<SocketType::Datagram>();
	sp.first.write("123", 4);
	char buf0[4];
	sp.second.read(buf0, 4);

	std::cout << "socketpair test: " << buf0 << "\n";

	int32_t sndbufsize = 1024 * 1024 * 10;
	socklen_t psst = 4;
	sp.first.setsockopt(SOL_SOCKET, SO_SNDBUF, &sndbufsize, psst);

	sp.first.getsockopt(SOL_SOCKET, SO_SNDBUF, &sndbufsize, &psst);
	std::cout << "snd buf size: " << sndbufsize << "\n";


	std::vector<uint8_t> largebuf(1024 * 1024, 'a');
	auto rc0 = sp.first.write(largebuf.data(), largebuf.size());
	std::cout << "socketpair test2: " << rc0 << "\n";
	std::cout << "socketpair test2: " << strerror(errno) << "\n";

	std::vector<uint8_t> data8(16);
	std::vector<uint16_t> data16(16);

	sp.first.write_all(data8);
	sp.first.write_all(data16);

	std::vector<uint8_t> largebuf2(1024 * 1024 * 10);
	auto rc = sp.second.read(largebuf2.data(), largebuf2.size());
	std::cout << "socketpair test2: " << rc << "\n";

	auto rd = sp.second.read_all(16);

	// UDP sendto, easy
	Socket<AddressFamily::IPv4, SocketType::Datagram> socket0;
	socket0.create();
	socket0.sendto({"127.0.0.1:9999"}, "123", 3);


	// TCP server event loop
	Socket<AddressFamily::IPv6, SocketType::Stream> socket1;
	socket1.create();
	socket1.bind({"[::]:8888"});
	socket1.listen();

	std::cout << "listening on: " << to_string(socket_cast<AddressFamily::Any, SocketType::Datagram>(socket1).local_address()) << "\n";

	enum {
		Listener, T1mer, Client
	};

	struct user_data {
		int type;
		int some_data;
	};
    
#ifdef __linux__
	std::cout << "sleeping for 1.5 secs...\n";
	Timer tmtm;
	tmtm.set_timeout(1.5);
	std::cout << *tmtm.read() << "\n";

	std::cout << "sleeping for 2 secs...\n";
	tmtm.set_timeout(2);
	std::cout << *tmtm.read() << "\n";

	std::cout << "Done sleeping!\n";

	tmtm.set_interval(1.5);
	tmtm.set_nonblocking();


	EventLoop<EventBackend::EPoll, user_data> event_loop;
	event_loop.add(socket1, EventType::In, {Listener});
	event_loop.add(tmtm, EventType::In, {T1mer});

	event_loop.on_events([](auto& event_loop, File& so, EventType ev, auto& userdata){
		auto &cur_socket = socket_cast<AddressFamily::IPv6, SocketType::Stream>(so);

		std::cout << "FD " << cur_socket.fd() << " In event " << (int)ev << "\n";

		if (userdata.type == Listener) {
			auto rc = cur_socket.accept();
			auto &client_socket = *rc;

			if (client_socket) {
				std::cout << "New client " << client_socket.fd() << ": " << client_socket.remote_address().to_string() << "\n";
				event_loop.add(client_socket, EventType::In, {Client});
			}
		} else if (userdata.type == Client) {
			char buf[128];
			auto rc = cur_socket.recv(buf, 128);
			if (rc > 0) {
				std::cout << "Read " << rc << " bytes from client "
					  << cur_socket.remote_address().to_string() << "\n";

				event_loop.modify(cur_socket, EventType::In, {Client, 23333});
				// Or
				event_loop.modify(cur_socket, EventType::In);
				userdata.some_data = 23333;

			} else {
				event_loop.del(cur_socket);
			}
		} else {
			std::cout << "Timer fires! " << *static_cast<Timer&>(so).read() << "\n";
			static_cast<Timer&>(so).stop();
		}
	});

	event_loop.run();
#endif
}
