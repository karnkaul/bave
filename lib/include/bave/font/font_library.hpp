#pragma once
#include <bave/font/detail/glyph_slot.hpp>
#include <memory>

namespace bave {
class FontLibrary {
  public:
	struct Null;

	FontLibrary(FontLibrary const&) = delete;
	FontLibrary(FontLibrary&&) = delete;
	auto operator=(FontLibrary const&) -> FontLibrary& = delete;
	auto operator=(FontLibrary&&) -> FontLibrary& = delete;

	FontLibrary() = default;
	virtual ~FontLibrary() = default;

	[[nodiscard]] virtual auto load(std::vector<std::byte> bytes) const -> std::unique_ptr<detail::GlyphSlot::Factory> = 0;

	[[nodiscard]] static auto make() -> std::unique_ptr<FontLibrary>;
};

struct FontLibrary::Null : FontLibrary {
	[[nodiscard]] auto load(std::vector<std::byte> /*bytes*/) const -> std::unique_ptr<detail::GlyphSlot::Factory> final {
		return std::make_unique<detail::GlyphSlot::Factory::Null>();
	}
};
} // namespace bave
