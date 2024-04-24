#include <physfs.h>
#include <bave/core/ptr.hpp>
#include <bave/core/scoped_resource.hpp>
#include <bave/core/string_hash.hpp>
#include <bave/io/zip_io.hpp>
#include <bave/logger.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace bave {
namespace {
struct ZipFs {
	struct Deleter {
		void operator()(Ptr<PHYSFS_File> file) const noexcept { PHYSFS_close(file); }
		void operator()(int /*i*/) const noexcept { PHYSFS_deinit(); }
	};

	explicit ZipFs() : m_instance(PHYSFS_init(nullptr)) {
		if (m_instance == 0) { m_log.warn("failed to initialize ZipFs"); }
	}

	[[nodiscard]] auto is_init() const -> bool { return m_instance != 0; }

  private:
	Logger m_log{"ZipFs"};
	ScopedResource<int, Deleter> m_instance{};
};

struct {
	std::optional<ZipFs> zip_fs{};
	std::mutex mutex{};
	std::unordered_map<std::string, std::vector<std::byte>, StringHash, std::equal_to<>> zips{};

	auto ensure_init() -> bool {
		auto lock = std::scoped_lock{mutex};
		if (!zip_fs) { zip_fs.emplace(); }
		return zip_fs->is_init();
	}

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
} g_state{};

auto open_read(CString const path) { return std::unique_ptr<PHYSFS_File, ZipFs::Deleter>{PHYSFS_openRead(path.c_str())}; }
} // namespace

auto zip::mount(std::string name, std::vector<std::byte> zip_bytes) -> bool {
	if (!g_state.ensure_init()) { return false; }

	if (PHYSFS_mountMemory(zip_bytes.data(), zip_bytes.size(), nullptr, name.c_str(), "", 1) == 0) { return false; }

	auto lock = std::scoped_lock{g_state.mutex};
	g_state.zips.insert_or_assign(std::move(name), std::move(zip_bytes));
	return true;
}

auto zip::unmount(std::string const& name) -> bool {
	if (!g_state.ensure_init()) { return false; }

	if (PHYSFS_unmount(name.c_str()) == 0) { return false; }

	auto lock = std::scoped_lock{g_state.mutex};
	g_state.zips.erase(name);
	return true;
}

auto zip::is_mounted(std::string_view const name) -> bool {
	auto lock = std::scoped_lock{g_state.mutex};
	return g_state.zips.contains(name);
}

auto zip::exists(CString const path) -> bool {
	if (!g_state.ensure_init()) { return false; }

	return open_read(path) != nullptr;
}

auto zip::read_bytes(std::vector<std::byte>& out, CString const path) -> bool {
	if (!g_state.ensure_init()) { return false; }

	auto file = open_read(path);
	if (!file) { return false; }

	auto const length = PHYSFS_fileLength(file.get());
	if (length < 0) { return false; }

	auto const size = static_cast<std::size_t>(length);
	out.resize(size);
	PHYSFS_readBytes(file.get(), out.data(), size);
	return true;
}
} // namespace bave
