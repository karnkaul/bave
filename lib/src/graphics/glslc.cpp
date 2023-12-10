#include <bave/logger.hpp>
#include <bave/shell.hpp>
#include <graphics/glslc.hpp>
#include <filesystem>

namespace bave {
namespace {
namespace fs = std::filesystem;

auto const g_log = Logger{"glslc"};
} // namespace

auto glslc::is_online() -> bool { return shell::exec_cmd_silent("glslc --version"); }

auto glslc::compile(std::string_view const glsl, std::string_view const spir_v) -> bool {
	if (!is_online()) {
		g_log.error("compiler not online");
		return false;
	}
	if (!fs::exists(glsl)) {
		g_log.error("source GLSL not found: '{}'", glsl);
		return false;
	}
	if (spir_v.empty()) {
		g_log.error("empty destination for SPIR-V");
		return false;
	}

	if (!shell::exec_cmd(fmt::format("glslc {} -o {}", glsl, spir_v).c_str())) {
		g_log.error("failed to compile GLSL: '{}'", glsl);
		return false;
	}

	g_log.info("compiled SPIR-V: '{}'", spir_v);
	return true;
}

auto glslc::make_spir_v_path(std::string_view const glsl) -> std::string { return fmt::format("{}.spv", glsl); }
} // namespace bave
