#include <boost/test/unit_test.hpp>
#include <boost/asio.hpp>
#include "logging/spawn.hpp"
#include "logging/log.hpp"
#include "testutil/checkEqualRanges.hpp"

namespace unitTest {

struct FixtureConnector {
	logging::detail::CoroSpecificLogStringStack::Datas&
	getDatas(logging::detail::CoroSpecificLogStringStack& stack) {
		return stack.datas;
	}
};

} // unitTest

BOOST_AUTO_TEST_SUITE(loggingSpawnTest)

BOOST_AUTO_TEST_CASE(spawn_should_work_with_io_servie)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	logging::spawn(ios, [&](asio::yield_context) {
		called = true;
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(spawn_should_work_with_strand)
{
	using namespace boost;
	asio::io_service ios;
	asio::strand strand{ios};
	bool called = false;

	logging::spawn(strand, [&](asio::yield_context) {
		called = true;
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(spawn_should_work_with_yield_context)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	logging::spawn(ios, [&](asio::yield_context yield) {
		logging::spawn(yield, [&](asio::yield_context) {
			called = true;
		});
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(nested_log_spawns_should_work)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	logging::spawn(ios, [&called](asio::yield_context yield) {
		logging::spawn(yield, [&called](asio::yield_context) {
			called = true;
		});
	});
	ios.run();
	BOOST_CHECK(called);
}




BOOST_AUTO_TEST_CASE(log_string_should_be_pushed_outside_spawn)
{
	LOGGING_SCOPED_CORO_STR("a");
	auto expected = {"a"};
	TESTUTIL_CHECK_EQUAL_RANGES(expected,
		logging::detail::stack.get());
}

BOOST_AUTO_TEST_CASE(log_string_should_be_poppped_at_end_of_scope_\
when_outside_spawn)
{
	{
		LOGGING_SCOPED_CORO_STR("a");
		auto expected = {"a"};
		TESTUTIL_CHECK_EQUAL_RANGES(expected,
			logging::detail::stack.get());
	}
	auto expected = std::vector<std::string>();
	TESTUTIL_CHECK_EQUAL_RANGES(expected,
		logging::detail::stack.get());
}

BOOST_AUTO_TEST_CASE(log_string_should_be_nested_when_outside_spawn)
{
	LOGGING_SCOPED_CORO_STR("a");
	{
		LOGGING_SCOPED_CORO_STR("b");
		auto expected = {"a", "b"};
		TESTUTIL_CHECK_EQUAL_RANGES(expected,
			logging::detail::stack.get());
	}
}

BOOST_AUTO_TEST_CASE(log_string_should_be_pushed_inside_spawn)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	logging::spawn(ios, [&](asio::yield_context) {
		LOGGING_SCOPED_CORO_STR("a");
		auto expected = {"a"};
		TESTUTIL_CHECK_EQUAL_RANGES(expected,
			logging::detail::stack.get());
		called = true;
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_FIXTURE_TEST_CASE(stack_should_be_erased_when_coro_ends,
		unitTest::FixtureConnector)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;
	const auto& stackDatas = getDatas(logging::detail::stack);
	const auto initialSize = stackDatas.size();

	logging::spawn(ios, [&](asio::yield_context) {
		LOGGING_SCOPED_CORO_STR("a");
		BOOST_CHECK_EQUAL(stackDatas.size(), initialSize + 1);
		{
			LOGGING_SCOPED_CORO_STR("b");
			BOOST_CHECK_EQUAL(stackDatas.size(), initialSize + 1);
		}
		logging::spawn(ios, [&](asio::yield_context) {
			LOGGING_SCOPED_CORO_STR("c");
			BOOST_CHECK_EQUAL(stackDatas.size(), initialSize + 2);
			called = true;
		});
		BOOST_CHECK_EQUAL(stackDatas.size(), initialSize + 1);
	});

	BOOST_CHECK_EQUAL(stackDatas.size(), initialSize);
	ios.run();
	BOOST_CHECK_EQUAL(stackDatas.size(), initialSize);
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(log_string_should_be_poppped_at_end_of_scope_\
when_inside_spawn)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	logging::spawn(ios, [&](asio::yield_context) {
		{
			LOGGING_SCOPED_CORO_STR("a");
			auto expected = {"a"};
			TESTUTIL_CHECK_EQUAL_RANGES(expected,
				logging::detail::stack.get());
		}
		auto expected = std::vector<std::string>();
		TESTUTIL_CHECK_EQUAL_RANGES(expected,
			logging::detail::stack.get());
		called = true;
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(log_string_should_be_nested_when_inside_spawn)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	logging::spawn(ios, [&](asio::yield_context) {
		LOGGING_SCOPED_CORO_STR("a");
		{
			LOGGING_SCOPED_CORO_STR("b");
			auto expected = {"a", "b"};
			TESTUTIL_CHECK_EQUAL_RANGES(expected,
				logging::detail::stack.get());
		}
		called = true;
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(log_stack_should_be_passed_to_child)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	LOGGING_SCOPED_CORO_STR("a");
	logging::spawn(ios, [&called](asio::yield_context yield) {

		auto expected = {"a"};
		TESTUTIL_CHECK_EQUAL_RANGES(expected, logging::detail::stack.get());

		LOGGING_SCOPED_CORO_STR("b");
		expected = {"a", "b"};
		TESTUTIL_CHECK_EQUAL_RANGES(expected, logging::detail::stack.get());

		logging::spawn(yield, [&called](asio::yield_context) {
			auto expected = {"a", "b"};
			TESTUTIL_CHECK_EQUAL_RANGES(expected, logging::detail::stack.get());
			called = true;

			LOGGING_SCOPED_CORO_STR("c");
			expected = {"a", "b", "c"};
			TESTUTIL_CHECK_EQUAL_RANGES(expected, logging::detail::stack.get());
		});

		expected = {"a", "b"};
		TESTUTIL_CHECK_EQUAL_RANGES(expected, logging::detail::stack.get());
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(passing_log_string_through_io_service_post)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	LOGGING_SCOPED_CORO_STR("a");
	logging::spawn(ios, [&](asio::yield_context) {
			LOGGING_SCOPED_CORO_STR("b");
			auto coroLogStrStack = logging::getCoroSpecificLogStrStack();
			ios.post([coroLogStrStack, &called](){
				LOGGING_SCOPED_CORO_STR_STACK(coroLogStrStack);
				auto expected = {"a", "b"};
				TESTUTIL_CHECK_EQUAL_RANGES(expected, logging::detail::stack.get());
				called = true;
			});
			auto expected = {"a", "b"};
			TESTUTIL_CHECK_EQUAL_RANGES(expected, logging::detail::stack.get());
	});

	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(passing_log_string_through_io_logging_post)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	LOGGING_SCOPED_CORO_STR("a");
	logging::spawn(ios, [&](asio::yield_context) {
			LOGGING_SCOPED_CORO_STR("b");
			auto coroLogStrStack = logging::getCoroSpecificLogStrStack();
			logging::post(ios, [coroLogStrStack, &called](){
				auto expected = {"a", "b"};
				TESTUTIL_CHECK_EQUAL_RANGES(expected, logging::detail::stack.get());
				called = true;
			});
			auto expected = {"a", "b"};
			TESTUTIL_CHECK_EQUAL_RANGES(expected, logging::detail::stack.get());
	});

	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(display_test)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	logging::addCoroSpecificLogAttribute();
	logging::Logger logger;
	setClass(logger,"Class");
	using Sev = logging::Severity;
	BOOST_LOG_SEV(logger, Sev::info) << "main before scope a";

	LOGGING_SCOPED_CORO_STR("a");
	BOOST_LOG_SEV(logger, Sev::info) << "main after scope a";
	logging::spawn(ios, [&logger, &called](asio::yield_context yield) {
		BOOST_LOG_SEV(logger, Sev::info) << "yield1 before scope b";
		LOGGING_SCOPED_CORO_STR("b");
		BOOST_LOG_SEV(logger, Sev::info) << "yield1 after scope b";
		logging::spawn(yield, [&logger, &called](asio::yield_context) {
			called = true;
			BOOST_LOG_SEV(logger, Sev::info) << "yield2 before scope c";
			LOGGING_SCOPED_CORO_STR("c");
			BOOST_LOG_SEV(logger, Sev::info) << "yield2 after scope c";
		});
		BOOST_LOG_SEV(logger, Sev::info) << "yield1 after yield2";
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_SUITE_END()

