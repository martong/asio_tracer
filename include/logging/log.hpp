#ifndef LOGGING_LOG_HPP_
#define LOGGING_LOG_HPP_

#include <iostream>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/empty_deleter.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>


namespace logging {

enum class Severity { debug, info, warning, error, critical };

inline const char* to_string(Severity severity)
{
	switch (severity) {
	case Severity::debug: return   	"DEBUG   ";
	case Severity::info: return    	"INFO    ";
	case Severity::warning: return 	"WARNING ";
	case Severity::error: return   	"ERROR   ";
	case Severity::critical: return "CRITICAL";
	default: return nullptr;
	}
}

inline std::ostream& operator<<(std::ostream& os, Severity severity)
{
	const char* str = to_string(severity);
	if (str) {
		os << str;
	} else {
		os << static_cast<int>(severity);
	}
	return os;
}

BOOST_LOG_ATTRIBUTE_KEYWORD(Class, "Class", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(Comment, "Comment", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", Severity)
BOOST_LOG_ATTRIBUTE_KEYWORD(CoroSpecificAttr, "CoroSpecificAttr", std::string)

namespace detail {

static const auto defaultLogExpression =
		boost::log::expressions::stream <<
				boost::log::expressions::format_date_time< boost::posix_time::ptime >
						("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") <<
				": [" << severity << "] " << CoroSpecificAttr << " " <<
				Class << " " << Comment <<
				": " << boost::log::expressions::message;

}

inline void initDefaultFileLogger(const std::string& filename)
{
	boost::log::add_common_attributes();
	boost::log::add_file_log
    (
        boost::log::keywords::file_name = filename,
        boost::log::keywords::format = //"%TimeStamp%: [%Severity%] %Class%: %Message%"
			(
				detail::defaultLogExpression
			)
    );
}

inline void initDefaultStreamLogger(std::ostream& stream)
{
	namespace logging = boost::log;
	namespace sinks = logging::sinks;

	logging::add_common_attributes();
	boost::shared_ptr< std::ostream > streamPtr(&stream, logging::empty_deleter());

	typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

    sink->set_formatter
    (
		detail::defaultLogExpression
	);

	sink->locked_backend()->add_stream(streamPtr);
	logging::core::get()->add_sink(sink);
}

typedef boost::log::sources::severity_logger<Severity> Logger;

template <typename Logger>
inline void setClass(Logger& logger, const std::string& value)
{
	logger.add_attribute("Class",
			boost::log::attributes::constant<std::string>(value));
}

template <typename Logger>
inline void setComment(Logger& logger, const std::string& value)
{
	logger.add_attribute("Comment",
			boost::log::attributes::constant<std::string>(value));
}

} // namespace log


#endif /* LOGGING_LOG_HPP_ */
