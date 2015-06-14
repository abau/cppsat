#ifndef CPPSAT
#define CPPSAT

#include <iosfwd>
#include <vector>

namespace cppsat {

  class Bit {
    public:
               Bit ();
      explicit Bit (bool);

      Bit  operator!  () const;
      Bit  operator&& (const Bit&) const;
      Bit  operator|| (const Bit&) const;
      Bit  operator== (const Bit&) const;
      Bit  operator!= (const Bit&) const;

      bool isConstant  () const;
      int  literal     () const;
      bool hasValue    () const;
      bool hasValue    (bool) const;
      bool value       () const;
      Bit  negate      () const;
      Bit  implies     (const Bit&) const;
      Bit  equals      (const Bit&) const;
      Bit  equalsNot   (const Bit&) const;
      Bit  ifThenElse  (const Bit&, const Bit&) const;

    private:
      explicit Bit (int);

      const bool _isConstant;
      const int  _value;
  };

  class Bits {
    public:
      Bits ();
      Bits (size_t);
      Bits (size_t, unsigned int);
      Bits (const std::vector <Bit>&);
      Bits (const std::vector <bool>&);

            Bits operator!  () const;
            Bits operator+  (const Bits&) const;
            Bits operator*  (const Bits&) const;
            Bit  operator== (const Bits&) const;
            Bit  operator!= (const Bits&) const;
            Bit  operator<  (const Bits&) const;
            Bit  operator<= (const Bits&) const;
            Bit  operator>= (const Bits&) const;
            Bit  operator>  (const Bits&) const;
            Bit  operator&& (const Bits&) const;
            Bit  operator|| (const Bits&) const;
      const Bit& operator[] (size_t) const;

      size_t             size      () const;
      void               add       (const Bit&);
      void               add       (const Bits&);
      bool               hasValue  () const;
      std::vector <bool> value     () const;
      unsigned int       valueNat  () const;

      Bits               negate    () const;
      void               assertAll () const;
      void               assertAny () const;
      Bit                all       () const;
      Bit                any       () const;
      Bit                none      () const;
      Bit                atmost    (unsigned int) const;
      Bit                exactly   (unsigned int) const;

      const std::vector <Bit>&          vector () const;
      std::vector <Bit>::iterator       begin  ();
      std::vector <Bit>::const_iterator begin  () const;
      std::vector <Bit>::const_iterator cbegin () const;
      std::vector <Bit>::iterator       end    ();
      std::vector <Bit>::const_iterator end    () const;
      std::vector <Bit>::const_iterator cend   () const;

    private:
      std::vector <Bit> _bits;
  };

  void assertAll       (const std::vector <Bit>&);
  void assertAny       (const std::vector <Bit>&);
  Bit  all             (const std::vector <Bit>&);
  Bit  any             (const std::vector <Bit>&);
  Bit  none            (const std::vector <Bit>&);
  Bit  atmost          (unsigned int, const std::vector <Bit>&);
  Bit  exactly         (unsigned int, const std::vector <Bit>&);
  Bit  allEqual        (const std::vector <Bits>&);
  Bit  allDifferent    (const std::vector <Bits>&);

  bool solve           ();
  bool solve           (Bit);
  void reset           ();
  void printStatistics ();

  template <typename It, typename F>
  Bit forall (It first, It last, F f) {
    std::vector <Bit> bits;
    for (; first != last; ++first) {
      bits.push_back (f (*first));
    }
    return cppsat::all (bits);
  }

  template <typename It, typename F>
  Bit exists (It first, It last, F f) {
    std::vector <Bit> bits;
    for (; first != last; ++first) {
      bits.push_back (f (*first));
    }
    return cppsat::any (bits);
  }
}

std::ostream& operator<<(std::ostream&, const cppsat::Bit&);
std::ostream& operator<<(std::ostream&, const cppsat::Bits&);

#endif
