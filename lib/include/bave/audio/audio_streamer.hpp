#pragma once
#include <bave/audio/audio_device.hpp>
#include <bave/core/not_null.hpp>
#include <bave/core/time.hpp>
#include <memory>
#include <optional>

namespace bave {
/// \brief Audio streamer with crossfade support.
class AudioStreamer {
  public:
	/// \brief Handle for a paused stream.
	struct Pause {};

	/// \brief Implicit constructor.
	/// \param audio_device AudioDevice.
	/*implicit*/ AudioStreamer(AudioDevice const& audio_device);

	/// \brief Start playing an audio clip.
	/// \param clip Clip to play.
	/// \param cross_fade Time to cross fade to full gain.
	void play(NotNull<std::shared_ptr<AudioClip>> const& clip, Seconds cross_fade = 1s);
	/// \brief Stop playback(if playing a stream).
	void stop() { m_primary.stream.stop(); }
	/// \brief Seek to time (if playing a stream).
	/// \param time Position to seek to.
	void seek(Seconds time) { m_primary.stream.seek(time); }

	/// \brief Pause playback (if playing a stream).
	/// \returns Token that must be passed back in resume().
	[[nodiscard]] auto pause() -> std::optional<Pause>;
	/// \brief Resume playback (if paused).
	void resume(Pause);

	/// \brief Obtain the current playback position.
	/// \returns Playback position in Seconds.
	[[nodiscard]] auto get_cursor() const -> Seconds { return m_primary.stream.cursor(); }
	/// \brief Obtain the current playback state.
	/// \returns Playback state.
	[[nodiscard]] auto get_state() const -> AudioState { return m_primary.stream.state(); }

	/// \brief Update (for cross fading).
	/// \param dt Time elapsed since last call.
	void tick(Seconds dt);

	/// \brief Master gain for streaming audio.
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
