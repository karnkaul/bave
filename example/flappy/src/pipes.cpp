#include <src/pipes.hpp>
#include <algorithm>

using bave::NotNull;
using bave::Rect;
using bave::Seconds;
using bave::Shader;
using bave::Sprite;

namespace {
constexpr auto is_oob_left(Rect<> const& rect, glm::vec2 const world_space) -> bool { return rect.top_right().x < -0.5f * world_space.x; }
} // namespace

void Pipes::Pipe::translate(float const distance) {
	top.transform.position.x += distance;
	bottom.transform.position.x += distance;
}

Pipes::Pipes(NotNull<bave::RenderDevice*> render_device, NotNull<Config const*> config)
	: m_render_device(render_device), m_config(config), m_next_spawn(m_config->pipe_period) {}

void Pipes::tick(Seconds dt) {
	m_next_spawn -= dt;
	if (m_next_spawn <= 0s) {
		spawn_pipe();
		m_next_spawn = m_config->pipe_period;
	}

	for (auto& pipe : m_pipes) { pipe.translate(m_config->pipe_speed * dt.count()); }
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
		.top = Sprite{m_render_device},
		.bottom = Sprite{m_render_device},
	};
	ret.top.transform.scale.y = -1.0f;
	ret.top.set_size(m_config->pipe_size);
	ret.bottom.set_size(m_config->pipe_size);
	ret.top.set_texture(m_config->pipe_texture);
	ret.bottom.set_texture(m_config->pipe_texture);
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
	auto const y_pos = m_random.in_range(-0.25f * m_config->world_space.y, 0.25f * m_config->world_space.y);
	auto const y_spacing = 0.5f * m_config->pipe_size.y + m_config->pipe_gap;
	pipe.top.transform.position.y = y_pos + y_spacing;
	pipe.bottom.transform.position.y = y_pos - y_spacing;
	pipe.top.transform.position.x = pipe.bottom.transform.position.x = 0.5f * (m_config->world_space.x + m_config->pipe_size.x);
}
