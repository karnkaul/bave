#include <bave/input/pinch_detector.hpp>
#include <glm/gtx/norm.hpp>

namespace bave {
auto PinchDetector::update(std::span<Pointer const> active) -> std::optional<float> {
	if (active.size() != 2) {
		m_distance.reset();
		m_delta = 0.0f;
		return {};
	}

	auto const dist = glm::length(active[1].position - active[0].position);
	if (!m_distance) { m_distance = dist; }
	m_delta = dist - *m_distance;
	m_distance = dist;
	return m_delta;
}
} // namespace bave
