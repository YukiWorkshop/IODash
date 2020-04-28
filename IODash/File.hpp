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

#include <unistd.h>
#include <fcntl.h>

namespace IODash {
	class File {
	protected:
		int fd_ = -1;
		bool connected_ = false, listening_ = false;

	public:
		File() = default;

		File(int __fd) : fd_(__fd) {

		}

		int fd() const noexcept {
			return fd_;
		}

		int &fd() noexcept {
			return fd_;
		}

		void close() noexcept {
			if (fd_ > 0) {
				::close(fd_);
				fd_ = -1;
				listening_ = false;
				connected_ = false;
			}
		}

		ssize_t write(const void *__buf, size_t __len) {
			return ::write(fd_, __buf, __len);
		}

		ssize_t read(void *__buf, size_t __len) {
			return ::read(fd_, __buf, __len);
		}

	};
}