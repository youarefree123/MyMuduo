/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class BaseBuffer
{
private:
    /* data */
public:
    BaseBuffer(/* args */);
    ~BaseBuffer();
};

BaseBuffer::BaseBuffer(/* args */)
{
}

BaseBuffer::~BaseBuffer()
{
}
