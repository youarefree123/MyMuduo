#include "timestamp.h"
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
using namespace std;

int main()
{
  for(int i = 0; i < 10; i++) {
    auto time = Timestamp::Now();
    cout<<time<<endl;
  }
  
  return 0;
}
