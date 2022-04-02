#include <xbond/coroutine.hpp>
#include <xbond/sync/lock.hpp>
#include <xbond/sync/condition_variable.hpp>
#include <xbond/channel.hpp>
#include <xbond/thread_pool.hpp>
#include <xbond/time/sleep_for.hpp>
#include <iostream>

using namespace xbond;

int core_coroutine_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    boost::asio::io_context io;
    coroutine_condition_variable cv(io);
    coroutine_unique_mutex mutex(io);
    coroutine_unique_lock guard(mutex);
    auto timer = std::make_shared<boost::asio::steady_timer>(io);
    auto channel = make_channel<int>(io);
    int shared = 0;

    coroutine::start(io, [&cv, &mutex, &channel, &shared] (coroutine_handler& ch) {
        std::cout << "\t\tcoroutine1: started\n";
        cv.wait(ch);
        std::cout << "\t\tcoroutine1: wake up\n";
        {
        coroutine_unique_lock guard(mutex, ch);
        std::cout << "\t\tcoroutine1: lock: " << shared++ << "\n";
        }
        int x;
        while (channel->from(x, ch)) {
            std::cout << "\t\t" << x << "\n";
        }
        std::cout << "\t\tcoroutine1: ended\n";
    });

    coroutine::start(io, [&io, &timer] (coroutine_handler& ch) {
        boost::system::error_code error;
        std::cout << "\t\tcoroutine2: started\n";
        timer->expires_after(std::chrono::seconds(10));
        timer->async_wait(ch[error]);
        if (error == boost::asio::error::operation_aborted) {
            std::cout << "\t\tcoroutine2: canceled\n";
        }
        std::cout << "\t\tcoroutine2: ended\n";
    });

    coroutine::start(io, [&cv, &guard, &channel, &shared, &timer] (coroutine_handler& ch) {
        std::cout << "\t\tcoroutine3: started\n";
        xbond::time::sleep_for(std::chrono::milliseconds(1000), ch);
        std::cout << "\t\tcoroutine3: (wakeup and) cancel coroutine2\n";
        timer->cancel();

        guard.lock(ch);
        cv.notify_all();

        shared = 1;
        xbond::time::sleep_for(std::chrono::milliseconds(1000), ch);
        guard.unlock(ch);

        for (int i=0;i<10;++i) {
            channel->into(i, ch);
        }
        channel->close();
        std::cout << "\t\tcoroutine3: ended\n";
    });

    thread_pool pool(4, [&io] () {
        io.run();
    });
    return 0;
}
