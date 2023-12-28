#pragma once
#include <bave/font/font.hpp>
#include <bave/graphics/drawable.hpp>

namespace bave {
class Text : public Drawable {
  public:
	enum class Align : std::uint8_t { eMid, eLeft, eRight };

	using Height = TextHeight;

	explicit Text(NotNull<RenderDevice*> render_device, std::shared_ptr<Font> font = {});

	auto set_font(std::shared_ptr<Font> font) -> Text&;
	auto set_string(std::string text) -> Text&;
	auto set_height(Height height) -> Text&;
	auto set_align(Align align) -> Text&;
	auto set_scale(float scale) -> Text&;

	[[nodiscard]] auto get_font() const -> std::shared_ptr<Font> const& { return m_font; }
	[[nodiscard]] auto get_string() const -> std::string_view { return m_text; }
	[[nodiscard]] auto get_height() const -> Height { return m_height; }
	[[nodiscard]] auto get_align() const -> Align { return m_align; }
	[[nodiscard]] auto get_scale() const -> float { return m_scale; }

	[[nodiscard]] auto get_bounds() const -> Rect<>;

  private:
	void refresh();

	std::shared_ptr<Font> m_font{};
	std::string m_text{};
	Height m_height{Height::eDefault};
	float m_scale{1.0f};
	Align m_align{Align::eMid};
	glm::vec2 m_extent{};
	float m_x_offset{};
};
} // namespace bave
