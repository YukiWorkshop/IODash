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

		File(const File& o) {
			puts("copy-constructed\n");
			if (fd_ >= 0)
				fd_ = dup(o.fd_);
		}

		File(File&& o) {
			puts("move-constructed\n");
			fd_ = o.fd_;
			o.fd_ = -1;
		}

		File& operator=(const File& o) {
			puts("copy-assigned\n");
			if (fd_ >= 0)
				fd_ = dup(o.fd_);
		}

		int fd() const noexcept {
			return fd_;
		}

		int &fd() noexcept {
			return fd_;
		}

		void open(const std::string& __path, int __mode = O_RDWR) {
			fd_ = ::open(__path.c_str(), __mode);

			if (fd_ < 0)
				throw std::system_error(errno, std::system_category(), "failed to open");
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

		ssize_t putc(uint8_t __c) {
			return write(&__c, 1);
		}

		int getc() {
			uint8_t c;
			int rc = read(&c, 1);
			if (rc == 1)
				return c;
			else
				return rc;
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