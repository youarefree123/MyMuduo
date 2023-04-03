#include "inet_address.h"
#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
  InetAddress addr{};
  cout<<addr.ToIp()<<endl;
  cout<<addr.ToPort()<<endl;
  cout<<addr.ToIpPort()<<endl;
  return 0;
}
