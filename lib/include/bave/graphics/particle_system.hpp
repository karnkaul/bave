#pragma once
#include <bave/core/enum_flags.hpp>
#include <bave/core/inclusive_range.hpp>
#include <bave/core/time.hpp>
#include <bave/graphics/particle_config.hpp>
#include <bave/graphics/sprite.hpp>

namespace bave {
/// \brief Particle Emitter.
///
/// An emitter's transform member is ignored.
class ParticleEmitter : public Sprite {
  public:
	enum class Modifier : int { eTranslate, eRotate, eScale, eTint, eCOUNT_ };
	using Modifiers = EnumFlags<Modifier>;

	inline static auto const all_modifiers_v = Modifiers{Modifier::eTranslate, Modifier::eRotate, Modifier::eScale, Modifier::eTint};

	using Config = ParticleConfig;

	Config config{};
	Modifiers modifiers{all_modifiers_v};

	void tick(Seconds dt);

	/// \brief Pre-warm particles by simulating ticks.
	/// \param dt Delta time to use per tick.
	/// \param ticks Number of ticks to simulate.
	void pre_warm(Seconds dt = 50ms, int ticks = 100);

	/// \brief Respawn particles.
	///
	/// Has no effect if config.respawn is true.
	void respawn();

	/// \brief Set emitter position. Does not affect already spawned particles.
	void set_position(glm::vec2 position) { m_position = position; }
	/// \brief Get the emitter's position.
	[[nodiscard]] auto get_position() const -> glm::vec2 { return m_position; }

	[[nodiscard]] auto active_particles() const -> std::size_t { return m_particles.size(); }

  private:
	struct Particle {
		struct {
			glm::vec2 linear{};
			Radians angular{};
		} velocity{};

		struct {
			InclusiveRange<Rgba> tint{};
			InclusiveRange<glm::vec2> scale{};
		} lerp{};

		Seconds ttl{};

		Transform transform{};
		Rgba tint{};
		Seconds elapsed{};

		void translate(Seconds dt);
		void rotate(Seconds dt);
		void scaleify(float alpha);
		void tintify(float alpha);
	};

	[[nodiscard]] auto make_particle() const -> Particle;

	void refresh_particles(bool respawn);
	void tick_particles(Seconds dt);
	void sync_instances();

	std::vector<Particle> m_particles{};
	glm::vec2 m_position{};
	bool m_ticked{};
};

/// \brief Container of ParticleEmitter instances.
class ParticleSystem : public IDrawable {
  public:
	/// \brief Draw all emitters using a given shader.
	/// \param shader Shader instance to use.
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
