#pragma once
#include <glm/vec2.hpp>
#include <random>

namespace bave {
/// Stateful random number generator.
template <typename Policy>
class BasicRandom {
  public:
	using seed_type = decltype(std::random_device{}());

	template <std::integral Type>
	using int_type = Policy::template Integral<Type>;

	template <std::floating_point Type>
	using real_type = Policy::template Real<Type>;

	/// Construct using a custom or default (randomized) seed.
	explicit BasicRandom(seed_type seed = std::random_device{}()) : m_engine(seed) {}

	/// Obtain a random integer in the interval [lo, hi].
	template <std::integral Type>
	[[nodiscard]] auto in_range(Type lo, Type hi) -> Type {
		auto distribution = int_type<Type>{lo, hi};
		return distribution(m_engine);
	}

	/// Obtain a random float in the interval [lo, hi].
	template <std::floating_point Type>
	[[nodiscard]] auto in_range(Type lo, Type hi) -> Type {
		auto distribution = real_type<Type>{lo, hi};
		return distribution(m_engine);
	}

	/// Obtain a random vec2 in the interval [lo, hi] (per dimension).
	template <typename Type>
	[[nodiscard]] auto in_range(glm::tvec2<Type> lo, glm::tvec2<Type> hi) -> glm::tvec2<Type> {
		return glm::tvec2<Type>{in_range(lo.x, hi.x), in_range(lo.y, hi.y)};
	}

  private:
	std::default_random_engine m_engine;
};

/// Random policy for uniform distributions.
struct RandomPolicyUniform {
	template <std::integral Type>
	using Integral = std::uniform_int_distribution<Type>;

	template <std::floating_point Type>
	using Real = std::uniform_real_distribution<Type>;
};

/// Random number generator using uniform distributions.
using Random = BasicRandom<RandomPolicyUniform>;
} // namespace bave
