#pragma once
#include <capo/pcm.hpp>
#include <capo/state.hpp>

namespace bave {
using Compression = capo::Compression;
using AudioState = capo::State;

class AudioClip {
  public:
	auto load_from_bytes(std::span<std::byte const> bytes, Compression compression = Compression::eUnknown) -> bool;

	[[nodiscard]] auto get_clip() const -> capo::Clip { return m_pcm.pcm.clip(); }

	explicit operator bool() const { return !!m_pcm; }

  private:
	capo::Pcm::Result m_pcm{};
};
} // namespace bave
