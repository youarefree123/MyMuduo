#include "log.h"
#include "acceptor.h"
#include "event_loop.h"
#include "inet_address.h"
#include "tcp_server.h"
using namespace std;

int main(int argc, char const *argv[])
{
  // srcDir_ = getcwd(nullptr, 256);
//   Buffer b{};
    EventLoop p{};
    InetAddress i{};
    TcpServer s( &p, i, "1" );
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
