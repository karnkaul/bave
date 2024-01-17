#include <src/pipes.hpp>
#include <algorithm>

using bave::NotNull;
using bave::Rect;
using bave::Seconds;
using bave::Shader;
using bave::Sprite9Slice;

namespace {
constexpr auto is_oob_left(Rect<> const& rect, glm::vec2 const world_space) -> bool { return rect.top_right().x < -0.5f * world_space.x; }
} // namespace

void Pipes::Pipe::translate(float const distance) {
	top.transform.position.x += distance;
	bottom.transform.position.x += distance;
}

Pipes::Pipes(NotNull<bave::RenderDevice*> render_device, NotNull<Config const*> config)
	: m_render_device(render_device), m_config(config), m_next_spawn(m_config->pipe_period) {}

auto Pipes::tick(Seconds dt) -> bool {
	m_next_spawn -= dt;
	if (m_next_spawn <= 0s) {
		spawn_pipe();
		m_next_spawn = m_config->pipe_period;
	}

	auto ret = false;
	auto const active_size = 0.5f * (m_config->pipe_width + m_config->player_size.x);
	for (auto& pipe : m_pipes) {
		pipe.translate(-m_config->pipe_speed * dt.count());
		auto const was_active = pipe.active;
		pipe.active = pipe.top.transform.position.x + active_size > 0.0f;
		if (was_active && !pipe.active) { ret = true; }
	}

	return ret;
}

void Pipes::draw(Shader& shader) const {
	for (auto const& pipe : m_pipes) {
		pipe.top.draw(shader);
		pipe.bottom.draw(shader);
	}
}

auto Pipes::is_colliding(bave::Rect<> const& rect) const -> bool {
	auto const func = [rect](Pipe const& pipe) {
		return bave::is_intersecting(pipe.top.get_bounds(), rect) || bave::is_intersecting(pipe.bottom.get_bounds(), rect);
	};
	return std::any_of(m_pipes.begin(), m_pipes.end(), func);
}

void Pipes::restart() {
	m_pipes.clear();
	m_next_spawn = m_config->pipe_period;
}

auto Pipes::make_pipe() const -> Pipe {
	auto ret = Pipe{
		.top = Sprite9Slice{m_render_device},
		.bottom = Sprite9Slice{m_render_device},
	};
	ret.top.transform.scale.y = -1.0f;
	ret.top.set_texture_9slice(m_config->pipe_texture);
	ret.bottom.set_texture_9slice(m_config->pipe_texture);
	return ret;
}

auto Pipes::get_next_pipe() -> Pipe& {
	for (auto& pipe : m_pipes) {
		if (is_oob_left(pipe.top.get_bounds(), m_config->world_space)) { return pipe; }
	}

	m_pipes.push_back(make_pipe());
	return m_pipes.back();
}

void Pipes::spawn_pipe() {
	auto& pipe = get_next_pipe();
	auto const top_size = glm::vec2{m_config->pipe_width, m_random.in_range(100.0f, m_config->pipe_n_max_height * m_config->world_space.y)};
	pipe.top.set_size(top_size);
	pipe.top.transform.position.y = 0.5f * (m_config->world_space.y - top_size.y);
	auto const bottom_size = glm::vec2{top_size.x, m_config->world_space.y - top_size.y - 2.0f * m_config->pipe_gap};
	pipe.bottom.set_size(bottom_size);
	pipe.bottom.transform.position.y = -0.5f * (m_config->world_space.y - bottom_size.y);
	pipe.top.transform.position.x = pipe.bottom.transform.position.x = 0.5f * (m_config->world_space.x + m_config->pipe_width);
}
