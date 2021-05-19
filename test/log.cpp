#include <xbond/log/logger.hpp>
using namespace xbond;

int log_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    log::logger logger;
    XBOND_LOGGER_RECORD_STREAM(logger, INFO) << "test log 1";
    log::file_writer writer("/tmp/logger.log");
    logger.writer(writer);
    XBOND_LOGGER_RECORD_STREAM(logger, DEBUG) << "test log 2";
}