#pragma once 

#include <memory>
#include <functional>
#include "net/base/timestamp.h"


class UnlimitedBuffer;
class TcpConnection; // 避免循环引用

using TcpConnectionPtr = std::shared_ptr<TcpConnection>; /* 为啥要用shared_ptr */
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>; /*  */
using CloseCallback = std::function<void(const TcpConnectionPtr&)>; /*  */
using WriteCompletedCallback = std::function<void(const TcpConnectionPtr&)>; /*  */

using MessageCallback = std::function<void( const TcpConnectionPtr&,
                                            UnlimitedBuffer*,
                                            Timestamp)>; /*  */

/// 高水位线回调
/// 如果输出缓冲的长度超过用户指定大小，就会触发回调（只在上升沿触发一次）。
/// 在非阻塞的发送数据情况下，假设Server发给Client数据流，
/// 为防止Server发过来的数据撑爆Client的输出缓冲区，
/// 一种做法是在Client的HighWaterMarkCallback中停止读取Server的数据，
/// 而在Client的WriteCompleteCallback中恢复读取Server的数据。
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;