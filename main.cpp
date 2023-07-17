#include "base/log.h"
#include "net/acceptor.h"
#include "net/event_loop.h"
#include "net/inet_address.h"
#include "net/tcp_server.h"
using namespace std;

int main(int argc, char const *argv[])
{
  // srcDir_ = getcwd(nullptr, 256);
//   Buffer b{};
    EventLoop p{};
    InetAddress i{};
    
//   ONLY_TO_FILE; LOGINIT(); LOG_LEVEL_INFO;
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
