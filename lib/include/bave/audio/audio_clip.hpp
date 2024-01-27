#pragma once
#include <capo/pcm.hpp>
#include <capo/state.hpp>

namespace bave {
/// \brief Compression of a PCM datastream.
using Compression = capo::Compression;
/// \brief State of audio source.
using AudioState = capo::State;

/// \brief Storage for PCM audio.
class AudioClip {
  public:
	/// \brief Attempt to load PCM data.
	/// \param bytes Compressed bytestream.
	/// \param compression Type of audio compression.
	/// \returns true if successful.
	auto load_from_bytes(std::span<std::byte const> bytes, Compression compression = Compression::eUnknown) -> bool;

	/// \brief Obtain a clip (view) into the stored PCM data.
	/// \returns View into stored PCM data.
	[[nodiscard]] auto get_clip() const -> capo::Clip { return m_pcm.pcm.clip(); }

	explicit operator bool() const { return !!m_pcm; }

  private:
	capo::Pcm::Result m_pcm{};
};
} // namespace bave
