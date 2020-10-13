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

#include <memory>
#include <functional>

#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "Buffer.hpp"

namespace IODash {
	enum class AsyncCapable : uint8_t {
		Unknown = 0, True = 1, False = 2
	};

	typedef std::function<void(int, Buffer)> ReadHandler;
	typedef std::function<void(int, size_t)> WriteHandler;
	typedef std::function<void(int )> AcceptHandler;

	class File {
	protected:
		int fd_ = -1;
		AsyncCapable async_capable_ = AsyncCapable::Unknown;
		std::shared_ptr<int> refcounter;

		void set_nonblocking(bool __nonblocking = true) {
			int flags = fcntl(fd_, F_GETFL, 0);
			if (flags == -1)
				throw std::system_error(errno, std::system_category(), "fcntl F_GETFL");

			flags = __nonblocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);

			if (fcntl(fd_, F_SETFL, flags))
				throw std::system_error(errno, std::system_category(), "fcntl F_SETFL");
		}

	public:
		File() = default;

		File(int __fd) : fd_(__fd), refcounter((int *)nullptr) {

		}

		virtual ~File() {
			close();
		}

		File(const File& o) {
			fd_ = o.fd_;
			refcounter = o.refcounter;
//			printf("copy-constructed, refcount=%ld\n", refcounter.use_count());
		}

		File(File&& o) {
//			puts("move-constructed\n");
			refcounter = std::move(o.refcounter);
			fd_ = o.fd_;
			o.fd_ = -1;
		}

		File& operator=(const File& o) = default;

		explicit operator bool() {
			return fd_ >= 0;
		}

		int fd() const noexcept {
			return fd_;
		}

		int &fd() noexcept {
			return fd_;
		}

		void open(const std::string& __path, int __mode = O_RDWR) {
			close();

			fd_ = ::open(__path.c_str(), __mode);

			if (fd_ < 0)
				throw std::system_error(errno, std::system_category(), "failed to open");

			refcounter.reset((int *)nullptr);
		}

		void close() noexcept {
//			printf("close, refcount=%ld\n", refcounter.use_count());
			if (refcounter.use_count() == 1) {
				::close(fd_);
				fd_ = -1;
			}
		}

		struct stat stat() const {
			struct stat stbuf;
			int rc = ::fstat(fd_, &stbuf);
			if (rc < 0)
				throw std::system_error(errno, std::system_category(), "failed to stat");
		}

		ssize_t write(const void *__buf, size_t __len) {
			return ::write(fd_, __buf, __len);
		}

		ssize_t read(void *__buf, size_t __len) {
			return ::read(fd_, __buf, __len);
		}

		ssize_t write_all(const void *__buf, size_t __len) {
			size_t written = 0;

			while (written < __len) {
				ssize_t rc = write((uint8_t *)__buf + written, __len - written);
				if (rc > 0) {
					written += rc;
				} else if (rc == 0) {
					return written;
				} else {
					return -1;
				}
			}

			return written;
		}

		template<typename T>
		bool write_all(const std::vector<T> &__buf) {
			return write_all(__buf.data(), __buf.size() * sizeof(T));
		}

		template<typename T>
		bool write_all(const T &__buf) {
			return write_all(__buf.inner_data(), __buf.size());
		}

		ssize_t read_all(void *__buf, size_t __len) {
			size_t readd = 0;

			while (readd < __len) {
				ssize_t rc = read((uint8_t *)__buf + readd, __len - readd);
				if (rc > 0) {
					readd += rc;
				} else if (rc == 0) {
					return readd;
				} else {
					return -1;
				}
			}

			return readd;
		}

		std::optional<std::vector<uint8_t>> read_all(size_t __len) {
			std::vector<uint8_t> ret(__len);

			if (read_all(ret.data(), __len) == -1) {
				return {};
			} else {
				return ret;
			}
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