#include <thread>
#include <xbond/log/logger.hpp>
#include <xbond/log/writer.hpp>
using namespace xbond;

#define XLOGGER(_level_) XBOND_LOGGER_RECORD_STREAM(logger, _level_) \
    << __func__ << " "

int log_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    log::logger logger;
    XBOND_LOGGER_RECORD_STREAM(logger, INFO) << "test log" << 1;
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 日志输出是异步线程
    logger.append(log::file_writer{"/tmp/logger.log"});
    XBOND_LOGGER_RECORD_STREAM(logger, DEBUG) << "test log" << 2;
    XLOGGER(WARN) << "test log" << 3;

    return 0;
}
