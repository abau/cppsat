#include <iostream>
#include "../cppsat.hpp"

using cppsat::Bit;
using cppsat::Bits;

int main () {
  const int n = 8;

  // build board
  std::vector <Bits> board;
  for (int i = 0; i < n; i++) {
    board.push_back (Bits (n));
  }

  // horizontal
  for (Bits& row : board) {
    cppsat::assertAny ({ row.exactly (1) });
  }

  // vertical
  for (int c = 0; c < n; c++) {
    Bits column;
    for (int r = 0; r < n; r++) {
      column.add (board [r][c]);
    }
    cppsat::assertAny ({ column.exactly (1) });
  }

  // (anti-)diagonal
  for (int c = -n + 1; c < n; c++) {
    Bits diagonal, antiDiagonal;
    for (int r = 0; r < n; r++) {
      if (c + r >= 0 && c + r < n) {
        diagonal.add (board [r][c + r]);
        antiDiagonal.add (board [n - r - 1][c + r]);
      }
    }
    cppsat::assertAny ({ diagonal.atmost (1) });
    cppsat::assertAny ({ antiDiagonal.atmost (1) });
  }

  // solve
  cppsat::printStatistics ();

  if (cppsat::solve ()) {
    for (Bits& row : board) {
      std::cout << row << std::endl;
    }
  }
  else {
    std::cout << "unsatisfiable\n";
  }
  return 0;
}
