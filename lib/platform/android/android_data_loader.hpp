#pragma once
#include <bave/core/not_null.hpp>
#include <bave/data_loader.hpp>
#include <bave/logger.hpp>
#include <bave/platform.hpp>

extern "C" {
struct android_app;
}

namespace bave {
static_assert(platform_v == Platform::eAndroid);

struct AndroidDataLoader : IDataLoader {
	explicit AndroidDataLoader(NotNull<android_app*> app) : m_app(app) {}

	[[nodiscard]] auto exists(std::string_view uri) const -> bool final;

	auto read_bytes(std::vector<std::byte>& out, std::string_view uri) const -> bool final;

	auto read_string(std::string& out, std::string_view uri) const -> bool final;

	Logger m_log{"AndroidDataLoader"};
	NotNull<android_app*> m_app;
};
} // namespace bave
