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

#include "IODash/EventLoop.hpp"
#include "IODash/File.hpp"
#include "IODash/Socket.hpp"
#include "IODash/Serial.hpp"
#include "IODash/Timer.hpp"
#include "IODash/SocketAddress.hpp"
#include "IODash/Buffer.hpp"
//#include "IODash/IOService.hpp"

namespace IODash {
	template <auto T>
	inline std::string to_string(const SocketAddress<T> &__addr) {
		switch (__addr.family()) {
			case AddressFamily::IPv4:
				return ((SocketAddress<AddressFamily::IPv4> *)&__addr)->to_string();
			case AddressFamily::IPv6:
				return ((SocketAddress<AddressFamily::IPv6> *)&__addr)->to_string();
			case AddressFamily::Unix:
				return std::string(((SocketAddress<AddressFamily::Unix> *)&__addr)->to_string());
			default:
				return {};
		}
	}
}