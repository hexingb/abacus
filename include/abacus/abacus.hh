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
  for (; rch != ch; --rch) {
    decimalHandler(*rch - '0');
  }
  return true;
}

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

  Number(const std::string &input, Word base = 10) {
    digits_ = std::make_shared<std::vector<Byte>>();
    Word quotient = 0;
    Word remainder = 0;
    std::vector<Byte> quotient_series;

    auto handler = [&](Word digit) {
      OnePassBaseConvert(digit, quotient, remainder, base, ABACUS_BYTE_MAX);
      if (quotient_series.size() > 0 || quotient != 0) {
        quotient_series.push_back(quotient);
      }
    };

    auto trail_hook = [&]() {
      digits_->push_back(remainder);

      if (quotient_series.size() > 0) {
        store_digits(quotient_series, base, ABACUS_BYTE_MAX);
      }
      if (point_pos_ == 0) {
        point_pos_ = digits_->size();
      }
    };

    num_read(
        input, base, [&](char sign) { negative_ = sign == '-' ? true : false; },
        handler,
        [&]() {
          trail_hook();

          // clear the circumstance for decimal part
          quotient = 0;
          remainder = 0;
          quotient_series.clear();
        },
        handler);

    trail_hook();

    // special case: remove the trailing .0
    if (point_pos_ == digits_->size() - 1 && (*digits_)[point_pos_] == 0) {
      digits_->pop_back();
    }

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

// TODO thread local
static Word g_out_base = 10;

inline void out_base(Word base) { g_out_base = base; }

inline Word out_base() { return g_out_base; }

static void print_integer(bool negative, const std::vector<char> &digits,
                          std::ostream &os) {
  os << std::string(negative ? "-" : "");
  for (auto iter = digits.rbegin(); iter != digits.rend(); ++iter) {
    os << *iter;
  }
}

static void print_decimal(const std::vector<char> &digits, std::ostream &os) {
  for (auto iter = digits.begin(); iter != digits.end(); ++iter) {
    os << *iter;
  }
}

std::ostream &operator<<(std::ostream &os, const Number &num) {
  std::vector<Byte> result;

  std::copy(num.digits_->rbegin() + (num.digits_->size() - num.point_pos_),
            num.digits_->rend(), std::back_inserter(result));

  std::vector<char> printable_integer;
  BaseConvert(result, ABACUS_BYTE_MAX, out_base(), [&](Word digit) {
    printable_integer.push_back((char)(digit + '0'));
  });

  result.clear();
  std::copy(num.digits_->rbegin(),
            num.digits_->rbegin() + (num.digits_->size() - num.point_pos_),
            std::back_inserter(result));

  std::vector<char> printable_decimal;
  BaseConvert(result, ABACUS_BYTE_MAX, out_base(), [&](Word digit) {
    printable_decimal.push_back((char)(digit + '0'));
  });

  print_integer(num.negative_, printable_integer, os);
  if (printable_decimal.size() > 0) {
    os << ".";
    print_decimal(printable_decimal, os);
  }

  return os;
}

}  // namespace abacus

#endif
