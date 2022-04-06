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
    sync::condition_variable cv(io);
    sync::unique_mutex mutex(io);
    sync::unique_lock guard(mutex);
    auto timer = std::make_shared<boost::asio::steady_timer>(io);
    channel<int, 2> chn(io);
    int shared = 0;

    coroutine::start(io, [&cv, &mutex, &chn, &shared] (coroutine_handler& ch) {
        std::cout << "\t\tcoroutine1: started\n";
        cv.wait(ch);
        std::cout << "\t\tcoroutine1: wake up\n";
        {
            std::cout << "\t\tcoroutine1: before lock\n";
            sync::unique_lock _(mutex, ch);
            std::cout << "\t\tcoroutine1: share = " << shared++ << "\n";
        }
        for (int x; chn >> x;) {
            std::cout << "\t\tcoroutine1: x = " << x << "\n";
        }
        std::cout << "\t\tcoroutine1: ended\n";
    });

    coroutine::start(io, [&io, &timer, &chn] (coroutine_handler& ch) {
        boost::system::error_code error;
        std::cout << "\t\tcoroutine2: started\n";
        timer->expires_after(std::chrono::seconds(10));
        timer->async_wait(ch[error]);
        if (error == boost::asio::error::operation_aborted) {
            std::cout << "\t\tcoroutine2: canceled\n";
        }
        for (int x; chn >> x;) {
            std::cout << "\t\tcoroutine2: x = " << x << "\n";
        }
        std::cout << "\t\tcoroutine2: ended\n";
    });

    coroutine::start(io, [&cv, &guard, &chn, &shared, &timer] (coroutine_handler& ch) {
        std::cout << "\t\tcoroutine3: started\n";
        xbond::time::sleep_for(std::chrono::milliseconds(1000), ch);
        std::cout << "\t\tcoroutine3: cancel coroutine2\n";
        timer.reset();

        guard.lock(ch);
        std::cout << "\t\tcoroutine3: wakeup coroutine1\n";
        cv.notify_all();

        shared++;
        xbond::time::sleep_for(std::chrono::milliseconds(1000), ch);
        std::cout << "\t\tcoroutine3: unlock\n";
        guard.unlock(ch);

        for (int i=0;i<10;++i) chn << i;
        std::cout << "\t\tcoroutine3: close channel\n";
        chn.close();
        std::cout << "\t\tcoroutine3: ended\n";
    });

    thread_pool pool(4, [&io] () {
        io.run();
    });
    return 0;
}
