#pragma once
#include <glm/vec2.hpp>
#include <concepts>
#include <random>

namespace bave {
template <typename Type>
concept NumericT = std::integral<Type> || std::floating_point<Type>;

/// Stateful random number generator.
template <typename Policy>
class BasicRandom {
  public:
	using seed_type = decltype(std::random_device{}());

	template <std::integral Type>
	using int_type = typename Policy::template Integral<Type>;

	template <std::floating_point Type>
	using real_type = typename Policy::template Real<Type>;

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
	template <NumericT Type>
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

template <typename Type>
[[nodiscard]] auto random_in_range(Type const& lo, Type const& hi) -> Type {
	static auto s_instance = Random{};
	return s_instance.in_range(lo, hi);
}
} // namespace bave
