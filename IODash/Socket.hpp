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
	private:
		using File::open;
	public:
		Socket() = default;


		Socket(int __fd) : File(__fd) {

		}

		using File::set_nonblocking;

		SocketAddress<AF> local_address() {
			SocketAddress<AF> ret;
			socklen_t sz = ret.size();
			if (getsockname(fd_, ret.raw(), &sz))
				throw std::system_error(errno, std::system_category(), "failed to get local address");

			return ret;
		}

		SocketAddress<AF> remote_address() {
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

			refcounter.reset((int *)nullptr);
		}

		void listen(const SocketAddress<AF>& __addr, int __backlog = 256) {
			int enable = 1;
			if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)))
				throw std::system_error(errno, std::system_category(), "failed to setsockopt");

			if (::bind(fd_, __addr.raw(), __addr.size()))
				throw std::system_error(errno, std::system_category(), "failed to bind socket");

			if (::listen(fd_, __backlog))
				throw std::system_error(errno, std::system_category(), "failed to listen on socket");
		}

		bool connect(const SocketAddress<AF>& __addr) {
			bool rc = ::connect(fd_, __addr.raw(), __addr.size()) == 0;

			return rc;
		}

		void shutdown(int __how = SHUT_RDWR) {
			if (::shutdown(fd_, __how))
				throw std::system_error(errno, std::system_category(), "failed to shutdown socket");
		}

		Socket<AF, ST> accept() {
			int newfd = ::accept(fd_, nullptr, nullptr);
			return {newfd};
		}

		int setsockopt(int __level, int __optname, const void *__optval, socklen_t __optlen) {
			return ::setsockopt(fd_, __level, __optname, __optval, __optlen);
		}

		int getsockopt(int __level, int __optname, void *__optval, socklen_t *__optlen) {
			return ::getsockopt(fd_, __level, __optname, __optval, __optlen);
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

