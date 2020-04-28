/*
    This file is part of IODash.
    Copyright (C) 2020 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#pragma once

#include <vector>
#include <system_error>

#include <unistd.h>
#include <fcntl.h>

#include "SocketAddress.hpp"
#include "File.hpp"

namespace IODash {

	enum class SocketType : uint16_t {
		Any = 0, Stream = SOCK_STREAM, Datagram = SOCK_DGRAM, SeqPacket = SOCK_SEQPACKET
	};

	template<AddressFamily AF, SocketType ST>
	class Socket : public File {
	public:
		Socket() = default;


		Socket(int __fd) : File(__fd) {

		}

		virtual ~Socket() {
//			close();
		}

		using File::fd;

		bool listening() const noexcept {
			return listening_;
		}

		bool connected() const noexcept {
			return connected_;
		}

		SocketAddress<AF> local_address() noexcept {
			SocketAddress<AF> ret;
			socklen_t sz = ret.size();
			if (getsockname(fd_, ret.raw(), &sz))
				throw std::system_error(errno, std::system_category(), "failed to get local address");

			return ret;
		}

		SocketAddress<AF> remote_address() noexcept {
			SocketAddress<AF> ret;
			socklen_t sz = ret.size();
			if (getpeername(fd_, ret.raw(), &sz))
				throw std::system_error(errno, std::system_category(), "failed to get remote address");

			return ret;
		}

		void create() {
			close();

			fd_ = socket((int)AF, (int)ST, 0);

			if (fd_ < 0)
				throw std::system_error(errno, std::system_category(), "failed to create socket");
		}

		void listen(const SocketAddress<AF>& __addr, int __backlog = 256) {
			if (::bind(fd_, __addr.raw(), __addr.size()))
				throw std::system_error(errno, std::system_category(), "failed to bind socket");

			if (::listen(fd_, __backlog))
				throw std::system_error(errno, std::system_category(), "failed to listen on socket");

			listening_ = true;
		}

		bool connect(const SocketAddress<AF>& __addr) {
			bool rc = ::connect(fd_, __addr.raw(), __addr.size()) == 0;
			if (rc)
				connected_ = true;

			return rc;
		}

		bool accept(Socket<AF, ST>& __new_socket) {
			int newfd = ::accept(fd_, nullptr, nullptr);

			if (newfd > 0) {
				__new_socket.fd_ = newfd;
				__new_socket.connected_ = true;
				return true;
			} else {
				return false;
			}
		}

		ssize_t send(const void *__buf, size_t __len, int __flags = 0) {
			return ::send(fd_, __buf, __len, __flags);
		}

		ssize_t sendto(const SocketAddress<AF>& __addr, const void *__buf, size_t __len, int __flags = 0) {
			return ::sendto(fd_, __buf, __len, __flags, __addr.raw(), __addr.size());
		}

		ssize_t recv(void *__buf, size_t __len, int __flags = 0) {
			return ::recv(fd_, __buf, __len, __flags);
		}

		ssize_t recvfrom(SocketAddress<AF>& __addr, void *__buf, size_t __len, int __flags = 0) {
			size_t sz = __addr.size();
			return ::recvfrom(fd_, __buf, __len, __flags, __addr.raw(), &sz);
		}
	};
}

namespace std {
	template<>
	struct hash<IODash::File> {
		std::size_t operator()(const IODash::File &k) const {
			using std::size_t;
			using std::hash;

			return hash<int>()(k.fd());
		}
	};
}