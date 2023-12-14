#pragma once
#include <bave/core/polymorphic.hpp>
#include <bave/font/codepoint.hpp>
#include <bave/font/text_height.hpp>
#include <bave/graphics/pixmap.hpp>

namespace bave::detail {
struct GlyphSlot {
	class Factory;

	Pixmap pixmap{};
	glm::ivec2 left_top{};
	glm::ivec2 advance{};
	Codepoint codepoint{};

	[[nodiscard]] auto has_pixmap() const -> bool { return !pixmap.is_empty(); }

	explicit operator bool() const { return advance.x > 0 || advance.y > 0; }
};

class GlyphSlot::Factory : public PolyPinned {
  public:
	struct Null;

	virtual auto set_height(TextHeight height) -> bool = 0;
	[[nodiscard]] virtual auto height() const -> TextHeight = 0;
	[[nodiscard]] virtual auto slot_for(Codepoint codepoint) const -> GlyphSlot = 0;

	auto slot_for(Codepoint codepoint, TextHeight height) -> GlyphSlot {
		if (!set_height(height)) { return {}; }
		return slot_for(codepoint);
	}
};

struct GlyphSlot::Factory::Null : GlyphSlot::Factory {
	using Factory::Factory;

	auto set_height(TextHeight /*height*/) -> bool final { return true; }
	[[nodiscard]] auto height() const -> TextHeight final { return {}; }
	[[nodiscard]] auto slot_for(Codepoint /*codepoint*/) const -> GlyphSlot final { return {}; }
};
} // namespace bave::detail
