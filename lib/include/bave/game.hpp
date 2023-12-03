#pragma once
#include <bave/app.hpp>
#include <bave/core/polymorphic.hpp>
#include <bave/logger.hpp>

namespace bave {
class Game : public PolyPinned {
  public:
	explicit Game(IApp& app) : m_app(app) {}

	[[nodiscard]] auto get_app() const -> IApp& { return m_app; }

  private:
	IApp& m_app;
};
} // namespace bave
