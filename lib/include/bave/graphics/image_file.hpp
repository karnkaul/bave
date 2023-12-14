#pragma once
#include <bave/graphics/bitmap.hpp>
#include <memory>

namespace bave {
class ImageFile {
  public:
	auto decompress(std::span<std::byte const> compressed) -> bool;

	[[nodiscard]] auto bitmap() const -> BitmapView;

	[[nodiscard]] auto is_empty() const -> bool { return m_impl == nullptr; }

	explicit operator bool() const { return !is_empty(); }

  private:
	struct Impl;
	struct Deleter {
		void operator()(Impl* ptr) const;
	};
	std::unique_ptr<Impl, Deleter> m_impl{};
};
} // namespace bave
