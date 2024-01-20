#pragma once
#include <bave/graphics/detail/buffer_type.hpp>
#include <bave/graphics/detail/buffering.hpp>
#include <bave/graphics/detail/render_resource.hpp>
#include <bave/logger.hpp>
#include <array>
#include <vector>

namespace bave::detail {
class BufferCache {
  public:
	explicit BufferCache(NotNull<RenderDevice*> render_device);

	auto allocate(BufferType type) -> RenderBuffer&;

	[[nodiscard]] auto get_empty(BufferType type) const -> RenderBuffer const& { return m_empty_buffers.at(static_cast<std::size_t>(type)); }

	[[nodiscard]] auto or_empty(Ptr<RenderBuffer const> buffer, BufferType type) const -> RenderBuffer const& {
		return buffer == nullptr ? get_empty(type) : *buffer;
	}

	auto next_frame() -> void;
	auto clear() -> void;

  private:
	struct Pool {
		std::vector<RenderBuffer> buffers{};
		std::size_t next{};
	};

	static constexpr auto types_count_v = static_cast<std::size_t>(BufferType::eCOUNT_);

	using Map = std::array<Pool, types_count_v>;

	Logger m_log{"BufferCache"};
	NotNull<RenderDevice*> m_render_device;
	std::array<RenderBuffer, types_count_v> m_empty_buffers;
	Buffered<Map> m_maps{};
};
} // namespace bave::detail
