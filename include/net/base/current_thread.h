#pragma once

#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

namespace CurrentThread {

// __thread 表示虽然是全局变量，但是每个线程的都是独立的一个
extern __thread int t_cached_tid;

void CacheTid();

inline int Tid() {
  if( __builtin_expect( t_cached_tid == 0, 0 ) ) {
    CacheTid();
  }
  return t_cached_tid;
}


}