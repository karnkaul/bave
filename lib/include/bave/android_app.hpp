#pragma once
#include <bave/app.hpp>
#include <bave/game.hpp>
#include <bave/logger.hpp>
#include <bave/platform.hpp>

static_assert(bave::platform_v == bave::Platform::eAndroid);

extern "C" {
struct android_app;
}

namespace bave {
class AndroidApp : public IApp { // NOLINT(cppcoreguidelines-special-member-functions)
  public:
	explicit AndroidApp(android_app& app);
	~AndroidApp() override;

	void set_game(std::unique_ptr<Game> game) final;
	auto run() -> ErrCode final;

  private:
	android_app& m_app;

	std::unique_ptr<Game> m_game{};
	Logger m_log{"AndroidApp"};
};
} // namespace bave
