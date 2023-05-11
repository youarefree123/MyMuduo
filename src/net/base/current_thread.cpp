#include "net/base/current_thread.h"
#include "net/base/timestamp.h"
namespace CurrentThread {

__thread int t_cached_tid = 0;

void CacheTid() {
  if( t_cached_tid == 0 ) {
    t_cached_tid = static_cast<pid_t>( ::syscall(SYS_gettid) );
  }
}

}