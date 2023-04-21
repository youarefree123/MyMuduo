
#include "noncopyable.h"
#include "log.h"

using namespace std;

int main()
{
  // srcDir_ = getcwd(nullptr, 256);
  

  ONLY_TO_CONSOLE; LOGINIT(); LOG_LEVEL_INFO;
  for( int i = 0; i < 10; i++ ) {
    if( i < 5 ) {
      LOG_LEVEL_INFO;
      TRACE("trace{}",i);
      DEBUG("debug{}",i);
      INFO("info{}",i);
      WARN("warn{}",i);
      ERROR("error{}",i);
      // CRITICAL( "crittcal{}",i );
    }
    else {
      LOG_LEVEL_TRACE;
      TRACE("trace{}",i);
      DEBUG("debug{}",i);
      INFO("info{}",i);
      WARN("warn{}",i);
      ERROR("error{}",i);
      // CRITICAL( "crittcal{}",i );
    }
  }
  
  return 0;
}
