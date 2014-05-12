#include <aim/asio/spawn.hpp>

namespace boost { namespace asio { namespace this_coro { namespace detail {
    boost::thread_specific_ptr<coro_id> id;
}}}}

