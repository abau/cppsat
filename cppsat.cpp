#include <algorithm>
#include <functional>
#include <iostream>
#include <minisat/core/Solver.h>
#include "cppsat.hpp"

namespace {
  static Minisat::Solver solver;

  Minisat::Lit toMinisatLit (const cppsat::Bit& bit) {
    assert (bit.isConstant () == false);
    return Minisat::toLit (bit.literal ());
  }

  Minisat::Var toMinisatVar (const cppsat::Bit& bit) {
    assert (bit.isConstant () == false);
    return Minisat::var (toMinisatLit (bit));
  }

  template <typename T>
  std::pair <T, T> halfAdder (const T& a, const T& b) {
    return std::make_pair (a != b, a && b);
  }

  template <typename T>
  std::pair <T, T> fullAdder (const T& a, const T& b, const T& c) {

    auto ha1 = halfAdder (a, b);
    auto ha2 = halfAdder (ha1.first, c);

    return std::make_pair (ha2.first, ha1.second || ha2.second);
  }

  std::vector <bool> toBinary (size_t s, unsigned int i) {
    std::vector <bool> bits;

    std::function <void (unsigned int)> go = [s, &bits, &go] (unsigned int j) {
      assert (bits.size () < s);
      if (j < 2) {
        bits.push_back (bool (j));
      }
      else {
        const std::div_t d = std::div (j, 2);

        bits.push_back (bool (d.rem));
        go (d.quot);
      }
    };
    go ((unsigned int) std::abs (i));

    while (bits.size () < s) {
      bits.push_back (false);
    }
    return bits;
  }

  unsigned int fromBinary (const std::vector <bool>& bits) {
    int result = 0;

    for (size_t i = 0; i < bits.size (); i++) {
      result += bits[i] ? (1 << i) : 0;
    }
    return result;
  }

  void equalizeLength (cppsat::Bits& a, cppsat::Bits& b) {
    const size_t max = std::max (a.size (), b.size ());

    while (a.size () < max) {
      a.add (cppsat::Bit (false));
    }
    while (b.size () < max) {
      b.add (cppsat::Bit (false));
    }
  }

  typedef std::pair <cppsat::Bit, cppsat::Bit> BitPair;

  BitPair compare (const cppsat::Bits& x, const cppsat::Bits& y) {
    cppsat::Bits a (x);
    cppsat::Bits b (y);

    equalizeLength (a, b);

    std::function <BitPair (size_t)> go = [&go,&a,&b] (size_t i) {
      if (i < a.size ()) {
        auto         rest   = go (i+1);
        cppsat::Bit& leRest = rest.first;
        cppsat::Bit& eqRest = rest.second;

        return BitPair ( leRest || (eqRest && !a[i] && b[i])
                       , eqRest && (a[i] == b[i]) );
      }
      else {
        return BitPair (cppsat::Bit (false), cppsat::Bit (true));
      }
    };
    return go (0);
  }
}

namespace cppsat {

  Bit :: Bit ()
    : _isConstant (false)
    , _value      (Minisat::toInt (Minisat::mkLit (solver.newVar ())))
  {}

  Bit :: Bit (bool b)
    : _isConstant (true)
    , _value      (b)
  {}

  Bit :: Bit (int i)
    : _isConstant (false)
    , _value      (i)
  {}

  Bit Bit :: operator!  ()             const { return this->negate (); }
  Bit Bit :: operator&& (const Bit& o) const { return cppsat::all ({ *this, o }); }
  Bit Bit :: operator|| (const Bit& o) const { return cppsat::any ({ *this, o }); }
  Bit Bit :: operator== (const Bit& o) const { return this->equals    (o); }
  Bit Bit :: operator!= (const Bit& o) const { return this->equalsNot (o); }

  bool Bit :: isConstant () const {
    return this->_isConstant;
  }

  int Bit :: literal () const {
    assert (this->isConstant () == false);
    return this->_value;
  }

  bool Bit :: hasValue () const {
    if (this->isConstant ()) {
      return true;
    }
    else {
      return solver.model.size () > toMinisatVar (*this);
    }
  }

  bool Bit :: hasValue (bool b) const {
    return this->hasValue () && this->value () == b;
  }

  bool Bit :: value () const {
    if (this->isConstant ()) {
      return bool (this->_value);
    }
    else {
      assert (this->hasValue ());

      Minisat::lbool v = solver.modelValue (toMinisatLit (*this));
      if (v == Minisat::l_True) {
        return true;
      }
      else if (v == Minisat::l_False) {
        return false;
      }
      else {
        std::abort ();
      }
    }
  }

  Bit Bit :: negate () const {
    return this->isConstant () ? Bit (! this->value ())
                               : Bit (Minisat::toInt (~toMinisatLit (*this)));
  }

  Bit Bit :: implies (const Bit& other) const {
    return this->negate () || other;
  }

  Bit Bit :: equals (const Bit& other) const {
    return this->equalsNot (other).negate ();
  }

  Bit Bit :: equalsNot (const Bit& other) const {
    if (this->hasValue ()) {
      return this->value () ? other.negate ()
                            : other;
    }
    else if (other.hasValue ()) {
      return other.value () ? this->negate ()
                            : *this;
    }
    else {
      Bit result;
      cppsat::assertAny ({ this->negate (), other          , result           });
      cppsat::assertAny ({*this           , other.negate (), result           });
      cppsat::assertAny ({*this           , other          , result.negate () });
      cppsat::assertAny ({ this->negate (), other.negate (), result.negate () });
      return result;
    }
  }

  Bit Bit :: ifThenElse  (const Bit& t, const Bit& f) const {
    if (this->hasValue ()) {
      return this->value () ? t : f;
    }
    else {
      Bit result;
      cppsat::assertAny ({ this->negate (), t.negate (), result           });
      cppsat::assertAny ({ this->negate (), t          , result.negate () });
      cppsat::assertAny ({*this           , f.negate (), result           });
      cppsat::assertAny ({*this           , f          , result.negate () });
      return result;
    }
  }

  Bits :: Bits () {}

  Bits :: Bits (size_t s) 
    : _bits (s) 
  {}

  Bits :: Bits (size_t s, unsigned int i) 
    : Bits (toBinary (s, i))
  {}

  Bits :: Bits (const std::vector <Bit>& bits) 
    : _bits (bits) 
  {}

  Bits :: Bits (const std::vector <bool>& bits) {
    for (bool b : bits) {
      this->add (Bit (b));
    }
  }

  Bits Bits :: operator! () const { return this->negate (); }

  Bits Bits :: operator+ (const Bits& other) const {
    Bits a (*this);
    Bits b (other);
    Bits result;

    equalizeLength (a, b);

    std::function <Bit (size_t, const Bit&)> go = [&go, &a, &b, &result]
                                                  (size_t i, const Bit& carry)
    {
      if (i < a.size ()) {
        BitPair r = fullAdder (a[i], b[i], carry);

        result.add (r.first);
        return go (i + 1, r.second);
      }
      else {
        return carry;
      }
    };
    cppsat::assertAny ({ ! go (0, Bit (false)) });
    return result;
  }

  Bits Bits :: operator* (const Bits& other) const {
    Bits a (*this);
    Bits b (other);

    equalizeLength (a, b);

    std::vector <std::vector <Bit>> weights;
    weights.resize ((a.size () * 2) - 1);

    auto isReducable = [&weights] () {
      for (const std::vector <Bit>& ws : weights) {
        if (ws.size () > 2) {
          return true;
        }
      }
      return false;
    };
    auto reduce = [&weights] () {
      for (int i = weights.size () - 1; i >= 0; i--) {
        const size_t numW = weights[i].size ();

        if (numW == 1) {
          continue;
        }
        else if (numW == 2) {
          BitPair r = halfAdder (weights[i][0], weights[i][1]);
          weights[i].clear ();
          weights[i].push_back (r.first);

          if (weights.size () == (unsigned int)(i + 1)) {
            weights.resize (weights.size () + 1);
          }
          weights[i+1].push_back (r.second);
        }
        else if (numW >= 3) {
          BitPair r = fullAdder ( weights[i][numW - 1]
                                , weights[i][numW - 2]
                                , weights[i][numW - 3] );
          weights[i].pop_back ();
          weights[i].pop_back ();
          weights[i].pop_back ();
          weights[i].push_back (r.first);

          if (weights.size () == (unsigned int) (i + 1)) {
            weights.resize (weights.size () + 1);
          }
          weights[i+1].push_back (r.second);
        }
      }
    };
    auto add = [&weights] () {
      Bits result;
      std::function <Bit (size_t, const Bit&)> go = [&go, &result, &weights] 
                                                    (size_t i, const Bit& carry)
      {
        if (i >= weights.size ()) {
          return carry;
        }
        else {
          assert (weights[i].size () == 1 || weights[i].size () == 2);

          BitPair r = fullAdder ( weights[i][0]
                                , weights[i].size () == 1 ? Bit (false)
                                                          : weights[i][1]
                                , carry );
          result.add (r.first);
          return go (i + 1, r.second);
        }
      };
      cppsat::assertAny ({ ! go (0, Bit (false)) });
      return result;
    };

    for (size_t i = 0; i < a.size (); i++) {
      for (size_t j = 0; j < b.size (); j++) {
        weights[i+j].push_back (a[i] && b[j]);
      }
    }
    while (isReducable ()) {
      reduce ();
    }

    Bits fullResult = add ();
    Bits result;

    for (size_t i = 0; i < fullResult.size (); i++) {
      if (i < a.size ()) {
        result.add (fullResult[i]);
      }
      else {
        cppsat::assertAny ({ ! fullResult[i] });
      }
    }
    return result;
  }

  Bit Bits :: operator== (const Bits& other) const {
    BitPair r = compare (*this, other);

    return !r.first && r.second;
  }

  Bit Bits :: operator!= (const Bits& other) const {
    return this->operator== (other).negate ();
  }

  Bit Bits :: operator< (const Bits& other) const {
    BitPair r = compare (*this, other);

    return r.first && !r.second;
  }

  Bit Bits :: operator<= (const Bits& other) const {
    BitPair r = compare (*this, other);

    return r.first || r.second;
  }

  Bit Bits :: operator>= (const Bits& other) const {
    return other.operator<= (*this);
  }

  Bit Bits :: operator> (const Bits& other) const {
    return other.operator< (*this);
  }

  Bit Bits :: operator&& (const Bits& other) const {
    Bits both (this->_bits);
    both.add (other);
    return both.all ();
  }

  Bit Bits :: operator|| (const Bits& other) const {
    Bits both (this->_bits);
    both.add (other);
    return both.any ();
  }

  const Bit& Bits :: operator[] (size_t i) const {
    return this->_bits [i];
  }

  size_t Bits :: size () const {
    return this->_bits.size ();
  }

  void Bits :: add (const Bit& bit) {
    this->_bits.push_back (bit);
  }

  void Bits :: add (const Bits& bits) {
    for (const Bit& b : bits) {
      this->add (b);
    }
  }

  bool Bits :: hasValue () const {
    return std::all_of (this->begin (), this->end (), [] (const Bit& b) {
      return b.hasValue ();
    });
  }

  std::vector <bool> Bits :: value () const {
    assert (this->hasValue ());

    std::vector <bool> result;
    for (const Bit& b : *this) {
      result.push_back (b.value ());
    }
    return result;
  }

  unsigned int Bits :: valueNat () const {
    assert (this->hasValue ());
    return fromBinary (this->value ());
  }

  Bits Bits :: negate () const {
    std::vector <Bit> negBits;
    for (const Bit& b : this->_bits) {
      negBits.push_back (!b);
    }
    return Bits (negBits);
  }

  void Bits :: assertAny ()               const {        cppsat::assertAny (this->_bits);    }
  void Bits :: assertAll ()               const {        cppsat::assertAll (this->_bits);    }
  Bit  Bits :: all       ()               const { return cppsat::all       (this->_bits);    }
  Bit  Bits :: any       ()               const { return cppsat::any       (this->_bits);    }
  Bit  Bits :: none      ()               const { return cppsat::none      (this->_bits);    }
  Bit  Bits :: atmost    (unsigned int k) const { return cppsat::atmost    (k, this->_bits); }
  Bit  Bits :: exactly   (unsigned int k) const { return cppsat::exactly   (k, this->_bits); }

  const std::vector <Bit>&          Bits :: vector () const { return this->_bits; }
  std::vector <Bit>::iterator       Bits :: begin  ()       { return this->_bits.begin  (); }
  std::vector <Bit>::const_iterator Bits :: begin  () const { return this->_bits.begin  (); }
  std::vector <Bit>::const_iterator Bits :: cbegin () const { return this->_bits.cbegin (); }
  std::vector <Bit>::iterator       Bits :: end    ()       { return this->_bits.end    (); }
  std::vector <Bit>::const_iterator Bits :: end    () const { return this->_bits.end    (); }
  std::vector <Bit>::const_iterator Bits :: cend   () const { return this->_bits.cend   (); }

  void assertAll (const std::vector <Bit>& bits) {
    for (const Bit& bit : bits) {
      cppsat::assertAny ({ bit });
    }
  }

  void assertAny (const std::vector <Bit>& bits) {
    if (std::none_of ( bits.begin (), bits.end ()
                     , [] (const Bit& b) { return b.hasValue (true); } ))
    {
      Minisat::vec <Minisat::Lit> vec;
      vec.capacity (bits.size ());

      for (const Bit& bit : bits) {
        if (bit.hasValue () == false) {
          vec.push (toMinisatLit (bit));
        }
      }
      solver.addClause (vec);
    }
  }

  Bit all (const std::vector <Bit>& bits) {
    if (bits.size () == 0) {
      return Bit (true);
    }
    else if (bits.size () == 1) {
      return *bits.begin ();
    }
    else if (std::any_of ( bits.begin (), bits.end ()
                         , [] (const Bit& b) { return b.hasValue (false); } ))
    {
      return Bit (false);
    }
    else if (std::all_of ( bits.begin (), bits.end ()
                         , [] (const Bit& b) { return b.hasValue (true); } ))
    {
      return Bit (true);
    }
    else {
      Bit result;
      for (const Bit& bit : bits) {
        cppsat::assertAny ({ result.negate (), bit }); 
      }
      std::vector <Bit> negBits (Bits (bits).negate ().vector ());
      negBits.push_back (result);
      cppsat::assertAny (negBits);

      return result;
    }
  }

  Bit any (const std::vector <Bit>& bits) {
    return Bits (bits).negate ().all ().negate ();
  }

  Bit none (const std::vector <Bit>& bits) {
    return cppsat::any (bits).negate ();
  }

  Bit atmost (unsigned int k, const std::vector <Bit>& bits) {
    if (k == 0) {
      return cppsat::none (bits);
    }
    else if (bits.empty ()) {
      return Bit (true);
    }
    else {
      const Bit&              firstBit (bits.front ());
      const std::vector <Bit> lessBits (++bits.begin (), bits.end ());

      return (firstBit.negate () && cppsat::atmost (k  , lessBits)) 
          || (firstBit           && cppsat::atmost (k-1, lessBits));
    }
  }

  Bit exactly (unsigned int k, const std::vector <Bit>& bits) {
    if (k == 0) {
      return cppsat::none (bits);
    }
    else if (bits.empty ()) {
      return Bit (false);
    }
    else {
      const Bit&              firstBit (bits.front ());
      const std::vector <Bit> lessBits (++bits.begin (), bits.end ());

      return (firstBit.negate () && cppsat::exactly (k  , lessBits)) 
          || (firstBit           && cppsat::exactly (k-1, lessBits));
    }
  }

  Bit allEqual (const std::vector <Bits>& bits) {
    assert (bits.size () > 0);

    return cppsat::forall ( bits.begin (), bits.end ()
                          , [&bits] (const Bits& b)
    {
      return b == bits[0];
    });
  }

  Bit allDifferent (const std::vector <Bits>& bits) {
    Bits diffs;
    for (size_t i = 0; i < bits.size () - 1; i++) {
      for (size_t j = i + 1; j < bits.size (); j++) {
        diffs.add (bits[i] != bits[j]);
      }
    }
    return diffs.all ();
  }

  bool solve () {
    return solver.solve ();
  }

  bool solve (Bit bit) {
    cppsat::assertAny ({ bit });
    return cppsat::solve ();
  }

  void reset () {
    solver.model.clear ();
  }

  void printStatistics () {
    std::cerr << "#variables: "  << solver.nVars ()
              << ", #clauses: "  << solver.nClauses ()
              << ", #literals: " << solver.clauses_literals
              << std::endl;
  }
}

std::ostream& operator<< (std::ostream& os, const cppsat::Bit& bit) {
  if (bit.hasValue ()) {
    os << bit.value ();
  }
  else {
    os << bit.literal ();
  }
  return os;
}

std::ostream& operator<< (std::ostream& os, const cppsat::Bits& bits) {
  auto it = bits.begin ();
  os << "[";
  if (it != bits.end ()) {
    os << *it;
  }
  ++it;
  for ( ; it != bits.end (); ++it) {
    os << ", " << *it;
  }
  os << "]";
  return os;
}
