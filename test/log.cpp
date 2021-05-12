#include <xbond/log/logger.hpp>
using namespace xbond;

int log_test(int argc, char* argv[]) {
    log::logger logger;
    XBOND_LOGGER_RECORD_STREAM(logger, INFO) << "test log";
}