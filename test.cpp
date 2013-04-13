#include <iostream>

#include "lockfreecircularbuffer.h"

#include <future>

#include <string>
#include <fstream>
#include <unordered_map>

int main(int argc, char **argv)
{
  LockFreeCircularBuffer<int> buffer(2);

  volatile bool done = false;
  auto producer1 = std::async([&] {
    size_t i = 0;
    while (i < 1000000) {
      if (buffer.push(i))
        ++i;
    }
  });

  auto producer3 = std::async([&] {
    size_t i = 1000000;
    while (i < 1500000) {
      if (buffer.push(i))
        ++i;
    }
  });

  auto producer2 = std::async([&] {
    size_t i = 1500000;
    while (i < 2000000) {
      if (buffer.push(i))
        ++i;
    }
  });

  auto consumer1 = std::async([&] {
    int i;
    while (!buffer.empty() || !done) {
      if (buffer.pop(i))
        printf("%d\n", i);
    }
  });

  auto consumer2 = std::async([&] {
    int i;
    while (!buffer.empty() || !done) {
      if (buffer.pop(i))
        printf("%d\n", i);
    }
  });

  auto consumer3 = std::async([&] {
    int i;
    while (!buffer.empty() || !done) {
      if (buffer.pop(i))
        printf("%d\n", i);
    }
  });

  auto consumer4 = std::async([&] {
    int i;
    while (!buffer.empty() || !done) {
      if (buffer.pop(i))
        printf("%d\n", i);
    }
  });

  auto consumer5 = std::async([&] {
    int i;
    while (!buffer.empty() || !done) {
      if (buffer.pop(i))
        printf("%d\n", i);
    }
  });

  producer1.wait();
  producer2.wait();
  producer3.wait();

  done = true;

  consumer1.wait();
  consumer2.wait();
  consumer3.wait();
  consumer4.wait();
  consumer5.wait();

  return 0;
}
