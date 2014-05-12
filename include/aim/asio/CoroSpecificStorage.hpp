#ifndef INCLUDE_AIM_ASIO_COROSPECIFICSTORAGE_HPP
#define INCLUDE_AIM_ASIO_COROSPECIFICSTORAGE_HPP

#include <map>
#include <mutex>

namespace unitTest { struct FixtureConnector; }

namespace aim {

template <typename CoroIdGetter, typename Data,
		 typename Mutex = std::mutex>
class CoroSpecificStorage {
	CoroIdGetter coroIdGetter;
	using CoroId = decltype(coroIdGetter());
	using Datas = std::map<CoroId, Data>;
	Datas datas;
	Mutex mutex;
	friend struct unitTest::FixtureConnector;
public:
	Data& get()
	{
		std::unique_lock<Mutex> lock{mutex};
		return datas[coroIdGetter()];
	}
	void erase()
	{
		std::unique_lock<Mutex> lock{mutex};
		datas.erase(coroIdGetter());
	}
};

} // aim

#endif /* INCLUDE_AIM_ASIO_COROSPECIFICSTORAGE_HPP */
