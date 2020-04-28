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

using namespace IODash;

int main() {

	std::cout << sizeof(SocketAddress<AddressFamily::Any>) << "," << sizeof(sockaddr) << "\n";
	std::cout << sizeof(SocketAddress<AddressFamily::IPv4>) << "," << sizeof(sockaddr_in)  << "\n";

	SocketAddress<AddressFamily::IPv4> s("127.0.0.1:8080");

	s.from_string("1:6");
	s.from_string(":6");
	s.from_string("::::::");
	s.from_string("");
	s.from_string(":");
	s.from_string("1.:");



	std::cout << s.to_string() << "\n";

	auto s2 = s;

	std::unordered_set<SocketAddress<AddressFamily::IPv4>> sss;

	sss.insert(s);
	sss.insert(s2);

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


	Socket<AddressFamily::IPv4, SocketType::Datagram> socket0;
	socket0.create();

	socket0.sendto({"127.0.0.1:9999"}, "123", 3);


	Socket<AddressFamily::IPv6, SocketType::Stream> socket1;

	socket1.create();
	socket1.listen({"[::]:8888"});

	EventLoop<EventBackend::Poll> event_loop;
	event_loop.add(socket1);

	event_loop.on_event(EventType::In, [](EventLoop<EventBackend::Any>& event_loop, File& so, EventType ev, int& userdata){
		auto cur_socket = static_cast<Socket<AddressFamily::IPv6, SocketType::Stream>&>(so);

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
}