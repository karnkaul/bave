#pragma once
#include <bave/core/time.hpp>
#include <span>
#include <string>

namespace bave {
class SpriteAnimation {
  public:
	struct KeyFrame {
		std::string tile_id{};
		Seconds timestamp{};
	};

	explicit SpriteAnimation(std::vector<KeyFrame> key_frames, Seconds duration);
	explicit SpriteAnimation(std::vector<std::string> tile_ids, Seconds duration);

	[[nodiscard]] auto get_tile_at(Seconds timestamp) const -> std::string_view;

	[[nodiscard]] auto get_key_frames() const -> std::span<KeyFrame const> { return m_key_frames; }
	[[nodiscard]] auto get_duration() const -> Seconds { return m_duration; }

  private:
	void sort_keyframes();

	std::vector<KeyFrame> m_key_frames{};
	Seconds m_duration{};
};
} // namespace bave
