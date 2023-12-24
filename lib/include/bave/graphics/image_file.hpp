#pragma once
#include <bave/graphics/bitmap.hpp>
#include <memory>

namespace bave {
class ImageFile : public IBitmapViewSource {
  public:
	auto load_from_bytes(std::span<std::byte const> compressed) -> bool;

	[[nodiscard]] auto get_bitmap_view() const -> BitmapView final;

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
