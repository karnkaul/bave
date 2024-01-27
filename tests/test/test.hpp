#pragma once
#include <cstdint>
#include <string_view>

namespace test {
struct SrcLoc {
	std::int64_t line_{};
	std::string_view file_name_{};

	[[nodiscard]] constexpr auto line() const { return line_; }
	[[nodiscard]] constexpr auto file_name() const { return file_name_; }
};

class Test {
  public:
	Test();
	Test(Test const&) = default;
	Test(Test&&) = default;
	auto operator=(Test const&) -> Test& = default;
	auto operator=(Test&&) -> Test& = default;

	virtual ~Test() = default;

	[[nodiscard]] virtual auto get_name() const -> std::string_view = 0;
	virtual void run() const = 0;

  protected:
	static void do_expect(bool pred, std::string_view expr, SrcLoc const& location);
	static void do_assert(bool pred, std::string_view expr, SrcLoc const& location);
};
} // namespace test

#define EXPECT(pred) do_expect(pred, #pred, {__LINE__, __FILE__}) // NOLINT(cppcoreguidelines-macro-usage)
#define ASSERT(pred) do_assert(pred, #pred, {__LINE__, __FILE__}) // NOLINT(cppcoreguidelines-macro-usage)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADD_TEST(Class)                                                                                                                                        \
	struct Test_##Class : ::test::Test {                                                                                                                       \
		void run() const final;                                                                                                                                \
		auto get_name() const -> std::string_view final { return #Class; }                                                                                     \
	};                                                                                                                                                         \
	inline Test_##Class const g_test_##Class{};                                                                                                                \
	inline void Test_##Class::run() const
