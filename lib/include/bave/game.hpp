#pragma once
#include <bave/app.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/graphics/rgba.hpp>
#include <bave/logger.hpp>
#include <vulkan/vulkan.hpp>

namespace bave {
class Game : public PolyPinned {
  public:
	explicit Game(App& app) : m_app(app) {}

	[[nodiscard]] auto get_app() const -> App& { return m_app; }

	[[nodiscard]] auto load_shader(std::string_view vertex, std::string_view fragment) const -> std::optional<Shader> {
		return get_app().get_frame_renderer().load_shader(vertex, fragment);
	}

	virtual void tick() {}
	virtual void render([[maybe_unused]] vk::CommandBuffer command_buffer) const {}

	virtual void shutdown() {}

	Rgba clear_colour{black_v};
	Transform render_view{};

  private:
	App& m_app;
};
} // namespace bave
