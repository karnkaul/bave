#include <bave/audio/audio_device.hpp>
#include <capo/error_handler.hpp>
#include <algorithm>
#include <cassert>

namespace bave {
AudioDevice::ErrLog::ErrLog() noexcept {
	capo::set_error_handler([](std::string_view msg) {
		static auto const log = Logger{"AudioDevice"};
		log.error("{}", msg);
	});
}

void AudioDevice::set_max_sfxs(std::size_t const count) {
	if (count == 0) { return; }
	m_max_sfxs = count;
}

void AudioDevice::play_once(AudioClip const& clip, float gain) const {
	gain *= sfx_gain;
	if (gain <= 0.0f || !clip) { return; }

	auto& sfx = get_sfx();
	sfx.source.set_clip(clip.get_clip());
	sfx.source.set_gain(gain);
	sfx.source.play();
	sfx.start = Clock::now();
}

auto AudioDevice::play_looped(AudioClip const& clip) const -> SoundSource {
	if (!clip) { return {}; }

	auto ret = make_sound_source();
	ret.set_gain(sfx_gain);
	ret.set_clip(clip.get_clip());
	ret.set_looping(true);
	ret.play();
	return ret;
}

auto AudioDevice::get_sfx() const -> Sfx& {
	auto const it = std::find_if(m_sfxs.begin(), m_sfxs.end(), [](Sfx const& sfx) { return sfx.source.state() != AudioState::ePlaying; });
	if (it != m_sfxs.end()) { return *it; }

	if (m_sfxs.size() < m_max_sfxs) {
		m_sfxs.push_back(Sfx{.source = make_sound_source()});
		return m_sfxs.back();
	}

	assert(!m_sfxs.empty());
	auto oldest = Clock::now();
	auto index = std::size_t{};
	for (std::size_t i = 0; i < m_sfxs.size(); ++i) {
		auto const& sfx = m_sfxs.at(i);
		if (sfx.start < oldest) {
			oldest = sfx.start;
			index = i;
		}
	}
	auto& ret = m_sfxs.at(index);
	ret.source.stop();
	return ret;
}
} // namespace bave
