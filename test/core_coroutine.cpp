#include <xbond/coroutine.hpp>
#include <xbond/coroutine_lock.hpp>
#include <xbond/coroutine_condition_variable.hpp>
#include <xbond/coroutine_channel.hpp>
#include <iostream>

using namespace xbond;

int core_coroutine_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    boost::asio::io_context io;
    coroutine_condition_variable cv(io);
    coroutine_unique_mutex mutex(io);
    coroutine_unique_lock guard(mutex);
    auto channel = make_channel<int>(io);
    int shared = 0;

    coroutine::start(io, [&cv, &mutex, &channel, &shared] (coroutine_handler& ch) {
        cv.wait(ch);
        std::cout << "\t\twake up\n";
        {
        coroutine_unique_lock guard(mutex, ch);
        std::cout << "\t\tlock: " << shared++ << "\n";
        }
        int x;
        while (channel->from(x, ch)) {
            std::cout << "\t\t" << x << "\n";
        }
    });

    coroutine::start(io, [&cv, &guard, &channel, &shared] (coroutine_handler& ch) {
        coroutine::sleep(std::chrono::milliseconds(1000), ch);
        guard.lock(ch);
        cv.notify_all();

        shared = 1;
        coroutine::sleep(std::chrono::milliseconds(1000), ch);
        guard.unlock(ch);

        for (int i=0;i<10;++i) {
            channel->into(i, ch);
        }
        channel->close();
    });

    io.run();
}
