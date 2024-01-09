#pragma once
#include <bave/graphics/shape.hpp>
#include <bave/loader.hpp>
#include <tools/applet.hpp>

namespace bave::tools {
class NineSlicer : public Applet {
	void tick() final;

	void on_drop(std::span<std::string const> paths) final;

	void file_menu_items() final;

	void metadata_control();
	void slice_control(NineSlice& out);

	void resize_guides();
	void position_guides();

	void resize_framebuffer();

	auto try_load_path(std::string_view path) -> bool;
	auto load_uri(std::string_view uri) -> bool;

	auto load_image_at(std::string_view uri) -> bool;

	void new_slice();
	auto load_slice(std::string_view uri) -> bool;
	auto try_save_slice(std::string_view path) -> bool;
	void save_slice();

	void set_title();

	[[nodiscard]] auto try_make_uri(std::string_view path) const -> std::string;
	[[nodiscard]] auto make_json_uri() const -> std::string;

	Logger m_log{"NineSlicer"};
	Loader m_loader;

	Ptr<NineQuadShape> m_image_quad{};
	Ptr<QuadShape> m_top{};
	Ptr<QuadShape> m_bottom{};
	Ptr<QuadShape> m_left{};
	Ptr<QuadShape> m_right{};

	bool m_unsaved{};

	std::string m_image_uri{};
	std::string m_json_uri{};

  public:
	NineSlicer(App& app, State state);
};
} // namespace bave::tools
