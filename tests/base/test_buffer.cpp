#include "base/buffer.h"

#include <gtest/gtest.h>
using std::string;


TEST( testBuffer, testBufferAppendRetrieve ) {
    Buffer buf;
    int init_PrependableBytes = Buffer::kCheapPrepend;
    int init_kInitialSize = Buffer::kInitialSize;
    
    EXPECT_EQ(buf.ReadableBytes(), 0);
    EXPECT_EQ(buf.WritableBytes(), init_kInitialSize);
    EXPECT_EQ(buf.PrependableBytes(), init_PrependableBytes);

    const string str(200, 'x');
    buf.Append(str);
    EXPECT_EQ(buf.ReadableBytes(), str.size());
    EXPECT_EQ(buf.WritableBytes(), init_kInitialSize - str.size());
    EXPECT_EQ(buf.PrependableBytes(), init_PrependableBytes);

    const string str2 =  buf.RetrieveAsString(50);
    EXPECT_EQ(str2.size(), 50);
    EXPECT_EQ(buf.ReadableBytes(), str.size() - str2.size());
    EXPECT_EQ(buf.WritableBytes(), init_kInitialSize - str.size());
    EXPECT_EQ(buf.PrependableBytes(), init_PrependableBytes + str2.size());
    EXPECT_EQ(str2, string(50, 'x'));

    buf.Append(str);
    EXPECT_EQ(buf.ReadableBytes(), 2*str.size() - str2.size());
    EXPECT_EQ(buf.WritableBytes(), init_kInitialSize - 2*str.size());
    EXPECT_EQ(buf.PrependableBytes(), init_PrependableBytes + str2.size());

    const string str3 =  buf.RetrieveAllAsString();
    EXPECT_EQ(str3.size(), 350);
    EXPECT_EQ(buf.ReadableBytes(), 0);
    EXPECT_EQ(buf.WritableBytes(), init_kInitialSize);
    EXPECT_EQ(buf.PrependableBytes(), init_PrependableBytes);
    EXPECT_EQ(str3, string(350, 'x'));
}

TEST( testBuffer, testBufferGrow ) {
    Buffer buf;
    int init_PrependableBytes = Buffer::kCheapPrepend;
    int init_kInitialSize = Buffer::kInitialSize;
    buf.Append(string(400, 'y'));
    EXPECT_EQ(buf.ReadableBytes(), 400);
    EXPECT_EQ(buf.WritableBytes(), init_kInitialSize-400);

    buf.Retrieve(50);
    EXPECT_EQ(buf.ReadableBytes(), 350);
    EXPECT_EQ(buf.WritableBytes(), init_kInitialSize-400);
    EXPECT_EQ(buf.PrependableBytes(), init_PrependableBytes+50);

    buf.Append(string(1000, 'z'));
    EXPECT_EQ(buf.ReadableBytes(), 1350);
    EXPECT_EQ(buf.WritableBytes(), 0);
    EXPECT_EQ(buf.PrependableBytes(), init_PrependableBytes+50); // FIXME

    buf.RetrieveAll();
    EXPECT_EQ(buf.ReadableBytes(), 0);
    EXPECT_EQ(buf.WritableBytes(), 1400); // FIXME
    EXPECT_EQ(buf.PrependableBytes(), init_PrependableBytes);
}

TEST( testBuffer, testBufferInsideGrow ) {
    Buffer buf;
    int init_PrependableBytes = Buffer::kCheapPrepend;
    int init_kInitialSize = Buffer::kInitialSize;
    buf.Append(string(800, 'y'));
    EXPECT_EQ(buf.ReadableBytes(), 800);
    EXPECT_EQ(buf.WritableBytes(), init_kInitialSize-800);

    buf.Retrieve(500);
    EXPECT_EQ(buf.ReadableBytes(), 300);
    EXPECT_EQ(buf.WritableBytes(), init_kInitialSize-800);
    EXPECT_EQ(buf.PrependableBytes(), init_PrependableBytes+500);

    buf.Append(string(300, 'z'));
    EXPECT_EQ(buf.ReadableBytes(), 600);
    EXPECT_EQ(buf.WritableBytes(), init_kInitialSize-600);
    EXPECT_EQ(buf.PrependableBytes(), init_PrependableBytes);

}

void output(Buffer&& buf, const void* inner)
{
  Buffer newbuf(std::move(buf));
  // printf("New Buffer at %p, inner %p\n", &newbuf, newbuf.Peek());
  EXPECT_EQ(inner, newbuf.Peek());
}

// NOTE: This test fails in g++ 4.4, passes in g++ 4.6.
TEST( testBuffer, testMove)
{
  Buffer buf;
  buf.Append("muduo", 5);
  const void* inner = buf.Peek();
  // printf("Buffer at %p, inner %p\n", &buf, inner);
  output(std::move(buf), inner);
}


int main(int argc, char **argv) {

    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();   
}






