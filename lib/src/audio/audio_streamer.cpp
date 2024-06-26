#include <bave/audio/audio_streamer.hpp>
#include <algorithm>

namespace bave {
namespace {
void setup_stream(AudioDevice const& audio_device, StreamSource& out) {
	out = audio_device.make_stream_source();
	out.set_looping(true);
}
} // namespace

AudioStreamer::AudioStreamer(AudioDevice const& audio_device) {
	setup_stream(audio_device, m_primary.stream);
	setup_stream(audio_device, m_secondary.stream);
}

void AudioStreamer::play(NotNull<std::shared_ptr<AudioClip>> const& clip, Seconds cross_fade) {
	std::swap(m_primary, m_secondary);
	m_primary.stream.stop();
	m_primary.clip = clip;
	m_primary.stream.set_stream(m_primary.clip->get_clip());

	if (cross_fade <= 0s) {
		m_secondary.stream.stop();
		m_primary.gain = 1.0f;
		m_primary.stream.set_gain(gain);
		m_primary.stream.play();
		m_transition.reset();
		return;
	}

	m_primary.gain = 0.0f;
	m_primary.stream.set_gain(0.0f);
	m_primary.stream.play();
	m_transition = Transition{.total = cross_fade, .fadeout_gain = m_secondary.gain};
}

void AudioStreamer::stop(Seconds const fadeout) {
	m_secondary.stream.stop();

	if (get_state() == AudioState::ePaused || fadeout <= 0s) {
		m_primary.stream.stop();
		m_transition.reset();
		return;
	}

	std::swap(m_primary, m_secondary);

	m_transition = Transition{.total = fadeout, .fadeout_gain = m_secondary.gain};
}

auto AudioStreamer::pause() -> std::optional<Pause> {
	if (get_state() != AudioState::ePlaying) { return {}; }
	m_primary.stream.pause();
	return Pause{};
}

void AudioStreamer::resume(Pause /*pause*/) { m_primary.stream.play(); }

void AudioStreamer::tick(Seconds const dt) {
	if (!m_transition) {
		m_primary.stream.set_gain(m_primary.gain * gain);
		return;
	}

	m_transition->elapsed += dt;
	auto const ratio = std::clamp(m_transition->elapsed / m_transition->total, 0.0f, 1.0f);
	m_primary.gain = ratio;
	m_secondary.gain = std::lerp(m_transition->fadeout_gain, 0.0f, ratio);

	m_primary.stream.set_gain(m_primary.gain * gain);
	m_secondary.stream.set_gain(m_secondary.gain * gain);

	if (m_transition->elapsed >= m_transition->total) {
		m_secondary.stream.stop();
		m_transition.reset();
	}
}
} // namespace bave
