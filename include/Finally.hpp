#ifndef FINALLY_HPP_
#define FINALLY_HPP_

#include <memory>
#include <utility>

template <typename Function>
class Finally {
public:
	Finally(const Function& function):
		function(new Function(function))
	{}
	~Finally()
	{
		if (function) {
			(*function)();
		}
	}

	// no copy is allowed
	Finally(const Finally&) = delete;
	Finally& operator=(const Finally&) = delete;

	Finally(Finally&& other):
		function(std::move(other.function))
	{}
	Finally& operator=(Finally&& other)
	{
		function = std::move(other.function);
		return *this;
	}
private:
	std::unique_ptr<Function> function;
};

template <typename Function>
Finally<Function> finally(const Function& function)
{
	return Finally<Function>(function);
}

#endif /* FINALLY_HPP_ */
