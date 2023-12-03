#pragma once

namespace bave {
class Polymorphic {
  public:
	Polymorphic(Polymorphic const&) = default;
	Polymorphic(Polymorphic&&) = default;
	auto operator=(Polymorphic const&) -> Polymorphic& = default;
	auto operator=(Polymorphic&&) -> Polymorphic& = default;

	Polymorphic() = default;
	virtual ~Polymorphic() = default;
};

class PolyPinned {
  public:
	PolyPinned(PolyPinned const&) = delete;
	PolyPinned(PolyPinned&&) = delete;
	auto operator=(PolyPinned const&) -> PolyPinned& = delete;
	auto operator=(PolyPinned&&) -> PolyPinned& = delete;

	PolyPinned() = default;
	virtual ~PolyPinned() = default;
};
} // namespace bave
