#include <chrono>
#include <sstream>
#include <inttypes.h>
#include <iomanip>
#include <sys/time.h>
#include "net/base/timestamp.h"

const size_t kMicroSecondsPerSecond = 1e6;

Timestamp Timestamp::Now() {
    auto tp =std::chrono::system_clock::now();
    auto time=std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
    return  Timestamp( (size_t)time ) ;
}

std::string Timestamp::ToString() const {
    time_t seconds  = static_cast<time_t>( microseconds_since_epoch_ / kMicroSecondsPerSecond );
    size_t microseconds  = microseconds_since_epoch_ % kMicroSecondsPerSecond;
    std::stringstream ss;
    ss<<std::put_time(std::localtime(&seconds),"%Y-%m-%d %H:%M:%S μs:")<<microseconds;
    return ss.str();
}