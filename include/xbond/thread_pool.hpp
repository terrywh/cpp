#include <thread>
#include <vector>

namespace xbond {

class thread_pool {
    std::vector<std::thread> worker_;
public:
    template <class Proc, class ... Args>
    thread_pool(int pool_size, Proc&& proc, Args&&... args) {
        for (int i=0;i<pool_size;++i) worker_.emplace_back(
            std::thread(std::forward<Proc>(proc), std::forward<Args>(args)...));
    }
    ~thread_pool() {
        for (int i=0;i<worker_.size();++i) worker_[i].join();
    }
};

} // namespace xbond
