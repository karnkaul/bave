#pragma once
#include <bave/graphics/particle_emitter.hpp>

namespace bave {
/// \brief Container of ParticleEmitter instances.
class ParticleSystem : public IDrawable {
  public:
	/// \brief Draw all emitters using a given shader.
	/// \param shader Shader to use.
	void draw(Shader& shader) const final {
		for (auto const& emitter : emitters) { emitter.draw(shader); }
	}

	/// \brief Tick all emitters.
	/// \param dt Delta time since last call.
	void tick(Seconds const dt) {
		for (auto& emitter : emitters) { emitter.tick(dt); }
	}

	/// \brief Pre-warm particles of all emitters by simulating ticks.
	/// \param dt Delta time to use per tick.
	/// \param ticks Number of ticks to simulate.
	void pre_warm(Seconds const dt = 50ms, int const ticks = 100) {
		for (auto& emitter : emitters) { emitter.pre_warm(dt, ticks); }
	}

	/// \brief Respawn particles for all emitters.
	///
	/// Has no effect if emitter's config.respawn is true.
	void respawn() {
		for (auto& emitter : emitters) { emitter.respawn(); }
	}

	/// \brief Vector of emitters.
	std::vector<ParticleEmitter> emitters{};
};
} // namespace bave
