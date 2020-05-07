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


int main() {
	// It's easy
	SocketAddress<AddressFamily::IPv4> s("127.0.0.1:8080");

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

	// UDP sendto, easy
	Socket<AddressFamily::IPv4, SocketType::Datagram> socket0;
	socket0.create();
	socket0.sendto({"127.0.0.1:9999"}, "123", 3);


	// TCP server event loop
	Socket<AddressFamily::IPv6, SocketType::Stream> socket1;
	socket1.create();
	socket1.listen({"[::]:8888"});

	std::cout << "listening on: " << to_string(socket_cast<AddressFamily::Any, SocketType::Datagram>(socket1).local_address()) << "\n";

	struct user_data {
		bool is_listening_socket = false;
		int some_data;
	};

	EventLoop<EventBackend::EPoll, user_data> event_loop;
	event_loop.add(socket1, EventType::In, {true});

	event_loop.on_event(EventType::In, [](auto& event_loop, File& so, EventType ev, auto& userdata){
		auto &cur_socket = socket_cast<AddressFamily::IPv6, SocketType::Stream>(so);

		std::cout << "FD " << cur_socket.fd() << " In event\n";

		if (userdata.is_listening_socket) {
			auto client_socket = cur_socket.accept();
			if (client_socket) {
				std::cout << "New client " << client_socket.fd() << ": " << client_socket.remote_address().to_string() << "\n";
				event_loop.add(client_socket, EventType::In);
			}
		} else {
			char buf[128];
			auto rc = cur_socket.recv(buf, 128);
			if (rc > 0) {
				std::cout << "Read " << rc << " bytes from client "
					  << cur_socket.remote_address().to_string() << "\n";

				event_loop.modify(cur_socket, EventType::In, {false, 23333});
				// Or
				event_loop.modify(cur_socket, EventType::In);
				userdata.some_data = 23333;

			} else {
				event_loop.del(cur_socket);
			}
		}
	});

	event_loop.run();
}