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

#include <variant>

#include "EventLoop.hpp"
#include "Buffer.hpp"

namespace IODash {
#ifdef __linux__
	template<EventBackend EB = EventBackend::EPoll>
#else
	template<EventBackend EB = EventBackend::Poll>
#endif
	class IOService {
	public:
		struct PerFileContext {
			bool once[3];
			Buffer buffer_read, buffer_write;
			std::unique_ptr<ReadHandler> handler_read;
			std::unique_ptr<WriteHandler> handler_write;
			std::unique_ptr<AcceptHandler> handler_accept;
			size_t buffer_pos = 0;
		};
	private:
		EventLoop<EB> event_loop;
		std::unordered_multimap<int, PerFileContext> files;

	public:
		void async_read(const File& __file, Buffer&& __buf, const ReadHandler& __handler, bool __once = true) {
			auto it = event_loop.watched_objects().find(__file.fd());
			if (it == event_loop.watched_objects().end()) {
				event_loop.add(__file, EventType::In|EventType::Error|EventType::Hangup);
			} else {
				event_loop.modify(__file, std::get<1>(it.second)|EventType::In|EventType::Error|EventType::Hangup);
			}

			PerFileContext fctx;
			fctx.once[0] = __once;
			fctx.buffer_read = std::forward<Buffer>(__buf);
			fctx.read_handler = std::make_unique<ReadHandler>(__handler);
			files.emplace({__file.fd(), std::move(fctx)});

		}

		void run() {

		}
	};
}