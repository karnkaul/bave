#pragma once
#include <bave/audio/audio_clip.hpp>
#include <bave/core/time.hpp>
#include <bave/logger.hpp>
#include <capo/device.hpp>
#include <vector>

namespace bave {
/// \brief Audio source for sounds (SFX).
using SoundSource = capo::SoundSource;
/// \brief Audio source for streams (music).
using StreamSource = capo::StreamSource;

/// \brief Audio device.
class AudioDevice {
  public:
	/// \brief Default maximum managed SFXs that will play simultaneously.
	static constexpr std::size_t max_sfxs_v{16};

	/// \brief Create a new SoundSource.
	/// \returns SoundSource instance.
	[[nodiscard]] auto make_sound_source() const -> SoundSource { return m_device.make_sound_source(); }
	/// \brief Create a new StreamSource.
	/// \returns StreamSource instance.
	[[nodiscard]] auto make_stream_source() const -> StreamSource { return m_device.make_stream_source(); }

	/// \brief Obtain the maximum number of managed SFXs.
	/// \returns Number of maximum managed SFXs.
	[[nodiscard]] auto get_max_sfxs() const -> std::size_t { return m_max_sfxs; }
	/// \brief Set the maximum number of managed SFXs.
	/// \param count Number of maximum managed SFXs.
	void set_max_sfxs(std::size_t count);

	/// \brief Play an audio clip once using a managed SFX.
	/// \param clip Clip to play.
	/// \param gain Gain to play at.
	///
	/// Clip data is copied to the native audio device before playback,
	/// so it does not need to be stored on the user side.
	void play_once(AudioClip const& clip, float gain = 1.0f) const;
	/// \brief Loop an audio clip.
	/// \param clip Clip to play.
	/// \returns SoundSource that will be playing the clip.
	///
	/// The clip will keep looping until the returned SoundSource is destroyed / modified.
	[[nodiscard]] auto play_looped(AudioClip const& clip) const -> SoundSource;

	/// \brief Master gain for managed SFX.
	float sfx_gain{1.0f};

  private:
	struct ErrLog {
		ErrLog() noexcept;
	};

	struct Sfx {
		SoundSource source{};
		Clock::time_point start{};
	};

	auto get_sfx() const -> Sfx&;

	ErrLog m_err_log{};
	Logger m_log{"AudioDevice"};
	capo::Device m_device{};

	mutable std::vector<Sfx> m_sfxs{};
	std::size_t m_max_sfxs{max_sfxs_v};
};
} // namespace bave
