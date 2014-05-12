#include "logging/spawn.hpp"
#include <boost/log/core/core.hpp>
#include <boost/log/attributes/function.hpp>
#include <boost/algorithm/string/join.hpp>

namespace logging { namespace detail {
	CoroSpecificLogStringStack stack;
}}

namespace logging {

std::string getCoroSpecificLogStr()
{
	auto v = detail::stack.get();
	return boost::algorithm::join(v, " ");
}

void addCoroSpecificLogAttribute()
{
	boost::log::core::get()->
		add_global_attribute("CoroSpecificAttr",
				boost::log::attributes::make_function(
					&getCoroSpecificLogStr
		));
}

} // logging
