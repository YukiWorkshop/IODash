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
#include <system_error>
#include <unordered_map>

#include <sys/ioctl.h>

#ifdef __linux__
#include <asm-generic/termbits.h>
#else
#include <termios.h>
#endif

#include "File.hpp"

static const std::unordered_map<uint, uint> b2speed = {
	{B0, 0},
	{B50, 50},
	{B75, 75},
	{B110, 110},
	{B134, 134},
	{B150, 150},
	{B200, 200},
	{B300, 300},
	{B600, 600},
	{B1200, 1200},
	{B1800, 1800},
	{B2400, 2400},
	{B4800, 4800},
	{B9600, 9600},
	{B19200, 19200},
	{B38400, 38400},
	{B57600, 57600},
	{B115200, 115200},
	{B230400, 230400},
	{B460800, 460800},
	{B500000, 500000},
	{B576000, 576000},
	{B921600, 921600},
	{B1000000, 1000000},
	{B1152000, 1152000},
	{B1500000, 1500000},
	{B2000000, 2000000},
	{B2500000, 2500000},
	{B3000000, 3000000},
	{B3500000, 3500000},
	{B4000000, 4000000}
};

static const std::unordered_map<uint, uint> speed2b = {
	{0, B0},
	{50, B50},
	{75, B75},
	{110, B110},
	{134, B134},
	{150, B150},
	{200, B200},
	{300, B300},
	{600, B600},
	{1200, B1200},
	{1800, B1800},
	{2400, B2400},
	{4800, B4800},
	{9600, B9600},
	{19200, B19200},
	{38400, B38400},
	{57600, B57600},
	{115200, B115200},
	{230400, B230400},
	{460800, B460800},
	{500000, B500000},
	{576000, B576000},
	{921600, B921600},
	{1000000, B1000000},
	{1152000, B1152000},
	{1500000, B1500000},
	{2000000, B2000000},
	{2500000, B2500000},
	{3000000, B3000000},
	{3500000, B3500000},
	{4000000, B4000000}
};

namespace IODash {

	enum class SerialParity : uint8_t {
		None, Even, Odd
	};

	class Serial : public File {
	private:
		void __tcgets(termios &__tio) {
			if (ioctl(fd_, TCGETS, &__tio))
				throw std::system_error(errno, std::system_category(), "TCGETS");
		}

		void __tcsets(termios &__tio) {
			if (ioctl(fd_, TCSETS, &__tio))
				throw std::system_error(errno, std::system_category(), "TCSETS");
		}

#ifdef __linux__
		void __tcgets2(termios2 &__tio) {
			if (ioctl(fd_, TCGETS2, &__tio))
				throw std::system_error(errno, std::system_category(), "TCGETS2");
		}

		void __tcsets2(termios2 &__tio) {
			if (ioctl(fd_, TCSETS2, &__tio))
				throw std::system_error(errno, std::system_category(), "TCSETS2");
		}
#endif

		void __set_speed(uint __speed) {
			auto it = speed2b.find(__speed);

			if (it != speed2b.end()) {
				struct termios tio;

				__tcgets(tio);
				tio.c_cflag &= ~CBAUD;
				tio.c_cflag |= it->second;
				__tcsets(tio);
			} else {
#ifdef __linux__
				struct termios2 tio;

				__tcgets2(tio);
				tio.c_cflag &= ~CBAUD;
				tio.c_cflag |= BOTHER;
				tio.c_ispeed = __speed;
				tio.c_ospeed = __speed;
				__tcsets2(tio);
#else
				throw std::logic_error("Non standard baud rates are only supported on Linux");

#endif
			}

		}

		uint __get_speed() {
#ifdef __linux__
			struct termios2 tio2;

			__tcgets2(tio2);

			int B = tio2.c_cflag & CBAUD;

			if (B == BOTHER) {
				return tio2.c_ospeed;
			}

			auto it = b2speed.find(B);

			if (it != b2speed.end()) {
				return it->second;
			} else {
				return 0;
			}
#else
			struct termios tio;

			if (ioctl(fd_, TCGETS, &tio))
				throw std::system_error(errno, std::system_category(), "TCGETS");

			auto it = b2speed.find(tio.c_cflag & CBAUD);

			if (it != b2speed.end()) {
				return it->second;
			} else {
				return 0;
			}
#endif
		}

	public:
		using File::set_nonblocking;

		auto speed() {
			struct {
				Serial *p;

				operator uint() {
					return p->__get_speed();
				}

				uint operator=(uint __speed) {
					p->__set_speed(__speed);
					return __speed;
				}
			} ret{this};

			return ret;
		}

		auto parity() {
			struct {
				Serial *p;

				operator SerialParity() {
					struct termios tio;

					p->__tcgets(tio);

					if (tio.c_cflag & PARENB) {
						if (tio.c_cflag & PARODD)
							return SerialParity::Odd;
						else
							return SerialParity::Even;
					} else {
						return SerialParity::None;
					}
				}

				SerialParity operator=(SerialParity __parity) {
					struct termios tio;

					p->__tcgets(tio);

					switch (__parity) {
						case SerialParity::None:
							tio.c_cflag &= ~PARENB;
							break;
						case SerialParity::Odd:
							tio.c_cflag |= PARENB;
							tio.c_cflag |= PARODD;
							break;
						case SerialParity::Even:
							tio.c_cflag |= PARENB;
							tio.c_cflag &= ~PARODD;
							break;
						default:
							break;
					}

					p->__tcsets(tio);

					return __parity;
				}
			} ret{this};

			return ret;
		}

		void make_raw() {
			struct termios tio;

			__tcgets(tio);
			tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
			tio.c_oflag &= ~OPOST;
			tio.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
			tio.c_cflag &= ~(CSIZE | PARENB);
			tio.c_cflag |= CS8;
			__tcsets(tio);
		}


	};
}