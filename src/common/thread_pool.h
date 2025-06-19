#pragma once

#define BOOST_ASIO_NO_EXCEPTIONS
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>

namespace lunaticvibes
{

std::pair<boost::asio::thread_pool&, std::mutex&> get_thread_pool();
void post_job(std::atomic<size_t>& counter, auto callable)
{
    auto [pool, mutex] = get_thread_pool();
    std::unique_lock l{mutex};
    boost::asio::post(pool, [&counter, callable = std::move(callable)]() {
        struct Counter
        {
            std::atomic<size_t>& counter;
            ~Counter() { counter++; }
        } _c(counter);
        callable();
    });
}

} // namespace lunaticvibes
