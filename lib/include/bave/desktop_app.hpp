#pragma once
#include <bave/app.hpp>
#include <bave/game.hpp>
#include <bave/logger.hpp>
#include <bave/platform.hpp>
#include <span>

static_assert(bave::platform_v == bave::Platform::eDesktop);

namespace bave {
class DesktopApp : public IApp {
  public:
	explicit DesktopApp(int argc, char const* const* argv);

	void set_game(std::unique_ptr<Game> game) final;
	auto run() -> ErrCode final;

  private:
	std::span<char const* const> m_args{};
	std::unique_ptr<Game> m_game{};
	Logger m_log{"DesktopApp"};
};
} // namespace bave
