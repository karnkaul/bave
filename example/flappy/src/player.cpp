#include <src/player.hpp>

using bave::App;
using bave::NotNull;
using bave::Seconds;
using bave::Shader;

Player::Player(App& app, NotNull<Config const*> config) : config(config), sprite(&app.get_render_device()) {}

void Player::tick(Seconds dt) {
	sprite.set_size(config->player_size);

	auto y_speed = config->gravity;

	if (m_jump_elapsed) {
		*m_jump_elapsed += dt;
		if (*m_jump_elapsed >= config->max_jump_duration) { m_jump_elapsed.reset(); }

		y_speed = config->jump_speed;
	}

	sprite.transform.position.y += y_speed * dt.count();
}

void Player::draw(Shader& shader) const { sprite.draw(shader); }

void Player::start_jump() { m_jump_elapsed = 0s; }

void Player::stop_jump() { m_jump_elapsed.reset(); }

void Player::restart() {
	m_jump_elapsed.reset();
	sprite.transform.position.y = {};
}
