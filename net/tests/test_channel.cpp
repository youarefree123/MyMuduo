#include "log.h"
#include "timestamp.h"
#include "channel.h"


#include <functional>
#include <map>

#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>

using namespace std;

int main(int argc, char const *argv[])
{
  ONLY_TO_CONSOLE; LOGINIT(); LOG_LEVEL_TRACE;
  for(int i = 0; i < 10; i++) {
    TRACE( "It is no Trace:{},time:{}", i, Timestamp::Now().ToString());
  }
  
  return 0;
}
