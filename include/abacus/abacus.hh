#ifndef ABACUS_ABACUS_HH
#define ABACUS_ABACUS_HH

#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace abacus {

class Number {
 public:
  Number() : digits_{0}, radix_point_pos_{1} {}

  friend const Number operator+(const Number& lhs, const Number& rhs);
  friend const Number operator-(const Number& lhs, const Number& rhs);
  friend const Number operator*(const Number& lhs, const Number& rhs);
  friend const Number operator/(const Number& lhs, const Number& rhs);
  friend const Number operator%(const Number& lhs, const Number& rhs);
  friend std::ostream& operator<<(std::ostream& os, const Number& n);
  friend std::istream& operator>>(std::istream& is, Number& n);

 private:
  bool is_positive() const { return (digits_[0] & 1) == 0; }
  void set_positive() { digits_[0] &= 0xFE; }

  bool is_negative() const { return (digits_[0] & 1); }
  void set_negative() { digits_[0] &= 1; }

 private:
  // digits_[0] is used as bit fields
  std::vector<uint8_t> digits_;
  size_t radix_point_pos_;
};

inline uint8_t char_to_value(uint8_t ch) { return ch - '0'; }
inline uint8_t value_to_char(uint8_t value) { return value + '0'; }

class Reader {
 public:
  Reader(std::istream& is) : is_(is) {}

  bool read() { return true; }

 private:
  std::istream& is_;
};

class Writer {
 public:
  Writer(std::ostream& os) : os_(os) {}

  bool write() { return true; }

 private:
  std::ostream& os_;
};

class Environment {
 public:
  Environment()
      : in_base_(10),
        out_base_(10),
        scale_(0),
        reader_(new Reader(std::cin)),
        writer_(new Writer(std::cout)) {}

  ~Environment() {
    delete reader_;
    delete writer_;
  }

 private:
  uint64_t in_base_;
  uint64_t out_base_;
  uint64_t scale_;
  Reader* reader_;
  Writer* writer_;
};

std::istream& operator>>(std::istream& is, Number& n) {
  uint8_t sign = '+';
  is >> sign;

  if (sign == '-') {
    n.set_negative();
  } else if (sign == '+') {
    n.set_positive();
  } else {
    n.set_positive();
    n.digits_.push_back(char_to_value(sign));
  }

  uint8_t digit = 0;
  while ((is >> digit)) {
    n.digits_.push_back(char_to_value(digit));
  }
  return is;
}

// just for debug;
std::ostream& operator<<(std::ostream& os, const Number& n) {
  os << "radix_point_pos = " << n.radix_point_pos_ << std::endl
     << "digits size = " << n.digits_.size() << std::endl;
  std::copy(n.digits_.begin(), n.digits_.end(),
            std::ostream_iterator<uint8_t>(os, ", "));
  return os;
}

}  // namespace abacus

#endif
