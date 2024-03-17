#pragma once
#include <bave/core/inclusive_range.hpp>
#include <bave/core/radians.hpp>
#include <bave/core/time.hpp>
#include <bave/graphics/rgba.hpp>
#include <glm/vec2.hpp>

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
} // namespace bave
