/*
    This file is part of IODash.
    Copyright (C) 2020 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <IODash.hpp>

#include <iostream>
#include <unordered_set>
#include <cassert>

using namespace IODash;

const char http_reply[] = "HTTP/1.0 200 OK\r\n"
			  "Date: Thu, 07 May 2020 12:49:30 GMT\r\n"
			  "Connection: close\r\n"
			  "Accept-Ranges: bytes\r\n"
			  "Last-Modified: Thu, 07 May 2020 12:49:25 GMT\r\n"
			  "Content-Length: 0\r\n"
			  "\r\n";

int main() {
	Socket<AddressFamily::IPv4, SocketType::Stream> socket1;

	socket1.create();
	socket1.listen({"127.0.0.1:8082"});

	std::cout << "listening on: " << to_string(socket_cast<AddressFamily::Any, SocketType::Datagram>(socket1).local_address()) << "\n";

	struct user_data {
		bool is_listening_socket = false;
		uint32_t write_pos = 0;
	};

	EventLoop<EventBackend::EPoll, user_data> event_loop;
	event_loop.add(socket1, EventType::In, {true});

//	int fd = open("/dev/null", O_RDWR);
//	dup2(fd, STDOUT_FILENO);

	event_loop.on_event(EventType::In, [](auto& event_loop, File& so, EventType ev, auto& userdata){
		auto &cur_socket = socket_cast<AddressFamily::IPv4, SocketType::Stream>(so);

		try {
			if (userdata.is_listening_socket) {
				auto client_socket = cur_socket.accept();
				if (client_socket) {
//					auto rmt_addr = client_socket.remote_address();
//					std::cout << rmt_addr.to_string();
//					printf("New %s client: %s\n",
//					       rmt_addr.family() == AddressFamily::IPv4 ? "IPv4" : "IPv6",
//					       rmt_addr.to_string().c_str());
					event_loop.add(client_socket, EventType::In | EventType::Out);
				}
			} else {
				char buf[1024];
				auto rc = cur_socket.recv(buf, 1024);
				if (rc > 0) {
//					std::cout << "Read " << rc << " bytes from client "
//						  << cur_socket.remote_address().to_string() << "\n";
//					auto rmt_addr = cur_socket.remote_address();
//					std::cout << rmt_addr.to_string();

				} else {
					event_loop.del(cur_socket);
				}
			}
		} catch (...) {

		}
	});

	event_loop.on_event(EventType::Out, [](auto& event_loop, File& so, EventType ev, user_data& userdata){
		auto &cur_socket = socket_cast<AddressFamily::IPv6, SocketType::Stream>(so);

		ssize_t rc = cur_socket.write(http_reply+userdata.write_pos, sizeof(http_reply)-1-userdata.write_pos);

		try {
			if (rc > 0) {
				userdata.write_pos += rc;
				if (userdata.write_pos == sizeof(http_reply) - 1) {
					cur_socket.shutdown();
					event_loop.del(cur_socket);
				}
			} else {
				cur_socket.shutdown();
				event_loop.del(cur_socket);
			}
		} catch (...) {

		}
	});

	event_loop.run();
}