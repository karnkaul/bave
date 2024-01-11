#pragma once
#include <bave/graphics/shape.hpp>
#include <bave/loader.hpp>
#include <tools/applet.hpp>

namespace bave::tools {
class NineSlicer : public Applet {
	void tick() final;

	void on_drop(std::span<std::string const> paths) final;

	void file_menu_items() final;

	void slice_control(NineSlice& out);
	void metadata_control();

	void resize_guides();
	void position_guides();

	auto load_uri(std::string_view uri) -> bool;

	auto load_image_at(std::string_view uri) -> bool;

	void new_slice();
	auto load_slice(std::string_view uri) -> bool;
	void save_slice();

	void set_title();

	Logger m_log{"NineSlicer"};
	Loader m_loader;

	Ptr<NineQuadShape> m_image_quad{};
	Ptr<QuadShape> m_top{};
	Ptr<QuadShape> m_bottom{};
	Ptr<QuadShape> m_left{};
	Ptr<QuadShape> m_right{};

	std::string m_image_uri{};
	std::string m_json_uri{};

	bool m_unsaved{};

  public:
	NineSlicer(App& app, NotNull<std::shared_ptr<State>> const& state);
};
} // namespace bave::tools
