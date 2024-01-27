#pragma once
#include <bave/core/inclusive_range.hpp>
#include <bave/core/make_bitset.hpp>
#include <bave/core/time.hpp>
#include <bave/graphics/shape.hpp>

namespace bave {
/// \brief Configuration for a particle emitter.
struct ParticleConfig {
	struct {
		InclusiveRange<glm::vec2> position{};
		InclusiveRange<Radians> rotation{};
	} initial{};

	struct {
		struct {
			InclusiveRange<Radians> angle{Degrees{-180.0f}, Degrees{180.0f}};
			InclusiveRange<float> speed{-200.0f, 200.0f};
		} linear{};
		InclusiveRange<Radians> angular{Degrees{-90.0f}, Degrees{90.0f}};
	} velocity{};

	struct {
		InclusiveRange<Rgba> tint{white_v, Rgba{.channels = {0xff, 0xff, 0xff, 0x0}}};
		InclusiveRange<glm::vec2> scale{glm::vec2{1.0f}, glm::vec2{0.0f}};
	} lerp{};

	InclusiveRange<Seconds> ttl{1s, 5s};
	glm::vec2 quad_size{50.0f};
	std::size_t count{100};
	bool respawn{true};
};

/// \brief Particle Emitter.
class ParticleEmitter : public QuadShape {
  public:
	struct Modifier {
		static constexpr std::size_t translate{0};
		static constexpr std::size_t rotate{1};
		static constexpr std::size_t scale{2};
		static constexpr std::size_t tint{3};

		static constexpr std::size_t count_v{tint + 1};
	};
	using Modifiers = std::bitset<Modifier::count_v>;
	inline static auto const all_modifiers_v = make_bitset<Modifiers>(Modifier::translate, Modifier::rotate, Modifier::scale, Modifier::tint);

	using Config = ParticleConfig;

	Config config{};
	Modifiers modifiers{all_modifiers_v};

	void tick(Seconds dt);

	void respawn_all();

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
		float alpha{};

		void translate(Seconds dt);
		void rotate(Seconds dt);
		void scaleify();
		void tintify();
	};

	[[nodiscard]] auto make_particle() const -> Particle;

	std::vector<Particle> m_particles{};
};
} // namespace bave
