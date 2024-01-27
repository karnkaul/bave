#pragma once
#include <glm/vec2.hpp>
#include <concepts>
#include <random>

namespace bave {
/// \brief Concept for numeric types.
template <typename Type>
concept NumericT = std::integral<Type> || std::floating_point<Type>;

/// \brief Stateful random number generator.
template <typename Policy>
class BasicRandom {
  public:
	using seed_type = decltype(std::random_device{}());

	template <std::integral Type>
	using int_type = typename Policy::template Integral<Type>;

	template <std::floating_point Type>
	using real_type = typename Policy::template Real<Type>;

	/// \brief Construct using a custom or default (randomized) seed.
	/// \param seed Initial seed.
	explicit BasicRandom(seed_type const seed = std::random_device{}()) : m_engine(seed) {}

	/// \brief Obtain a random integer in the interval [lo, hi].
	/// \param lo Lower bound.
	/// \param hi Upper bound.
	/// \returns Random integer.
	template <std::integral Type>
	[[nodiscard]] auto in_range(Type const lo, Type const hi) -> Type {
		auto distribution = int_type<Type>{lo, hi};
		return distribution(m_engine);
	}

	/// \brief Obtain a random float in the interval [lo, hi].
	/// \param lo Lower bound.
	/// \param hi Upper bound.
	/// \returns Random floating point.
	template <std::floating_point Type>
	[[nodiscard]] auto in_range(Type lo, Type hi) -> Type {
		auto distribution = real_type<Type>{lo, hi};
		return distribution(m_engine);
	}

	/// \brief Obtain a random vec2 in the interval [lo, hi] (per dimension).
	/// \param lo Lower bound.
	/// \param hi Upper bound.
	/// \returns Random GLM vector.
	template <NumericT Type>
	[[nodiscard]] auto in_range(glm::tvec2<Type> lo, glm::tvec2<Type> hi) -> glm::tvec2<Type> {
		return glm::tvec2<Type>{in_range(lo.x, hi.x), in_range(lo.y, hi.y)};
	}

  private:
	std::default_random_engine m_engine;
};

/// \brief Random policy for uniform distributions.
struct RandomPolicyUniform {
	template <std::integral Type>
	using Integral = std::uniform_int_distribution<Type>;

	template <std::floating_point Type>
	using Real = std::uniform_real_distribution<Type>;
};

/// \brief Random number generator using uniform distributions.
using Random = BasicRandom<RandomPolicyUniform>;

/// \brief Generate a random number using global state (per thread).
/// \param lo Lower bound.
/// \param hi Upper bound.
/// \returns Random Type.
template <typename Type>
[[nodiscard]] auto random_in_range(Type const& lo, Type const& hi) -> Type {
	thread_local auto s_instance = Random{};
	return s_instance.in_range(lo, hi);
}
} // namespace bave
