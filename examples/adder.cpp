#include <iostream>
#include <utility>
#include "../cppsat.hpp"

using cppsat::Bit;

std::pair <Bit, Bit> halfAdder (const Bit& a, const Bit& b) {
  return std::make_pair (a != b, a && b);
}

std::pair <Bit, Bit> fullAdder (const Bit& a, const Bit& b, const Bit& c) {
  auto ha1 = halfAdder (a, b);
  auto ha2 = halfAdder (ha1.first, c);

  return std::make_pair (ha2.first, ha1.second || ha2.second);
}

int main () {
  Bit a, b, c;

  if (cppsat::solve (fullAdder (a, b, c).second == Bit (true))) {
    std::cout << "a: " << a << std::endl;
    std::cout << "b: " << b << std::endl;
    std::cout << "c: " << c << std::endl;
  }
  else {
    std::cout << "unsatisfiable\n";
  }
  return 0;
}
