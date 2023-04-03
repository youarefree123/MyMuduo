#include "buffer.h"
#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
  Buffer bs{};
  bs.HasWrite( "1111" );
  bs.HasWrite("2222");
  string str = bs.HasRead( 3 );
  cout<<str<<endl;

  str = bs.HasRead( 3 );
  cout<<str<<endl;
  return 0;
}
