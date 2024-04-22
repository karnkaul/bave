#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <platform/android/android_data_loader.hpp>
#include <memory>

namespace bave {
namespace {
struct Deleter {
	void operator()(AAsset* asset) const noexcept {
		if (asset == nullptr) { return; }
		AAsset_close(asset);
	}
};

[[nodiscard]] auto open_asset(AAssetManager* manager, CString path) -> std::unique_ptr<AAsset, Deleter> {
	return std::unique_ptr<AAsset, Deleter>{AAssetManager_open(manager, path.c_str(), AASSET_MODE_BUFFER)};
}

template <typename Type>
auto do_read_data(android_app* app, Type& out, CString path) -> bool {
	auto asset = open_asset(app->activity->assetManager, path);
	if (asset == nullptr) { return false; }
	auto const* data = AAsset_getBuffer(asset.get());
	auto const size = AAsset_getLength(asset.get());
	out.resize(static_cast<size_t>(size));
	if (size > 0) { std::memcpy(out.data(), data, out.size()); }
	return true;
}
} // namespace

auto AndroidDataLoader::exists(CString path) const -> bool { return open_asset(m_app->activity->assetManager, path) != nullptr; }

auto AndroidDataLoader::read_bytes(std::vector<std::byte>& out, CString path) const -> bool { return do_read_data(m_app, out, path); }

auto AndroidDataLoader::read_string(std::string& out, CString path) const -> bool { return do_read_data(m_app, out, path); }
} // namespace bave
