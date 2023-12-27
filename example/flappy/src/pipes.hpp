#pragma once
#include <bave/core/random.hpp>
#include <bave/graphics/sprite.hpp>
#include <src/config.hpp>

class Pipes {
  public:
	explicit Pipes(bave::NotNull<bave::RenderDevice*> render_device, bave::NotNull<Config const*> config);

	void tick(bave::Seconds dt);
	void draw(bave::Shader& shader) const;

	[[nodiscard]] auto pipe_exists() const -> bool { return !m_pipes.empty(); }
	[[nodiscard]] auto is_colliding(bave::Rect<> const& rect) const -> bool;
	void restart();

	std::shared_ptr<bave::Texture> pipe_texture{};

  private:
	struct Pipe {
		bave::Sprite top;
		bave::Sprite bottom;

		void translate(float distance);
	};

	[[nodiscard]] auto make_pipe() const -> Pipe;
	auto get_next_pipe() -> Pipe&;
	void spawn_pipe();

	bave::NotNull<bave::RenderDevice*> m_render_device;
	bave::NotNull<Config const*> m_config;
	std::vector<Pipe> m_pipes{};
	bave::Seconds m_next_spawn{};
	bave::Random m_random{};
};
