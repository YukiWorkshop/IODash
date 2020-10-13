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
#include <vector>
#include <string>
#include <string_view>

#include <cstring>

namespace IODash {
	class Buffer {
	private:
		std::variant<std::vector<uint8_t>, std::string, std::string_view> inner_data;

	public:
		Buffer() = default;

		Buffer(size_t __len) {
			inner_data = std::vector<uint8_t>(__len);
		}

		size_t size() const noexcept {
			if (std::holds_alternative<std::vector<uint8_t>>(inner_data)) {
				return std::get<std::vector<uint8_t>>(inner_data).size();
			} else if (std::holds_alternative<std::string_view>(inner_data)) {
				return std::get<std::string_view>(inner_data).size();
			} else if (std::holds_alternative<std::string>(inner_data)) {
				return std::get<std::string>(inner_data).size();
			} else {
				return 0;
			}
		}

		void resize(size_t __len) {
			if (std::holds_alternative<std::vector<uint8_t>>(inner_data)) {
				std::get<std::vector<uint8_t>>(inner_data).resize(__len);
			} else if (std::holds_alternative<std::string>(inner_data)) {
				std::get<std::string>(inner_data).resize(__len);
			} else {
				throw std::logic_error("Empty buffer or reference");
			}
		}

		uint8_t *data() noexcept {
			if (std::holds_alternative<std::vector<uint8_t>>(inner_data)) {
				return (uint8_t *)std::get<std::vector<uint8_t>>(inner_data).data();
			} else if (std::holds_alternative<std::string_view>(inner_data)) {
				return (uint8_t *)std::get<std::string_view>(inner_data).data();
			} else if (std::holds_alternative<std::string>(inner_data)) {
				return (uint8_t *)std::get<std::string>(inner_data).data();
			} else {
				return nullptr;
			}
		}

		const uint8_t *data() const noexcept {
			if (std::holds_alternative<std::vector<uint8_t>>(inner_data)) {
				return (uint8_t *)std::get<std::vector<uint8_t>>(inner_data).data();
			} else if (std::holds_alternative<std::string_view>(inner_data)) {
				return (uint8_t *)std::get<std::string_view>(inner_data).data();
			} else if (std::holds_alternative<std::string>(inner_data)) {
				return (uint8_t *)std::get<std::string>(inner_data).data();
			} else {
				return nullptr;
			}
		}

		template <typename T>
		static Buffer copy(const std::vector<T> &__buf) {
			Buffer ret;
			ret.inner_data = std::vector<uint8_t>((const uint8_t *)__buf.data(), ((const uint8_t *)__buf.data()) + __buf.size() * sizeof(T));
			return ret;
		}

		template <typename T>
		static Buffer copy(const T &__buf) {
			Buffer ret;
			ret.inner_data = std::vector<uint8_t>((const uint8_t *)__buf.data(), ((const uint8_t *)__buf.data()) + __buf.size());
			return ret;
		}

		static Buffer copy(const char *__buf, ssize_t __len = -1) {
			Buffer ret;
			if (__len == -1) {
				__len = strlen(__buf);
			}
			ret.inner_data = std::vector<uint8_t>((const uint8_t *)__buf, ((const uint8_t *)__buf) + __len);
			return ret;
		}

		template <typename T>
		static Buffer reference(const std::vector<T> &__buf) {
			Buffer ret;
			ret.inner_data = std::string_view((const char *)__buf.data(), __buf.size() * sizeof(T));
			return ret;
		}

		template <typename T>
		static Buffer reference(const T &__buf) {
			Buffer ret;
			ret.inner_data = std::string_view((const char *)__buf.data(), __buf.size());
			return ret;
		}

		static Buffer reference(const char *__buf, ssize_t __len = -1) {
			Buffer ret;
			if (__len == -1) {
				__len = strlen(__buf);
			}
			ret.inner_data = std::string_view((const char *)__buf, __len);
			return ret;
		}

		template <typename T>
		static Buffer steal(T &&__buf) {
			Buffer ret;
			ret.inner_data = std::move(__buf);
			return ret;
		}

	};
}