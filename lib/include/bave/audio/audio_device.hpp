#pragma once
#include <bave/audio/audio_clip.hpp>
#include <bave/core/time.hpp>
#include <bave/logger.hpp>
#include <capo/device.hpp>
#include <vector>

namespace bave {
using SoundSource = capo::SoundSource;
using StreamSource = capo::StreamSource;

class AudioDevice {
  public:
	static constexpr std::size_t max_sfxs_v{16};

	[[nodiscard]] auto make_sound_source() const -> SoundSource { return m_device.make_sound_source(); }
	[[nodiscard]] auto make_stream_source() const -> StreamSource { return m_device.make_stream_source(); }

	[[nodiscard]] auto get_max_sfxs() const -> std::size_t { return m_max_sfxs; }
	void set_max_sfxs(std::size_t count);

	void play_once(AudioClip const& clip, float gain = 1.0f) const;
	[[nodiscard]] auto play_looped(AudioClip const& clip) const -> SoundSource;

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
