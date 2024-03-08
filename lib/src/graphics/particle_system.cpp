#include <bave/core/random.hpp>
#include <bave/core/visitor.hpp>
#include <bave/graphics/particle_system.hpp>
#include <algorithm>

namespace bave {
namespace {
auto random_vec2(InclusiveRange<glm::vec2> const& range) {
	auto ret = glm::vec2{};
	ret.x = random_in_range(range.lo.x, range.hi.x);
	ret.y = random_in_range(range.lo.y, range.hi.y);
	return ret;
}
} // namespace

void ParticleEmitter::Particle::translate(Seconds const dt) { transform.position += velocity.linear * dt.count(); }
void ParticleEmitter::Particle::rotate(Seconds const dt) { transform.rotation.value += velocity.angular.value * dt.count(); }
void ParticleEmitter::Particle::scaleify(float const alpha) { transform.scale = glm::mix(lerp.scale.lo, lerp.scale.hi, alpha); }
void ParticleEmitter::Particle::tintify(float const alpha) { tint.channels = glm::mix(lerp.tint.lo.channels, lerp.tint.hi.channels, alpha); }

void ParticleEmitter::pre_warm(Seconds const dt, int ticks) {
	m_particles.clear();
	for (; ticks > 0; --ticks) {
		refresh_particles();
		tick_particles(dt);
	}
}

void ParticleEmitter::tick(Seconds dt) {
	refresh_particles();
	tick_particles(dt);
	sync_instances();
	set_shape(Quad{.size = config.quad_size});
}

auto ParticleEmitter::make_particle() const -> Particle {
	auto ret = Particle{};

	auto const spread = Degrees{random_in_range(config.velocity.linear.angle.lo.value, config.velocity.linear.angle.hi.value)};
	auto const direction = glm::vec3{glm::sin(spread), glm::cos(spread), 0.0f};
	ret.velocity.linear = random_in_range(config.velocity.linear.speed.lo, config.velocity.linear.speed.hi) * direction;
	ret.velocity.angular = random_in_range(config.velocity.angular.lo.value, config.velocity.angular.hi.value);

	ret.lerp.scale = config.lerp.scale;
	ret.lerp.tint = config.lerp.tint;

	ret.transform.position = m_position + random_vec2(config.initial.position);
	ret.transform.rotation = random_in_range(config.initial.rotation.lo.value, config.initial.rotation.hi.value);
	ret.transform.scale = config.lerp.scale.lo;

	ret.ttl = Seconds{random_in_range(config.ttl.lo.count(), config.ttl.hi.count())};
	ret.tint = ret.lerp.tint.lo;

	return ret;
}

void ParticleEmitter::refresh_particles() {
	std::erase_if(m_particles, [](Particle const& p) { return p.elapsed >= p.ttl; });

	if (config.respawn) {
		while (m_particles.size() < config.count) { m_particles.push_back(make_particle()); }
	}
}

void ParticleEmitter::tick_particles(Seconds const dt) {
	auto const do_translate = modifiers.test(Modifier::eTranslate);
	auto const do_rotate = modifiers.test(Modifier::eRotate);
	auto const do_scale = modifiers.test(Modifier::eScale);
	auto const do_tint = modifiers.test(Modifier::eTint);

	for (auto& particle : m_particles) {
		particle.elapsed += dt;
		auto const alpha = std::clamp(particle.elapsed / particle.ttl, 0.0f, 1.0f);
		if (do_translate) { particle.translate(dt); }
		if (do_rotate) { particle.rotate(dt); }
		if (do_scale) { particle.scaleify(alpha); }
		if (do_tint) { particle.tintify(alpha); }
	}
}

void ParticleEmitter::sync_instances() {
	parented_instances = false;
	instances.resize(m_particles.size());
	for (std::size_t index = 0; index < m_particles.size(); ++index) {
		auto const& particle = m_particles.at(index);
		instances.at(index) = RenderInstance{.transform = particle.transform, .tint = particle.tint};
	}
}
} // namespace bave
