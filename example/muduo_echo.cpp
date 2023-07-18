#include "net/tcp_server.h"
#include "base/log.h"

#include <string>
#include <functional>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

class EchoTcpServer {
public:
    EchoTcpServer(EventLoop* loop, const InetAddress& addr)
        : server_(loop, addr, "EchoTcpServer")
    {
        server_.set_conn_callback(
            std::bind(&EchoTcpServer::onConnection, this, std::placeholders::_1)
        );
        server_.set_msg_callback(std::bind(&EchoTcpServer::onMessage, this, _1, _2, _3));
    }

    void start() {
        server_.Start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
    }

    void onMessage(const TcpConnectionPtr &conn,
                UnlimitedBuffer *buf,
                Timestamp time) {

                std::string msg = buf->RetrieveAll();
                // muduo::string msg(buf->retrieveAllAsString());
                conn->Send(msg);
    }

    TcpServer server_;
};

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: cmd port\n");
        return -10;
    }
    int port = atoi(argv[1]);

    // muduo::g_logLevel = muduo::Logger::ERROR;
    // muduo::net::EventLoop loop;
    ONLY_TO_CONSOLE; LOGINIT(); LOG_LEVEL_ERROR; // 只输出在控制台，LOG_LEVEL_ERROR级别

    EventLoop loop;
    InetAddress addr(port);
    EchoTcpServer server(&loop, addr);
    server.start();

    loop.Loop();

    return 0;
}
