#include <boost/test/unit_test.hpp>
#include "aim/asio/spawn.hpp"
//#include <boost/asio/spawn.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <mutex>

BOOST_AUTO_TEST_SUITE(aimSpawnTest)

BOOST_AUTO_TEST_CASE(get_id_should_not_throw)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	asio::spawn(ios, [&](asio::yield_context) {
		BOOST_CHECK_NO_THROW(asio::this_coro::get_id());
		called = true;
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(
		id_should_remain_the_same_during_the_whole_spawned_function)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id1 = asio::this_coro::get_id();
		boost::asio::deadline_timer t(ios, boost::posix_time::milliseconds(1));
		t.async_wait(yield);
		auto id2 = asio::this_coro::get_id();
		t.async_wait(yield);
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id1, id2);
		BOOST_CHECK_EQUAL(id2, id3);
		called = true;
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(
		get_id_should_return_bool_convertible_false_\
if_called_outside_of_a_spawn_function_and_io_service_run_had_not_run_previously)
{
	using namespace boost;
	BOOST_CHECK(!asio::this_coro::get_id());
}

BOOST_AUTO_TEST_CASE(
		get_id_should_behave_correctly_if_called_outside_of_a_spawn_function)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;
	asio::this_coro::coro_id coroId;

	auto mainId1 = asio::this_coro::get_id();
	asio::spawn(ios, [&](asio::yield_context) {
		coroId = asio::this_coro::get_id();
		BOOST_CHECK(mainId1 != coroId);
		called = true;
	});
	ios.run();
	auto mainId2 = asio::this_coro::get_id();
	BOOST_CHECK(called);
	BOOST_CHECK(coroId != mainId2);
}

BOOST_AUTO_TEST_CASE(
		get_id_should_be_the_same_after_a_nested_spawn)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	asio::spawn(ios, [&](asio::yield_context) {
		auto id2 = asio::this_coro::get_id();
		asio::spawn(ios, [&](asio::yield_context) {
			called = true;
		});
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id2, id3);
	});

	ios.run();
	BOOST_CHECK(called);
}

struct YieldFixture {
	void async_wait(boost::asio::io_service& ios,
			boost::asio::yield_context yield)
	{
		boost::asio::deadline_timer t{ios};
		t.expires_from_now(boost::posix_time::milliseconds{0});
		t.async_wait(yield);
	}
};

BOOST_FIXTURE_TEST_CASE(
		get_id_should_be_the_same_after_a_nested_spawn_\
when_there_is_yield_inside,
		YieldFixture)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	asio::spawn(ios, [&](asio::yield_context) {
		auto id2 = asio::this_coro::get_id();
		asio::spawn(ios, [&](asio::yield_context yield2) {
			async_wait(ios, yield2);
			called = true;
		});
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id2, id3);
	});

	ios.run();
	BOOST_CHECK(called);
}

BOOST_FIXTURE_TEST_CASE(
		get_id_should_be_the_same_after_a_nested_spawn_\
when_there_is_yield_outside,
		YieldFixture)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id2 = asio::this_coro::get_id();
		async_wait(ios, yield);
		asio::spawn(ios, [&](asio::yield_context) {
			called = true;
		});
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id2, id3);
	});

	ios.run();
	BOOST_CHECK(called);
}

BOOST_FIXTURE_TEST_CASE(
		get_id_should_be_the_same_after_a_nested_spawn_\
when_there_is_yield_inside_and_outside,
		YieldFixture)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id2 = asio::this_coro::get_id();
		async_wait(ios, yield);
		asio::spawn(ios, [&](asio::yield_context yield2) {
			async_wait(ios, yield2);
			called = true;
		});
		async_wait(ios, yield);
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id2, id3);
	});

	ios.run();
	BOOST_CHECK(called);
}

BOOST_FIXTURE_TEST_CASE(
		get_id_should_be_the_same_after_a_nested_spawn_\
when_there_are_more_yields_inside_and_outside,
		YieldFixture)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id2 = asio::this_coro::get_id();
		async_wait(ios, yield);
		async_wait(ios, yield);
		asio::spawn(ios, [&](asio::yield_context yield2) {
			async_wait(ios, yield2);
			async_wait(ios, yield2);
			called = true;
		});
		async_wait(ios, yield);
		async_wait(ios, yield);
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id2, id3);
	});

	ios.run();
	BOOST_CHECK(called);
}

BOOST_FIXTURE_TEST_CASE(
		get_id_should_be_the_same_after_a_nested_spawn_\
when_there_is_yield_inside_and_outside\
and_more_coroutines_inside,
		YieldFixture)
{
	using namespace boost;
	asio::io_service ios;
	bool called1 = false;
	bool called2 = false;

	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id2 = asio::this_coro::get_id();
		async_wait(ios, yield);
		asio::spawn(ios, [&](asio::yield_context yield2) {
			async_wait(ios, yield2);
			called1 = true;
		});
		asio::spawn(ios, [&](asio::yield_context yield2) {
			async_wait(ios, yield2);
			called2 = true;
		});
		async_wait(ios, yield);
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id2, id3);
	});

	ios.run();
	BOOST_CHECK(called1);
	BOOST_CHECK(called2);
}

BOOST_FIXTURE_TEST_CASE(
		get_id_should_be_the_same_after_a_nested_spawn_\
when_there_is_yield_inside_and_outside\
and_more_coroutines,
		YieldFixture)
{
	using namespace boost;
	asio::io_service ios;
	bool called1 = false;
	bool called2 = false;

	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id2 = asio::this_coro::get_id();
		async_wait(ios, yield);
		asio::spawn(ios, [&](asio::yield_context yield2) {
			async_wait(ios, yield2);
			called1 = true;
		});
		async_wait(ios, yield);
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id2, id3);
	});
	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id2 = asio::this_coro::get_id();
		async_wait(ios, yield);
		asio::spawn(ios, [&](asio::yield_context yield2) {
			async_wait(ios, yield2);
			called2 = true;
		});
		async_wait(ios, yield);
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id2, id3);
	});

	ios.run();
	BOOST_CHECK(called1);
	BOOST_CHECK(called2);
}

BOOST_FIXTURE_TEST_CASE(
		get_id_should_be_the_same_after_a_nested_spawn_\
when_there_is_yield_inside_and_outside\
and_more_coroutines_inside_and_outside,
		YieldFixture)
{
	using namespace boost;
	asio::io_service ios;
	bool called1 = false;
	bool called2 = false;
	bool called3 = false;
	bool called4 = false;

	BOOST_CHECK(true);
	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id2 = asio::this_coro::get_id();
		async_wait(ios, yield);
		asio::spawn(ios, [&](asio::yield_context yield2) {
			async_wait(ios, yield2);
			called1 = true;
		});
		asio::spawn(ios, [&](asio::yield_context yield2) {
			async_wait(ios, yield2);
			called2 = true;
		});
		async_wait(ios, yield);
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id2, id3);
	});
	BOOST_CHECK(true);
	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id2 = asio::this_coro::get_id();
		async_wait(ios, yield);
		asio::spawn(ios, [&](asio::yield_context yield2) {
			async_wait(ios, yield2);
			called3 = true;
		});
		asio::spawn(ios, [&](asio::yield_context yield2) {
			async_wait(ios, yield2);
			called4 = true;
		});
		async_wait(ios, yield);
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id2, id3);
	});

	BOOST_CHECK(true);
	ios.run();
	BOOST_CHECK(called1);
	BOOST_CHECK(called2);
	BOOST_CHECK(called3);
	BOOST_CHECK(called4);
}

BOOST_AUTO_TEST_CASE(
		id_should_remain_the_same_during_the_whole_spawned_function_\
when_there_are_more_spawned_functions)
{
	using namespace boost;
	asio::io_service ios;
	bool called1 = false;
	bool called2 = false;

	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id1 = asio::this_coro::get_id();
		boost::asio::deadline_timer t(ios, boost::posix_time::milliseconds(1));
		t.async_wait(yield);
		auto id2 = asio::this_coro::get_id();
		t.async_wait(yield);
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id1, id2);
		BOOST_CHECK_EQUAL(id2, id3);
		called1 = true;
	});
	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id1 = asio::this_coro::get_id();
		boost::asio::deadline_timer t(ios, boost::posix_time::milliseconds(1));
		t.async_wait(yield);
		auto id2 = asio::this_coro::get_id();
		t.async_wait(yield);
		auto id3 = asio::this_coro::get_id();
		BOOST_CHECK_EQUAL(id1, id2);
		BOOST_CHECK_EQUAL(id2, id3);
		called2 = true;
	});
	ios.run();
	BOOST_CHECK(called1);
	BOOST_CHECK(called2);
}

BOOST_AUTO_TEST_CASE(
		id_should_remain_the_same_during_the_whole_spawned_function_\
and_ids_should_be_different_in_different_coros)
{
	using namespace boost;
	asio::io_service ios;
	bool called1 = false;
	bool called2 = false;
	asio::this_coro::coro_id idA1, idA2, idA3, idB1, idB2, idB3;

	asio::spawn(ios, [&](asio::yield_context yield) {
		idA1 = asio::this_coro::get_id();
		boost::asio::deadline_timer t(ios, boost::posix_time::milliseconds(1));
		t.async_wait(yield);
		idA2 = asio::this_coro::get_id();
		t.async_wait(yield);
		idA3 = asio::this_coro::get_id();
		called1 = true;
	});
	asio::spawn(ios, [&](asio::yield_context yield) {
		idB1 = asio::this_coro::get_id();
		boost::asio::deadline_timer t(ios, boost::posix_time::milliseconds(1));
		t.async_wait(yield);
		idB2 = asio::this_coro::get_id();
		t.async_wait(yield);
		idB3 = asio::this_coro::get_id();
		called2 = true;
	});
	ios.run();
	BOOST_CHECK(called1);
	BOOST_CHECK(called2);
	BOOST_CHECK_EQUAL(idA1, idA2);
	BOOST_CHECK_EQUAL(idA2, idA3);
	BOOST_CHECK_EQUAL(idB1, idB2);
	BOOST_CHECK_EQUAL(idB2, idB3);
	BOOST_CHECK(idA1 != idB1);
}

BOOST_AUTO_TEST_CASE(
		id_should_remain_the_same_during_the_whole_spawned_function_\
when_there_are_more_spawned_functions_\
and_more_threads)
{
	using namespace boost;
	asio::io_service ios;
	bool called1 = false;
	bool called2 = false;
	bool called3 = false;
	// BOOST_CHECK_EQUAL is not thread safe
	std::mutex mutex;

	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id1 = asio::this_coro::get_id();
		boost::asio::deadline_timer t(ios, boost::posix_time::milliseconds(1));
		t.async_wait(yield);
		auto id2 = asio::this_coro::get_id();
		t.async_wait(yield);
		auto id3 = asio::this_coro::get_id();
		std::unique_lock<std::mutex> lock{mutex};
		BOOST_CHECK_EQUAL(id1, id2);
		BOOST_CHECK_EQUAL(id2, id3);
		called1 = true;
	});
	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id1 = asio::this_coro::get_id();
		boost::asio::deadline_timer t(ios, boost::posix_time::milliseconds(1));
		t.async_wait(yield);
		auto id2 = asio::this_coro::get_id();
		t.async_wait(yield);
		auto id3 = asio::this_coro::get_id();
		std::unique_lock<std::mutex> lock{mutex};
		BOOST_CHECK_EQUAL(id1, id2);
		BOOST_CHECK_EQUAL(id2, id3);
		called2 = true;
	});
	asio::spawn(ios, [&](asio::yield_context yield) {
		auto id1 = asio::this_coro::get_id();
		boost::asio::deadline_timer t(ios, boost::posix_time::milliseconds(1));
		t.async_wait(yield);
		auto id2 = asio::this_coro::get_id();
		t.async_wait(yield);
		auto id3 = asio::this_coro::get_id();
		std::unique_lock<std::mutex> lock{mutex};
		BOOST_CHECK_EQUAL(id1, id2);
		BOOST_CHECK_EQUAL(id2, id3);
		called3 = true;
	});

	boost::thread t1{[&](){ ios.run(); }};
	boost::thread t2{[&](){ ios.run(); }};
	t1.join();
	t2.join();
	BOOST_CHECK(called1);
	BOOST_CHECK(called2);
}


BOOST_AUTO_TEST_SUITE(parent_coro_id)

BOOST_AUTO_TEST_CASE(parent_id_should_be_ok_when_called_outside_spawn)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	auto id1 = asio::this_coro::get_id();
	asio::spawn(ios, [&](asio::yield_context yield) {
		BOOST_CHECK_EQUAL(id1, yield.parent_coro_id_);
		called = true;
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_CASE(parent_id_should_be_ok_when_called_inside_spawn)
{
	using namespace boost;
	asio::io_service ios;
	bool called = false;

	asio::spawn(ios, [&](asio::yield_context) {
		auto id1 = asio::this_coro::get_id();
		asio::spawn(ios, [&](asio::yield_context yield) {
			BOOST_CHECK_EQUAL(id1, yield.parent_coro_id_);
			called = true;
		});
	});
	ios.run();
	BOOST_CHECK(called);
}

BOOST_AUTO_TEST_SUITE_END() // parent_coro_id

BOOST_AUTO_TEST_SUITE_END()


