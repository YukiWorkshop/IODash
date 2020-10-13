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

#include <unordered_map>
#include <functional>

#include <poll.h>

#ifdef __linux__
#include <sys/epoll.h>
#endif

#include <portable-endian.h>

#include "Socket.hpp"

namespace IODash {

	enum class EventBackend : uint8_t {
		Any = 0,
		Poll = 1,
#ifdef __linux__
		EPoll = 2,
#endif
#ifdef __FreeBSD__ // TODO: Other BSDs
		Kqueue = 3
#endif
	};

	enum EventType : uint8_t {
		None = 0x0,

		In = 0x1, Out = 0x2, Error = 0x4, Hangup = 0x8,

		All = In | Out | Error | Hangup
	};


	inline constexpr EventType
	operator&(EventType x, EventType y) {
		return static_cast<EventType>
		(static_cast<uint8_t>(x) & static_cast<uint8_t>(y));
	}

	inline constexpr EventType
	operator|(EventType x, EventType y) {
		return static_cast<EventType>
		(static_cast<uint8_t>(x) | static_cast<uint8_t>(y));
	}

	inline constexpr EventType
	operator^(EventType x, EventType y) {
		return static_cast<EventType>
		(static_cast<uint8_t>(x) ^ static_cast<uint8_t>(y));
	}

	inline constexpr EventType
	operator~(EventType x) {
		return static_cast<EventType>(~static_cast<uint8_t>(x));
	}

	inline EventType &
	operator&=(EventType & x, EventType y) {
		x = x & y;
		return x;
	}

	inline EventType &
	operator|=(EventType & x, EventType y) {
		x = x | y;
		return x;
	}

	inline EventType &
	operator^=(EventType & x, EventType y) {
		x = x ^ y;
		return x;
	}

#ifdef __linux__
	template<EventBackend EB = EventBackend::EPoll, typename UD = int>
#else
	template<EventBackend EB = EventBackend::Poll, typename UD = int>
#endif
	class EventLoop {
	private:

	protected:
		std::unordered_map<int, std::tuple<File, EventType, UD>> watched_fds;
		std::function<void(EventLoop&, File&, EventType, UD&)> event_handlers[(uint8_t)EventType::All+1];

		std::function<void(EventLoop&)> handler_post_events;
		std::function<void(EventLoop&)> handler_idle;

		bool run_ = false;

		virtual void __lower_add(int __fd, EventType __events) {

		};

		virtual void __lower_mod(int __fd, EventType __events) {

		};

		virtual void __lower_del(int __fd) {

		};

		void __call_event_handler(int __fd, EventType __ev) {
			auto it = watched_fds.find(__fd);

			if (it != watched_fds.end()) {
				for (uint8_t i=0; i<=EventType::All; i++) {
					if ((i & __ev) == __ev) {
						if (event_handlers[i]) {
							event_handlers[i](*this, std::get<0>(it->second), __ev,
									  std::get<2>(it->second));
						}
					}
				}
			}
		}

	public:
		virtual void run() = 0;

		void stop() {
			run_ = false;
		}

		void add(const File& __target, EventType __events = EventType::All, const UD& __user_data = {}) {
			__lower_add(__target.fd(), __events);
			watched_fds.insert({__target.fd(), {__target, __events, __user_data}});
		}

		void modify(const File& __target, EventType __events, const UD& __user_data) {
			__lower_mod(__target.fd(), __events);
			auto &it = watched_fds[__target.fd()];
			std::get<1>(it) = __events;
			std::get<2>(it) = __user_data;
		}

		void modify(const File& __target, EventType __events) {
			__lower_mod(__target.fd(), __events);
			auto &it = watched_fds[__target.fd()];
			std::get<1>(it) = __events;
		}

		void del(const File& __target) {
			__lower_del(__target.fd());
			watched_fds.erase(__target.fd());
		}

		const std::unordered_map<int, std::tuple<File, EventType, UD>>& watched_objects() const {
			return watched_fds;
		}

		template <typename T>
		void on_event(EventType __events, const T& __func) {
			event_handlers[__events] = __func;
		}

		void on_post_events(const std::function<void(EventLoop&)>& __func) {
			handler_post_events = __func;
		}

		void on_idle(const std::function<void(EventLoop&)>& __func) {
			handler_idle = __func;
		}

	};

#ifdef __linux__
	template<typename T>
	class EventLoop<EventBackend::EPoll, T> : public EventLoop<EventBackend::Any, T> {
	protected:
		int fd_poll = -1;

		EventType __translate_events_to(int __epoll_events) {
			EventType ret = EventType::None;

			if (__epoll_events & EPOLLIN)
				ret |= EventType::In;

			if (__epoll_events & EPOLLOUT)
				ret |= EventType::Out;

			if (__epoll_events & EPOLLERR)
				ret |= EventType::Error;

			if (__epoll_events & EPOLLHUP)
				ret |= EventType::Hangup;

			return ret;
		}

		int __translate_events_from(EventType __events) {
			int ret = 0;

			if (__events & EventType::In)
				ret |= EPOLLIN;

			if (__events & EventType::Out)
				ret |= EPOLLOUT;

			if (__events & EventType::Error)
				ret |= EPOLLERR;

			if (__events & EventType::Hangup)
				ret |= EPOLLHUP;

			return ret;
		}

		void __add_pre() {
			for (auto &it : EventLoop<EventBackend::Any, T>::watched_fds) {
				epoll_event ev;
				ev.data.fd = it.first;
				ev.events = __translate_events_from(std::get<1>(it.second));

				if (epoll_ctl(fd_poll, EPOLL_CTL_ADD, ev.data.fd, &ev))
					throw std::system_error(errno, std::system_category(), "EPOLL_CTL_ADD");
			}
		}

		virtual void __lower_add(int __fd, EventType __events) override {
			if (fd_poll > 0) {
				epoll_event ev;
				ev.data.fd = __fd;
				ev.events = __translate_events_from(__events);

				if (epoll_ctl(fd_poll, EPOLL_CTL_ADD, __fd, &ev))
					throw std::system_error(errno, std::system_category(), "EPOLL_CTL_ADD");
			}
		}

		virtual void __lower_mod(int __fd, EventType __events) override {
			if (fd_poll > 0) {
				epoll_event ev;
				ev.data.fd = __fd;
				ev.events = __translate_events_from(__events);

				if (epoll_ctl(fd_poll, EPOLL_CTL_MOD, __fd, &ev))
					throw std::system_error(errno, std::system_category(), "EPOLL_CTL_MOD");
			}
		}

		virtual void __lower_del(int __fd) override {
			if (fd_poll > 0) {
				if (epoll_ctl(fd_poll, EPOLL_CTL_DEL, __fd, nullptr))
					throw std::system_error(errno, std::system_category(), "EPOLL_CTL_DEL");
			}
		}

	public:
		virtual void run() override {
			EventLoop<EventBackend::Any, T>::run_ = true;
			fd_poll = epoll_create(42);
			__add_pre();

			epoll_event evs[128];
			while (EventLoop<EventBackend::Any, T>::run_) {
				int rc = epoll_wait(fd_poll, evs, 128, 5000);

				if (rc > 0) {
					for (uint32_t i=0; i<rc; i++) {
						auto &cur_ev = evs[i];
						EventLoop<EventBackend::Any, T>::__call_event_handler(cur_ev.data.fd, __translate_events_to(cur_ev.events));
					}
					if (EventLoop<EventBackend::Any, T>::handler_post_events)
						EventLoop<EventBackend::Any, T>::handler_post_events(*this);
				} else if (rc == 0) {
					if (EventLoop<EventBackend::Any, T>::handler_idle)
						EventLoop<EventBackend::Any, T>::handler_idle(*this);
				} else if (rc < 0) {
					if (errno != EINTR)
						throw std::system_error(errno, std::system_category(), "epoll_wait");
				}
			}

		}
	};
#endif

	template<typename T>
	class EventLoop<EventBackend::Poll, T> : public EventLoop<EventBackend::Any, T> {
	protected:

		EventType __translate_events_to(int __poll_events) {
			EventType ret = EventType::None;

			if (__poll_events & POLLIN)
				ret |= EventType::In;

			if (__poll_events & POLLOUT)
				ret |= EventType::Out;

			if (__poll_events & POLLERR)
				ret |= EventType::Error;

			if (__poll_events & POLLHUP)
				ret |= EventType::Hangup;

			return ret;
		}

		short __translate_events_from(EventType __events) {
			short ret = 0;

			if (__events & EventType::In)
				ret |= POLLIN;

			if (__events & EventType::Out)
				ret |= POLLOUT;

			if (__events & EventType::Error)
				ret |= POLLERR;

			if (__events & EventType::Hangup)
				ret |= POLLHUP;

			return ret;
		}


	public:
		virtual void run() override {
			EventLoop<EventBackend::Any, T>::run_ = true;

			while (EventLoop<EventBackend::Any, T>::run_) {
				std::vector<pollfd> pfds;
				pfds.reserve(EventLoop<EventBackend::Any, T>::watched_fds.size());

				for (auto &it : EventLoop<EventBackend::Any, T>::watched_fds) {
					auto &r = pfds.emplace_back();
					r.fd = it.first;
					r.events = __translate_events_from(std::get<1>(it.second));
				}

				int rc = poll(pfds.data(), pfds.size(), 5000);

				if (rc > 0) {
					for (auto &it : pfds) {
						if (it.revents) {
							EventLoop<EventBackend::Any, T>::__call_event_handler(it.fd, __translate_events_to(it.revents));
						}
					}
					if (EventLoop<EventBackend::Any, T>::handler_post_events)
						EventLoop<EventBackend::Any, T>::handler_post_events(*this);
				} else if (rc == 0) {
					if (EventLoop<EventBackend::Any, T>::handler_idle)
						EventLoop<EventBackend::Any, T>::handler_idle(*this);
				} else if (rc < 0) {
					if (errno != EINTR && errno != EAGAIN)
						throw std::system_error(errno, std::system_category(), "poll");
				}
			}

		}
	};
}