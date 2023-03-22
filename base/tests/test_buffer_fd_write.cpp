// #include "buffer.h"
// #include <iostream>
// #include <sys/types.h>
// #include <unistd.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <cassert>
// using namespace std;

// int main(int argc, char const *argv[])
// {
//   Buffer bs{};
//   int rfd = open("../tests/data", O_RDONLY);
//   int wfd = open("../tests/dtmp", O_WRONLY|O_CREAT|O_TRUNC, 0644);
//   assert(rfd > 0);
//     assert(rfd >= 0);
//   // int wfd = open( "data_cp", O_WRONLY | O_CREAT);
  
//   while( bs.read_fd(rfd) > 0 )  {
//      bs.write_fd( wfd );
//     // cout<<bs.size()<<endl;
//   }
//   cout<<bs.size()<<endl;

//   close( rfd );
//   close( wfd );
//   return 0;
// }
