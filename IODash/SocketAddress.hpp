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

#include <string>
#include <tuple>
#include <memory>
#include <vector>

#include <cinttypes>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

namespace IODash {
	enum class AddressFamily : uint16_t {
		Any = AF_UNSPEC, Unix = AF_UNIX, IPv4 = AF_INET, IPv6 = AF_INET6
	};

	template<AddressFamily AF>
	class SocketAddress {
	protected:
		union sa_any {
			sockaddr s;
			sockaddr_in in;
			sockaddr_in6 in6;
			sockaddr_un un;
		} sa;

	public:
		SocketAddress() {

		}

		virtual ~SocketAddress() {

		}

		void reset() noexcept {
			memset(&sa, 0, sizeof(sa));
		}

		auto family() const noexcept {
			struct {
				sa_any *sa;

				operator AddressFamily() {
					return (AddressFamily)sa->s.sa_family;
				}

				AddressFamily operator=(AddressFamily af) {
					sa->s.sa_family = (uint16_t)af;
					return af;
				}
			} ret{(sa_any *)(&sa)};

			return ret;
		}

		virtual size_t size() const noexcept {
			return sizeof(sockaddr_storage);
		}

		sockaddr *raw() noexcept {
			return &sa.s;
		}

		const sockaddr *raw() const noexcept {
			return reinterpret_cast<const sockaddr *>(&sa);
		}

		std::vector<uint8_t> serialize() const {
			std::vector<uint8_t> ret;

			if (family() == AddressFamily::IPv4) {
				ret.resize(sizeof(sockaddr_in));
				memcpy(ret.data(), sa.in, sizeof(sockaddr_in));
			} else if (family() == AddressFamily::IPv6) {
				ret.resize(sizeof(sockaddr_in6));
				memcpy(ret.data(), sa.in6, sizeof(sockaddr_in6));
			} else if (family() == AddressFamily::Unix) {
				ret.resize(sizeof(sockaddr_un));
				memcpy(ret.data(), sa.un, sizeof(sockaddr_un));
			}

			return ret;
		}

		void deserialize(const std::vector<uint8_t>& data) {
			if (data.size() > sizeof(sa)) {
				throw std::logic_error("SockAddress: deserialize: data too large");
			}

			memcpy(sa, data.data(), data.size());
		}

		std::string to_string(bool __with_port = true) const {
			if (family() == AddressFamily::IPv4) {
				return as_ipv4()->to_string(__with_port);
			} else if (family() == AddressFamily::IPv6) {
				return as_ipv6()->to_string(__with_port);
			} else if (family() == AddressFamily::Unix) {
				return std::string(as_unix()->to_string());
			} else {
				return {};
			}
		}

		SocketAddress<AddressFamily::IPv4>* as_ipv4() noexcept {
			return (SocketAddress<AddressFamily::IPv4>*)this;
		}

		SocketAddress<AddressFamily::IPv6>* as_ipv6() noexcept {
			return (SocketAddress<AddressFamily::IPv6>*)this;
		}

		SocketAddress<AddressFamily::Unix>* as_unix() noexcept {
			return (SocketAddress<AddressFamily::Unix>*)this;
		}

		SocketAddress<AddressFamily::IPv4>* as_ipv4() const noexcept {
			return (SocketAddress<AddressFamily::IPv4>*)this;
		}

		SocketAddress<AddressFamily::IPv6>* as_ipv6() const noexcept {
			return (SocketAddress<AddressFamily::IPv6>*)this;
		}

		SocketAddress<AddressFamily::Unix>* as_unix() const noexcept {
			return (SocketAddress<AddressFamily::Unix>*)this;
		}
	};

	template<>
	class SocketAddress<AddressFamily::IPv4> : public SocketAddress<AddressFamily::Any> {
	public:
		SocketAddress() {
			reset();
		}

		SocketAddress(sockaddr_in *__sa) : SocketAddress() {
			sa.in.sin_port = __sa->sin_port;
			sa.in.sin_addr.s_addr = sa.in.sin_addr.s_addr;
		}

		SocketAddress(const std::string &__addr_str) : SocketAddress() {
			from_string(__addr_str);
		}

		void reset() noexcept {
			memset(&sa.in, 0, sizeof(sockaddr_in));
			sa.in.sin_family = AF_INET;
		}

		virtual size_t size() const noexcept override {
			return sizeof(sockaddr_in);
		}

		auto port() noexcept {
			struct {
				sockaddr_in *p;

				operator uint16_t() {
					return ntohs(p->sin_port);
				}

				uint16_t operator=(uint16_t s) {
					p->sin_port = htons(s);
					return s;
				}
			} ret{reinterpret_cast<sockaddr_in *>(&sa)};

			return ret;
		}

		uint16_t port() const noexcept {
			return ntohs(sa.in.sin_port);
		}

		std::string to_string(bool __with_port = true) const {
			char buf[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &sa.in.sin_addr, buf, INET_ADDRSTRLEN);

			std::string ret = buf;
			if (__with_port)
				ret += ":" + std::to_string(port());
			return ret;
		}

		void from_string(const std::string &__addr_str) {
			std::size_t p = __addr_str.rfind(':');

			if (p != std::string::npos) {
				std::string buf = __addr_str;
				buf[p] = 0;
				if (p + 1 < buf.size())
					port() = strtol(buf.data() + p + 1, nullptr, 10);
				inet_pton(AF_INET, buf.c_str(), &sa.in.sin_addr);
			} else {
				inet_pton(AF_INET, __addr_str.c_str(), &sa.in.sin_addr);
			}


		}

		friend inline bool operator<(const SocketAddress &lhs, const SocketAddress &rhs) {
			return std::tie(lhs.sa.in.sin_addr.s_addr, lhs.sa.in.sin_port) <
			       std::tie(rhs.sa.in.sin_addr.s_addr, rhs.sa.in.sin_port);
		}

		friend inline bool operator==(const SocketAddress &lhs, const SocketAddress &rhs) {
			return memcmp(&lhs.sa, &rhs.sa, sizeof(sockaddr_in)) == 0;
		}

		friend inline bool operator!=(const SocketAddress &lhs, const SocketAddress &rhs) {
			return !(lhs == rhs);
		}


	};

	template<>
	class SocketAddress<AddressFamily::IPv6> : public SocketAddress<AddressFamily::Any> {
	public:
		SocketAddress() {
			reset();
		}

		SocketAddress(sockaddr_in6 *__sa) : SocketAddress() {
			sa.in6.sin6_port = __sa->sin6_port;
			sa.in6.sin6_flowinfo = __sa->sin6_flowinfo;
			sa.in6.sin6_scope_id = __sa->sin6_scope_id;
			memcpy(&sa.in6.sin6_addr, &__sa->sin6_addr, sizeof(in6_addr));
		}

		SocketAddress(const std::string &__addr_str) : SocketAddress() {
			from_string(__addr_str);
		}

		void reset() noexcept {
			memset(&sa.in6, 0, sizeof(sockaddr_in6));
			sa.in6.sin6_family = AF_INET6;
		}

		virtual size_t size() const noexcept override {
			return sizeof(sockaddr_in6);
		}

		auto port() noexcept {
			struct {
				sockaddr_in6 *p;

				operator uint16_t() {
					return ntohs(p->sin6_port);
				}

				uint16_t operator=(uint16_t s) {
					p->sin6_port = htons(s);
					return s;
				}
			} ret{reinterpret_cast<sockaddr_in6 *>(&sa)};

			return ret;
		}

		uint16_t port() const noexcept {
			return ntohs(sa.in6.sin6_port);
		}

		std::string to_string(bool __with_port = true) const {
			char buf[INET6_ADDRSTRLEN + 2 + 1 + 5];

			if (__with_port) {
				buf[0] = '[';
				inet_ntop(AF_INET6, &sa.in6.sin6_addr, buf + 1, INET6_ADDRSTRLEN);
				auto epos = strlen(buf);
				sprintf(buf + epos, "]:%u", port());
			} else {
				inet_ntop(AF_INET6, &sa.in6.sin6_addr, buf, INET6_ADDRSTRLEN);
			}

			return buf;
		}

		void from_string(const std::string &__addr_str) {
			std::size_t p = __addr_str.rfind(']');

			if (p != std::string::npos) {
				std::string buf = __addr_str;
				buf[p] = 0;
				if (p + 2 < buf.size())
					port() = strtol(buf.data() + p + 2, nullptr, 10);
				inet_pton(AF_INET6, buf.data() + 1, &sa.in6.sin6_addr);
			} else {
				inet_pton(AF_INET6, __addr_str.c_str(), &sa.in6.sin6_addr);
			}


		}

		friend inline bool operator<(const SocketAddress &lhs, const SocketAddress &rhs) {
			auto *a1 = (uint8_t *) &lhs.sa.in6.sin6_addr;
			auto *a2 = (uint8_t *) &rhs.sa.in6.sin6_addr;

			return std::tie(a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], a1[6], a1[7],
					a1[8], a1[9], a1[10], a1[11], a1[12], a1[13], a1[14], a1[15],
					lhs.sa.in6.sin6_port) <
			       std::tie(a2[0], a2[1], a2[2], a2[3], a2[4], a2[5], a2[6], a2[7],
					a2[8], a2[9], a2[10], a2[11], a2[12], a2[13], a2[14], a2[15],
					rhs.sa.in6.sin6_port);
		}

		friend inline bool operator==(const SocketAddress &lhs, const SocketAddress &rhs) {
			return memcmp(&lhs.sa, &rhs.sa, sizeof(sockaddr_in)) == 0;
		}

		friend inline bool operator!=(const SocketAddress &lhs, const SocketAddress &rhs) {
			return !(lhs == rhs);
		}
	};

	template<>
	class SocketAddress<AddressFamily::Unix> : public SocketAddress<AddressFamily::Any> {
	public:
		SocketAddress() {
			reset();
		}

		SocketAddress(sockaddr_un *__sa) : SocketAddress() {
			strcpy(sa.un.sun_path, __sa->sun_path);
		}

		SocketAddress(const std::string &__addr_str) : SocketAddress() {
			from_string(__addr_str);
		}

		void reset() noexcept {
			memset(&sa.un, 0, sizeof(sockaddr_un));
			sa.un.sun_family = AF_UNIX;
		}

		virtual size_t size() const noexcept override {
			return sizeof(sockaddr_un);
		}

		const std::string_view to_string() const {
			return sa.un.sun_path;
		}

		void from_string(const std::string &__addr_str) {
			strncpy(sa.un.sun_path, __addr_str.c_str(), sizeof(sa.un.sun_path) - 1);
		}

		friend inline bool operator<(const SocketAddress &lhs, const SocketAddress &rhs) {
			std::string_view a(lhs.sa.un.sun_path), b(rhs.sa.un.sun_path);

			return a < b;
		}

		friend inline bool operator==(const SocketAddress &lhs, const SocketAddress &rhs) {
			return memcmp(&lhs.sa, &rhs.sa, sizeof(sockaddr_in)) == 0;
		}

		friend inline bool operator!=(const SocketAddress &lhs, const SocketAddress &rhs) {
			return !(lhs == rhs);
		}
	};

}

namespace std {
	template <>
	struct hash<IODash::SocketAddress<IODash::AddressFamily::IPv4>> {
		std::size_t operator()(const IODash::SocketAddress<IODash::AddressFamily::IPv4>& k) const {
			using std::size_t;
			using std::hash;

			return ((hash<uint32_t>()(((sockaddr_in *)k.raw())->sin_addr.s_addr)
				 ^ (hash<uint16_t>()(k.port()) << 1)) >> 1);
		}
	};

	template <>
	struct hash<IODash::SocketAddress<IODash::AddressFamily::IPv6>> {
		std::size_t operator()(const IODash::SocketAddress<IODash::AddressFamily::IPv6>& k) const {
			using std::size_t;
			using std::hash;

			auto *a = (uint64_t *)&((sockaddr_in6 *)k.raw())->sin6_addr;

			return ((hash<uint64_t>()(a[0])
				 ^ (hash<uint64_t>()(a[1]) << 1)) >> 1)
			       ^ (hash<uint16_t>()(k.port()) << 1);
		}
	};

	template <>
	struct hash<IODash::SocketAddress<IODash::AddressFamily::Unix>> {
		std::size_t operator()(const IODash::SocketAddress<IODash::AddressFamily::Unix>& k) const {
			using std::size_t;
			using std::hash;

			return hash<std::string_view>()(((sockaddr_un *)k.raw())->sun_path);
		}
	};
}