#include <iostream>
#include "../cppsat.hpp"

using cppsat::Bit;
using cppsat::Bits;

int main () {
  const unsigned int width = 8;
  const unsigned int result = 234;

  Bits b1 (width);
  Bits b2 (width);

  cppsat::assertAll ( { b1 * b2 == Bits (width, result)
                      ,      b1  > Bits (width, 1)
                      ,      b2  > Bits (width, 1)
                      } );

  if (cppsat::solve ()) {
    std::cout << b1.valueNat () << " * "
              << b2.valueNat () << " = "
              << (b1 * b2).valueNat ()
              << std::endl;
  }
  else {
    std::cout << "unsatisfiable\n";
  }
  return 0;
}
