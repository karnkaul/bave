#include <bave/font/font_library.hpp>
#include <font/detail/freetype.hpp>

namespace bave {
auto FontLibrary::make() -> std::unique_ptr<FontLibrary> {
	auto ret = std::unique_ptr<FontLibrary>{};
#if defined(BAVE_USE_FREETYPE)
	ret = std::make_unique<detail::Freetype>();
#else
	ret = std::make_unique<FontLibrary::Null>();
#endif
	return ret;
}
} // namespace bave
