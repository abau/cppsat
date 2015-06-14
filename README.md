# cppsat - Propositional Encodings in C++11

cppsat is C++11 library for specifying and solving propositional encodings.
The [MiniSat](https://github.com/niklasso/minisat) SAT solver is used as solver backend.

cppsat provides a `Bit` class for representing potentially unknown Boolean
values.
As `Bit` overloads many C++ operators, propositional encodings can be
specified in a concise and natural way.
cppsat also provides the `Bits` class as a collection of `Bit`s that supports
unsigned integer arithmetic.
See [cppsat.hpp] (https://github.com/apunktbau/cppsat/tree/master/cppsat.hpp)
for cppsat's complete API.

## Example

Specification of an adder circuit:
```
std::pair <Bit, Bit> halfAdder (const Bit& a, const Bit& b) {
  return std::make_pair (a != b, a && b);
}

std::pair <Bit, Bit> fullAdder (const Bit& a, const Bit& b, const Bit& c) {
  auto ha1 = halfAdder (a, b);
  auto ha2 = halfAdder (ha1.first, c);

  return std::make_pair (ha2.first, ha1.second || ha2.second);
}
```
See [examples] (https://github.com/apunktbau/cppsat/tree/master/examples)
for more complex examples, e.g., a sudoku and a n-queens solver.

In order to build the examples, install 
[MiniSat](https://github.com/niklasso/minisat). Compile via
```
g++ --std=c++11 cppsat.cpp examples/sudoku.cpp -lminisat
```

## Related Work

- [Satchmo](https://github.com/jwaldmann/satchmo)
- [Satchmo-core](https://github.com/apunktbau/satchmo-core)
- [Ersatz](https://github.com/ekmett/ersatz)
