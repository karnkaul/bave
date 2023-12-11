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

	virtual void tick() {}
	virtual void render([[maybe_unused]] vk::CommandBuffer command_buffer) const {}

	virtual void shutdown() {}

	Rgba clear_colour{black_v};

  private:
	App& m_app;
};
} // namespace bave
