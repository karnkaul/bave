#pragma once
#include <bave/data_loader.hpp>
#include <bave/io/zip_io.hpp>

namespace bave {
class ZipLoader : public IDataLoader {
	[[nodiscard]] auto exists(std::string_view uri) const -> bool final { return zip::exists(std::string{uri}.c_str()); }
	[[nodiscard]] auto read_bytes(std::vector<std::byte>& out, std::string_view uri) const -> bool final {
		return zip::read_bytes(out, std::string{uri}.c_str());
	}
};
} // namespace bave
