#include <cassert>
#include <iostream>
#include "../cppsat.hpp"

using cppsat::Bit;
using cppsat::Bits;

typedef std::vector <std::vector <int>>  BoardSpecification;
typedef std::vector <std::vector <Bits>> Board;

Board sudoku (const BoardSpecification&);

int main () {
  Board board = sudoku ( { {  5,  3, -1, -1,  7, -1, -1, -1, -1 }
                         , {  6, -1, -1,  1,  9,  5, -1, -1, -1 }
                         , { -1,  9,  8, -1, -1, -1, -1,  6, -1 }
                         , {  8, -1, -1, -1,  6, -1, -1, -1,  3 }
                         , {  4, -1, -1,  8, -1,  3, -1, -1,  1 }
                         , {  7, -1, -1, -1,  2, -1, -1, -1,  6 }
                         , { -1,  6, -1, -1, -1, -1,  2,  8, -1 }
                         , { -1, -1, -1,  4,  1,  9, -1, -1,  5 }
                         , { -1, -1, -1, -1,  8, -1, -1,  7,  9 } });

  if (cppsat::solve ()) {
    for (std::vector <Bits>& row : board) {
      for (Bits& b : row) {
        std::cout << b.valueNat () << " ";
      }
      std::cout << std::endl;
    }
  }
  else {
    std::cout << "unsatisfiable\n";
  }
  return 0;
}

Board sudoku (const BoardSpecification& spec) {
  assert (spec.size () == 9);

  Board board;
  board.resize (9);

  // build board
  for (size_t i = 0; i < 9; i++) {
    assert (spec[i].size () == 9);

    for (size_t j = 0; j < 9; j++) {
      assert (spec[i][j] <= 9);

      Bits number = spec[i][j] < 1 ? Bits (4) : Bits (4, spec[i][j]);

      cppsat::assertAll ({ number >= Bits (4, 1), number <= Bits (4, 9) });

      board [i].push_back (number);
    }
  }

  // horizontal
  for (std::vector <Bits>& row : board) {
    cppsat::assertAny ({ cppsat::allDifferent (row) });
  }

  // vertical
  for (size_t i = 0; i < 9; i++) {
    std::vector <Bits> column;
    for (size_t j = 0; j < 9; j++) {
      column.push_back (board [j][i]);
    }
    cppsat::assertAny ({ cppsat::allDifferent (column) });
  }

  // blocks
  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < 3; j++) {
      std::vector <Bits> block;
      for (size_t u = 0; u < 3; u++) {
        for (size_t v = 0; v < 3; v++) {
          block.push_back (board [(3*i)+u] [(3*j)+v]);
        }
      }
      cppsat::assertAny ({ cppsat::allDifferent (block) });
    }
  }
  return board;
}
