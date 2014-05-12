#ifndef OBSERV_TEST_TESTLOGSINK_HPP_
#define OBSERV_TEST_TESTLOGSINK_HPP_

#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/test/unit_test.hpp>
#include "logging/log.hpp"

namespace testutil {

class TestLogSink: public boost::log::sinks::basic_formatted_sink_backend<
		char, boost::log::sinks::synchronized_feeding>
{
public:
	 void consume(boost::log::record_view const&, string_type const& log)
	 {
		 BOOST_MESSAGE(log);
	 }
};

struct TestLogSinkInitializer {
	TestLogSinkInitializer()
	{
		boost::log::add_common_attributes();

		typedef boost::log::sinks::synchronous_sink< TestLogSink > text_sink;
		boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

		sink->set_formatter
		(
			logging::detail::defaultLogExpression
		);

		boost::log::core::get()->add_sink(sink);
	}
};

}  // namespace testutil


#endif /* OBSERV_TEST_TESTLOGSINK_HPP_ */
