// // Copyright 2010, Shuo Chen.  All rights reserved.
// // http://code.google.com/p/muduo/
// //
// // Use of this source code is governed by a BSD-style license
// // that can be found in the License file.

// // Author: Shuo Chen (chenshuo at chenshuo dot com)
// //

// #include "net/connector.h"

// #include "base/log.h"
// #include "net/channel.h"
// #include "net/socket_wrapper.h"
// #include "net/event_loop.h"

// #include <errno.h>
// #include <sys/types.h>          /* See NOTES */
// #include <sys/socket.h>

// const int Connector::kMaxRetryDelayMs;


// int getSocketError(int sockfd)
// {
//     int optval;
//     socklen_t optlen = static_cast<socklen_t>(sizeof optval);

//     if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
//     {
//         return errno;
//     }
//     else
//     {
//         return optval;
//     }
// }

// struct sockaddr_in6 getLocalAddr(int sockfd)
// {
//     struct sockaddr_in6 localaddr;
//     bzero( &localaddr, sizeof localaddr );
//     socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
//     if (::getsockname(sockfd, (sockaddr*)(&localaddr), &addrlen) < 0)
//     {
//         ERROR( "sockets::getLocalAddr" );
//     }
//     return localaddr;
// }


// struct sockaddr_in6 getPeerAddr(int sockfd)
// {
//     struct sockaddr_in6 peeraddr;
//     bzero(&peeraddr, sizeof peeraddr);
//     socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
//     if (::getpeername(sockfd,(sockaddr*)(&peeraddr), &addrlen) < 0)
//     {
//         ERROR( "sockets::getPeerAddr" );
//     }
//     return peeraddr;
// }

// bool isSelfConnect(int sockfd)
// {
//     struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
//     struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
//     if (localaddr.sin6_family == AF_INET)
//     {
//         const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
//         const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
//         return laddr4->sin_port == raddr4->sin_port
//         && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
//     }
//     else if (localaddr.sin6_family == AF_INET6)
//     {
//         return localaddr.sin6_port == peeraddr.sin6_port
//         && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
//     }
//     else
//     {
//         return false;
//     }   
// }


// Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
//   : loop_(loop),
//     serverAddr_(serverAddr),
//     connect_(false),
//     state_(kDisconnected),
//     retryDelayMs_(kInitRetryDelayMs)
// {
//     DEBUG( "ctor [{}]", this );
// }

// Connector::~Connector()
// {
//     DEBUG( "dtor [{}]", this );
//     assert(!channel_);
// }

// void Connector::start()
// {
//   connect_ = true;
//   loop_->RunInLoop(std::bind(&Connector::startInLoop, this)); // FIXME: unsafe
// }

// void Connector::startInLoop()
// {
//   loop_->AssertInLoopThread();
//   assert(state_ == kDisconnected);
//   if (connect_)
//   {
//     connect();
//   }
//   else
//   {
//     DEBUG( "do not connect" );
//   }
// }

// void Connector::stop()
// {
//   connect_ = false;
//   loop_->QueueInLoop(std::bind(&Connector::stopInLoop, this)); // FIXME: unsafe
//   // FIXME: cancel timer
// }

// void Connector::stopInLoop()
// {
//   loop_->AssertInLoopThread();
//   if (state_ == kConnecting)
//   {
//     setState(kDisconnected);
//     int sockfd = removeAndResetChannel();
//     retry(sockfd);
//   }
// }

// void Connector::connect()
// {
//   SocketWrapper conn_sock = CreateNonblockSocket();    
//   int sockfd = conn_sock.fd();
//   int ret = ::connect(sockfd, (const sockaddr*)( serverAddr_.get_sockaddr() ), sizeof( sockaddr ) );
//   int savedErrno = (ret == 0) ? 0 : errno;
//   switch (savedErrno)
//   {
//     case 0:
//     case EINPROGRESS:
//     case EINTR:
//     case EISCONN:
//       connecting(sockfd);
//       break;

//     case EAGAIN:
//     case EADDRINUSE:
//     case EADDRNOTAVAIL:
//     case ECONNREFUSED:
//     case ENETUNREACH:
//       retry(sockfd);
//       break;

//     case EACCES:
//     case EPERM:
//     case EAFNOSUPPORT:
//     case EALREADY:
//     case EBADF:
//     case EFAULT:
//     case ENOTSOCK:
//         ERROR( "connect error in Connector::startInLoop " );
//         ::close(sockfd);
//       break;

//     default:
//         ERROR( "Unexpected error in Connector::startInLoop {}", savedErrno );;
//         ::close(sockfd);
//       // connectErrorCallback_();
//       break;
//   }
// }

// void Connector::restart()
// {
//   loop_->AssertInLoopThread();
//   setState(kDisconnected);
//   retryDelayMs_ = kInitRetryDelayMs;
//   connect_ = true;
//   startInLoop();
// }

// void Connector::connecting(int sockfd)
// {
//   setState(kConnecting);
//   assert(!channel_);
//   channel_.reset(new Channel(loop_, sockfd));
//   channel_->set_write_callback(
//       std::bind(&Connector::handleWrite, this)); // FIXME: unsafe
//   channel_->set_error_callback(
//       std::bind(&Connector::handleError, this)); // FIXME: unsafe

//   // channel_->tie(shared_from_this()); is not working,
//   // as channel_ is not managed by shared_ptr
//   channel_->EnableReading();
// }

// int Connector::removeAndResetChannel()
// {
//   channel_->DisableAll();
//   channel_->Remove();
//   int sockfd = channel_->fd();
//   // Can't reset channel_ here, because we are inside Channel::handleEvent
//   loop_->QueueInLoop(std::bind(&Connector::resetChannel, this)); // FIXME: unsafe
//   return sockfd;
// }

// void Connector::resetChannel()
// {
//   channel_.reset();
// }

// void Connector::handleWrite()
// {
//     DEBUG( "Connector::handleWrite {}", state_);
//     if (state_ == kConnecting)
//     {
//         int sockfd = removeAndResetChannel();
//         int err = getSocketError(sockfd);
//         if (err)
//         {
//             WARN( "Connector::handleWrite - SO_ERROR = {}", err ) ;//strerror_tl(err)
//             retry(sockfd);
//         }
//         else if (isSelfConnect(sockfd))
//         {
//             WARN("Connector::handleWrite - Self connect");
//             retry(sockfd);
//         }
//         else
//         {
//             setState(kConnected);
//             if (connect_)
//             {
//                 newConnectionCallback_(sockfd);
//             }
//             else
//             {
//                 ::close(sockfd);
//             }
//         }
//     }
//     else
//     {
//         // what happened?
//         assert(state_ == kDisconnected);
//     }
// }

// void Connector::handleError()
// {
//     ERROR( "Connector::handleError state= {}", state_ );
//     if (state_ == kConnecting)
//     {
//         int sockfd = removeAndResetChannel();
//         int err = getSocketError(sockfd);
//         retry(sockfd);
//     }
// }

// void Connector::retry(int sockfd)
// {
//     ::close(sockfd);
//     setState(kDisconnected);
//     if (connect_)
//     {
//         INFO( "Connector::retry - Retry connecting to {} in {} milliseconds.", serverAddr_.ToIpPort(),retryDelayMs_ );
        
//         // sleep 几秒
//         loop_->RunInLoop(std::bind(&Connector::startInLoop, shared_from_this())) ;
                    
//         retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
//     }
//     else
//     {
//         DEBUG( "do not connect" );
//     }
// }