#pragma once
#include <bave/audio/audio_device.hpp>
#include <bave/core/not_null.hpp>
#include <bave/core/time.hpp>
#include <memory>
#include <optional>

namespace bave {
class AudioStreamer {
  public:
	struct Pause {};

	AudioStreamer(AudioDevice const& audio_device);

	void play(NotNull<std::shared_ptr<AudioClip>> const& clip, Seconds cross_fade = 1s);
	void stop() { m_primary.stream.stop(); }
	void seek(Seconds time) { m_primary.stream.seek(time); }

	[[nodiscard]] auto pause() -> std::optional<Pause>;
	void resume(Pause);

	[[nodiscard]] auto get_cursor() const -> Seconds { return m_primary.stream.cursor(); }
	[[nodiscard]] auto get_state() const -> AudioState { return m_primary.stream.state(); }

	void tick(Seconds dt);

	//! Master gain for streaming audio.
	float gain{1.0f};

  private:
	struct Track {
		std::shared_ptr<AudioClip> clip{};
		StreamSource stream{};
		float gain{};
	};

	struct Transition {
		Seconds total{};
		Seconds elapsed{};
		float fadeout_gain{};
	};

	Track m_primary{};
	Track m_secondary{};
	std::optional<Transition> m_transition{};
};
} // namespace bave
