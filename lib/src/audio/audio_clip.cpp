#include <bave/audio/audio_clip.hpp>

namespace bave {
auto AudioClip::load_from_bytes(std::span<std::byte const> bytes, Compression compression) -> bool {
	auto result = capo::Pcm::make(bytes, compression);
	if (!result) { return false; }
	m_pcm = std::move(result);
	return true;
}
} // namespace bave
