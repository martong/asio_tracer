#ifndef INCLUDE_LOGGING_SPAWN_HPP
#define INCLUDE_LOGGING_SPAWN_HPP

#include <string>
#include "aim/asio/spawn.hpp"

#include "aim/asio/CoroSpecificStorage.hpp"
#include "Finally.hpp"

namespace logging {

namespace detail {

struct CoroIdGetter {
	auto operator()() -> decltype(boost::asio::this_coro::get_id()) {
		return boost::asio::this_coro::get_id();
	}
};
using CoroSpecificLogStringStack = aim::CoroSpecificStorage<
	CoroIdGetter, std::vector<std::string>>;

extern CoroSpecificLogStringStack stack;

} // detail


class CoroLogStringPusher {
public:
	template <typename S>
	CoroLogStringPusher(S str)
	{
		detail::stack.get().emplace_back(std::move(str));
	}
	~CoroLogStringPusher()
	{
		detail::stack.get().pop_back();
	}
};

#define LOGGING_SCOPED_CORO_STR(str) \
	logging::CoroLogStringPusher raii{(str)};

class CoroLogStringStack {
	std::vector<std::string> oldStack;
public:
	CoroLogStringStack(std::vector<std::string> newStack)
	{
		oldStack = std::move(detail::stack.get());
		detail::stack.get() = std::move(newStack);
	}
	~CoroLogStringStack()
	{
		detail::stack.get() = std::move(oldStack);
	}
};

#define LOGGING_SCOPED_CORO_STR_STACK(stack) \
	logging::CoroLogStringStack raii{(stack)};

inline std::vector<std::string> getCoroSpecificLogStrStack() {
	return detail::stack.get();
}

std::string getCoroSpecificLogStr();
void addCoroSpecificLogAttribute();

namespace detail {

template <typename Function>
class Holder {
	Function function;
	std::vector<std::string> parentLogStrings;
public:
	explicit Holder(Function function) : function(function),
			 parentLogStrings(stack.get())
	{}
	void operator()(boost::asio::yield_context yield)
	{
		// set coroutine specific log string to parentLogString
		stack.get() = std::move(parentLogStrings);
		auto f = finally([](){ stack.erase(); });
		function(yield);
	}
};

} // detail

template <typename Function>
void spawn(boost::asio::io_service& ioService, Function function,
		const boost::coroutines::attributes& attributes
			  = boost::coroutines::attributes())
{
	boost::asio::spawn(ioService, detail::Holder<Function>(function),
			attributes);
}

template <typename Arg0, typename Function>
void spawn(Arg0 arg0, Function function,
		const boost::coroutines::attributes& attributes
			  = boost::coroutines::attributes())
{
	boost::asio::spawn(arg0, detail::Holder<Function>{function},
			attributes);
}




namespace detail {

template <typename Function>
class PostHolder {
	Function function;
	std::vector<std::string> parentLogStrings;
public:
	explicit PostHolder(Function function) : function(function),
			 parentLogStrings(stack.get())
	{}
	void operator()()
	{
		// set coroutine specific log string to parentLogString
		CoroLogStringStack raii{std::move(parentLogStrings)};
		function();
	}
};

} // detail

template <typename Arg0, typename Function>
auto post(Arg0& arg0, BOOST_ASIO_MOVE_ARG(Function) function)
-> BOOST_ASIO_INITFN_RESULT_TYPE(Function, void())
{
	boost::asio::detail::async_result_init<
			Function, void()> init{
				BOOST_ASIO_MOVE_CAST(Function)(function)};
	arg0.post(detail::PostHolder<decltype(init.handler)>{init.handler});
	return init.result.get();
}


} // logging

#endif /* INCLUDE_LOGGING_SPAWN_HPP */
