#pragma once

namespace bave {
class Pinned {
  public:
	Pinned(Pinned const&) = delete;
	Pinned(Pinned&&) = delete;
	auto operator=(Pinned const&) -> Pinned& = delete;
	auto operator=(Pinned&&) -> Pinned& = delete;

	Pinned() = default;
	~Pinned() = default;
};
} // namespace bave
