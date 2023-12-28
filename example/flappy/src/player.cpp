#include <src/player.hpp>

using bave::App;
using bave::NotNull;
using bave::Seconds;
using bave::Shader;

Player::Player(NotNull<App const*> app, NotNull<Config const*> config) : sprite(&app->get_render_device()), m_app(app), m_config(config) {
	sprite.set_texture(config->player_texture);
	sprite.set_size(config->player_size);
}

void Player::tick(Seconds dt) {
	auto y_speed = m_config->gravity;

	if (m_jump_elapsed) {
		*m_jump_elapsed += dt;
		if (*m_jump_elapsed >= m_config->max_jump_duration) { m_jump_elapsed.reset(); }

		y_speed = m_config->jump_speed;
	}

	sprite.transform.position.y += y_speed * dt.count();
}

void Player::draw(Shader& shader) const { sprite.draw(shader); }

void Player::start_jump() {
	m_jump_elapsed = 0s;
	if (m_config->jump_sfx) { m_app->get_audio_device().play_once(*m_config->jump_sfx); }
}

void Player::stop_jump() { m_jump_elapsed.reset(); }

void Player::restart() {
	m_jump_elapsed.reset();
	sprite.transform.position.y = {};
}
