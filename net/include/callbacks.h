#pragma once 

#include <memory>
#include <functional>

#include "timestamp.h"
#include "tcp_connection.h"


class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>; /* 为啥要用shared_ptr */
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>; /*  */
using CloseCallback = std::function<void(const TcpConnectionPtr&)>; /*  */
using WriteCompletedCallback = std::function<void(const TcpConnectionPtr&)>; /*  */

using MessageCallback = std::function<void( const TcpConnectionPtr&,
                                            Buffer*,
                                            Timestamp)>; /*  */
