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

#include "File.hpp"

#include <optional>
#include <system_error>

#include <cstring>

#ifdef __linux__
#include <sys/timerfd.h>
#endif

namespace IODash {

#ifdef __linux__

	class Timer : public File {
	private:
		using File::read;
		using File::write;

	public:
		Timer(int __clockid = CLOCK_MONOTONIC, int __flags = 0) {
			fd_ = timerfd_create(__clockid, __flags);

			if (fd_ < 0)
				throw std::system_error(errno, std::system_category(), "timerfd_create");

			refcounter.reset((int *)nullptr);
		}

		void set_interval(double __seconds) {
			itimerspec tm;

			tm.it_value.tv_sec = tm.it_interval.tv_sec = __seconds;
			tm.it_value.tv_nsec = tm.it_interval.tv_nsec = ((__seconds - (double)tm.it_value.tv_sec) * 1000000000.0);

			if (timerfd_settime(fd_, 0, &tm, nullptr))
				throw std::system_error(errno, std::system_category(), "timerfd_settime");

		}

		void set_timeout(double __seconds) {
			itimerspec tm;

			tm.it_value.tv_sec = __seconds;
			tm.it_value.tv_nsec = ((__seconds - (double)tm.it_value.tv_sec) * 1000000000.0);

			memset(&tm.it_interval, 0, sizeof(struct timespec));

			if (timerfd_settime(fd_, 0, &tm, nullptr))
				throw std::system_error(errno, std::system_category(), "timerfd_settime");

		}

		void stop() {
			itimerspec tm;

			memset(&tm, 0, sizeof(struct itimerspec));

			if (timerfd_settime(fd_, 0, &tm, nullptr))
				throw std::system_error(errno, std::system_category(), "timerfd_settime");
		}

		std::optional<uint64_t> read() {
			uint64_t ret;

			if (File::read(&ret, sizeof(uint64_t)) == sizeof(uint64_t))
				return ret;
			else
				return {};
		}

	};

#endif

}