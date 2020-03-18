#ifndef ABACUS_ABACUS_HH
#define ABACUS_ABACUS_HH

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace abacus {

// digits in buffer_ have a base of 2^32
using Byte = uint32_t;
using Word = uint64_t;
const Word ABACUS_BYTE_MAX = (1ul << (sizeof(Byte) * 8));

void OnePassBaseConvert(Word digit, Word &quotient, Word &remainder,
                        Word in_base, Word out_base) {
  Word current_divident = remainder * in_base + digit;
  if (current_divident >= out_base) {
    quotient = current_divident / out_base;
    remainder = current_divident % out_base;
  } else {
    quotient = 0;
    remainder = current_divident;
  }
}

template <typename Load, typename Store>
void IterativeBaseConvert(Load load, Store store, Word in_base, Word out_base) {
  Word digit = 0;
  Word quotient = 0;
  Word remainder = 0;

  while (!load(digit)) {
    OnePassBaseConvert(digit, quotient, remainder, in_base, out_base);
    store(quotient, remainder);
  }
}

// 'quotient_series' must not be empty
template <typename Store>
void BaseConvert(std::vector<Byte> &quotient_series, Word in_base,
                 Word out_base, Store store) {
  size_t start = 0;
  size_t end = quotient_series.size();
  size_t save_pos = 0;
  IterativeBaseConvert(
      [&](Word &digit) {
        bool done = start == end;

        if (done == false) {
          digit = quotient_series[start++];
        }
        return done;
      },
      [&](Word quotient, Word &remainder) {
        if (save_pos != 0 || quotient != 0) {
          quotient_series[save_pos++] = quotient;
        }

        if (start == end) {
          start = 0;
          end = save_pos;
          store(remainder);
          remainder = 0;
          save_pos = 0;
        }
      },
      in_base, out_base);
}

template <typename SignHandler, typename IntegerHandler,
          typename DecimalPointHandler, typename DecimalHandler>
bool num_read(const std::string &number, Word base, SignHandler signHandler,
              IntegerHandler integerHandler,
              DecimalPointHandler decimalPointHandler,
              DecimalHandler decimalHandler) {
  const char *ch = number.c_str();
  for (; *ch != '.' && *ch != '\0'; ++ch) {
    if (*ch == '+' || *ch == '-') {
      signHandler(*ch);
      continue;
    }

    integerHandler(*ch - '0');
  }

  if (*ch == '\0') {
    return true;
  }
  decimalPointHandler();

  const char *rch = number.c_str() + number.size() - 1;
  while (rch > ch && *rch == '0') {  // skip trailing zeros
    --rch;
  }

  for (++ch; ch <= rch; ++ch) {
    decimalHandler(*ch - '0');
  }
  return true;
}

static void DigitCountInByte(Word base, int &bits, int &count) {
  if (sizeof(Byte) >= 4 && base <= (1ul << 32)) {
    bits = 32;
    count = 1;
  }

  if (sizeof(Byte) >= 2 && base <= (1ul << 16)) {  // two bytes for one digit
    bits = 16;
    count = sizeof(Byte) / 2;
  }

  if (sizeof(Byte) >= 1 && base <= (1ul << 8)) {  // one byte for one digit
    bits = 8;
    count = sizeof(Byte);
  }

  if (sizeof(Byte) >= 1 && base <= (1ul << 4)) {  // 4 bits for one digit
    bits = 4;
    count = sizeof(Byte) * 2;
  }
}

static void StoreDigitIntoByte(Byte &byte, Word digit, int idx, int bits) {
  byte |= (digit << (idx * bits));
}

// TODO thread local
static Word g_out_base = 10;
static Word g_in_base = 10;

inline void out_base(Word base) { g_out_base = base; }

inline Word out_base() { return g_out_base; }

inline void in_base(Word base) { g_in_base = base; }

inline Word in_base() { return g_in_base; }

static int g_bits_for_one_digit = 0;
static int g_digits_in_one_unit = 0;
static size_t g_digits_mask = 0;

class Initializer {
 public:
  Initializer(Word ibase = 10, Word obase = 10) {
    in_base(ibase);
    out_base(obase);
    DigitCountInByte(in_base(), g_bits_for_one_digit, g_digits_in_one_unit);
    for (int idx = 0; idx < g_bits_for_one_digit; ++idx) {
      g_digits_mask |= (1 << idx);
    }
  }
};

// TODO specified by user
static Initializer initializer;

class Number {
 public:
  // TODO use Number as a primitive type
  Number(int n = 0) {
    negative_ = n < 0;
    digits_ = std::make_shared<std::vector<Byte>>();
    digits_->push_back(abs(n));  // FIXME abs(-2147483648) ?
    point_pos_ = 1;
  }

  Number(unsigned int) {}
  Number(long) {}
  Number(unsigned long) {}
  Number(double) {}
  Number(float) {}

  Number(const std::string &input) {
    digits_ = std::make_shared<std::vector<Byte>>();
    Word quotient = 0;
    Word remainder = 0;
    std::vector<Byte> quotient_series;

    auto integer_handler = [&](Word digit) {
      OnePassBaseConvert(digit, quotient, remainder, in_base(),
                         ABACUS_BYTE_MAX);
      if (quotient_series.size() > 0 || quotient != 0) {
        quotient_series.push_back(quotient);
      }
    };

    auto integer_process_hook = [&]() {
      if (point_pos_ == 0) {
        digits_->push_back(remainder);

        if (quotient_series.size() > 0) {
          store_digits(quotient_series, in_base(), ABACUS_BYTE_MAX);
        }

        point_pos_ = digits_->size();
      }
    };

    int idx = 0;
    Byte byte = 0;
    auto decimal_process_hook = [&](Word digit) {
      if (idx == g_digits_in_one_unit) {
        digits_->push_back(byte);
        byte = 0;
        idx = 0;
      }
      StoreDigitIntoByte(byte, digit, idx, g_bits_for_one_digit);
      ++idx;
    };

    num_read(
        input, in_base(),
        [&](char sign) { negative_ = sign == '-' ? true : false; },
        integer_handler, integer_process_hook, decimal_process_hook);
    digits_->push_back(byte);  // well I dislike it.

    // in case no decimal point '.' presented
    integer_process_hook();

    // special case: for -0 set negative as false
    if (point_pos_ == 1 && digits_->size() == 1 && digits_->at(0) == 0) {
      negative_ = false;
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const Number &n);
  friend const Number abs(const Number &lhs);
  friend const Number operator+(const Number &lhs, const Number &rhs);
  friend const Number operator-(const Number &lhs, const Number &rhs);
  friend const Number operator*(const Number &lhs, const Number &rhs);
  friend const Number operator/(const Number &lhs, const Number &rhs);
  friend const Number operator%(const Number &lhs, const Number &rhs);
  friend bool operator<(const Number &lhs, const Number &rhs);
  friend bool operator<=(const Number &lhs, const Number &rhs);
  friend bool operator>(const Number &lhs, const Number &rhs);
  friend bool operator>=(const Number &lhs, const Number &rhs);
  friend bool operator==(const Number &lhs, const Number &rhs);
  friend bool operator!=(const Number &lhs, const Number &rhs);

 private:
  void store_digits(std::vector<Byte> &quotient_series, Word in_base,
                    Word out_base) {
    BaseConvert(quotient_series, in_base, out_base,
                [&](Word remainder) { digits_->push_back(remainder); });
  }

 private:
  bool negative_ = false;

  // 'digits_' contains two part of digits: integer digits at ahead and decimal
  // digits later, which are separated by 'point_pos_'.
  size_t point_pos_ = 0;
  std::shared_ptr<std::vector<Byte>> digits_;

#ifdef ABACUS_DEBUG
#define debug_num(num)                                                       \
  do {                                                                       \
    std::cout << (num.negative_ ? "-" : "+") << " " << num.point_pos_ << " " \
              << num.digits_->size();                                        \
    for (auto x : (*num.digits_)) {                                          \
      std::cout << " " << x;                                                 \
    }                                                                        \
    std::cout << std::endl;                                                  \
  } while (0)
#else
#define debug_num(num)
#endif
};  // namespace abacus

typedef Number num_t;

static void print_integer(bool negative, const std::vector<char> &digits,
                          std::ostream &os) {
  os << std::string(negative ? "-" : "");
  for (auto iter = digits.rbegin(); iter != digits.rend(); ++iter) {
    os << *iter;
  }
}

static void print_decimal(std::ostream &os, Byte byte) {
  for (int idx = 0; idx < g_digits_in_one_unit; ++idx) {
    byte >>= (idx * g_bits_for_one_digit);
    // if (byte != 0) {
    // os << static_cast<char>((byte & g_digits_mask) + '0');
    // }
    std::cerr << " -- byte = " << byte
              << " digit: " << static_cast<char>((byte & g_digits_mask) + '0')
              << " idx = " << idx
              << " byte&g_digits_mask = " << (byte & g_digits_mask)
              << std::endl;
  }
  std::cerr << " done g_digits_in_one_unit = " << g_digits_in_one_unit
            << " g_bits_for_one_digit = " << g_bits_for_one_digit << std::endl;
}

std::ostream &operator<<(std::ostream &os, const Number &num) {
  std::vector<Byte> result;

  std::copy(num.digits_->rbegin() + (num.digits_->size() - num.point_pos_),
            num.digits_->rend(), std::back_inserter(result));

  std::vector<char> printable_integer;
  BaseConvert(result, ABACUS_BYTE_MAX, out_base(), [&](Word digit) {
    printable_integer.push_back((char)(digit + '0'));
  });

  print_integer(num.negative_, printable_integer, os);

  size_t pos = num.point_pos_;
  if (pos < num.digits_->size()) {
    os << ".";
  }

  if (in_base() == out_base()) {
    for (; pos < num.digits_->size(); ++pos) {
      print_decimal(os, num.digits_->at(pos));
    }
  } else {
    // FIXME do base conversion with a scale
  }

  return os;
}

}  // namespace abacus

#endif
