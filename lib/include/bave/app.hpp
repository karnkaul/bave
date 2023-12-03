#pragma once
#include <bave/build_version.hpp>
#include <bave/core/polymorphic.hpp>
#include <memory>

namespace bave {
class Game;

enum struct ErrCode : int { eSuccess = 0, eFailure = 1 };

class IApp : public PolyPinned {
  public:
	virtual void set_game(std::unique_ptr<Game> game) = 0;
	virtual auto run() -> ErrCode = 0;
};
} // namespace bave
