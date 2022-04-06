#include <thread>
#include <vector>

namespace xbond {

class thread_pool {
    std::vector<std::thread> worker_;
    bool joined_ = false;
public:
    template <class Proc, class ... Args>
    thread_pool(int pool_size, Proc&& proc, Args&&... args) {
        for (int i=0;i<pool_size;++i) worker_.emplace_back(
            std::forward<Proc>(proc), std::forward<Args>(args)...);
    }
    inline void wait() {
        join();
    }
    void join() {
        joined_ = true;
        for (int i=0;i<worker_.size();++i) worker_[i].join();
    }
    ~thread_pool() {
        if (!joined_) join();
    }
};

} // namespace xbond
