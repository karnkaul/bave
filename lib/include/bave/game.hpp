#pragma once
#include <bave/app.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/logger.hpp>

namespace bave {
class Game : public PolyPinned {
  public:
	explicit Game(App& app) : m_app(app) {}

	[[nodiscard]] auto get_app() const -> App& { return m_app; }

	virtual void tick() {}

	virtual void shutdown() {}

  private:
	App& m_app;
};
} // namespace bave
