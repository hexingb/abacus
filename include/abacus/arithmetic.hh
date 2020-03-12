#ifndef ABACUS_ARITHMETIC_HH
#define ABACUS_ARITHMETIC_HH

#include <abacus/abacus.hh>

namespace abacus {

const Number abs(const Number &lhs) {
  Number result;
  result.negative_ = false;
  result.digits_ = lhs.digits_;
  result.point_pos_ = lhs.point_pos_;
  return result;
}

bool operator<(const Number &lhs, const Number &rhs) {
  debug_num(lhs);
  debug_num(rhs);

  if (lhs.negative_ == true && rhs.negative_ == false) {
    return true;
  }

  if (lhs.negative_ == false && rhs.negative_ == true) {
    return false;
  }

  if (lhs.negative_ == true && rhs.negative_ == true) {
    return !(abs(lhs) < abs(rhs));
  }

  // lhs.negative_ == false && rhs.negative_ == false
  if (lhs.point_pos_ != rhs.point_pos_) {
    return lhs.point_pos_ < rhs.point_pos_;
  }

  // compare integer part
  for (size_t idx = lhs.point_pos_; idx != 0; --idx) {
    if (lhs.digits_->at(idx - 1) != rhs.digits_->at(idx - 1)) {
      return lhs.digits_->at(idx - 1) < rhs.digits_->at(idx - 1);
    }
  }

  // compare decimal part
  size_t idx = lhs.point_pos_;
  for (; idx < lhs.digits_->size() && idx < rhs.digits_->size(); ++idx) {
    if (lhs.digits_->at(idx) != rhs.digits_->at(idx)) {
      return lhs.digits_->at(idx) < rhs.digits_->at(idx);
    }
  }

  return lhs.digits_->size() < rhs.digits_->size();
}

bool operator==(const Number &lhs, const Number &rhs) {
  return lhs.negative_ == rhs.negative_ && lhs.point_pos_ == rhs.point_pos_ &&
         (*lhs.digits_) == (*rhs.digits_);
}

bool operator<=(const Number &lhs, const Number &rhs) {
  return lhs < rhs || lhs == rhs;
}

bool operator>(const Number &lhs, const Number &rhs) { return !(lhs <= rhs); }
bool operator>=(const Number &lhs, const Number &rhs) { return !(lhs < rhs); }
bool operator!=(const Number &lhs, const Number &rhs) { return !(lhs == rhs); }

Byte sum_one_digit_with_carry(Word a, Word b, Word &carry) {
  Word sum = a + b + carry;
  if (sum > ABACUS_BYTE_MAX) {
    sum -= ABACUS_BYTE_MAX;
    carry = 1;
  } else {
    carry = 0;
  }
  return static_cast<Byte>(sum);
}

const Number operator+(const Number &lhs, const Number &rhs) {
  debug_num(lhs);
  debug_num(rhs);

  if (lhs.negative_ == true && rhs.negative_ == false) {
    return rhs - abs(lhs);
  }

  if (lhs.negative_ == false && rhs.negative_ == true) {
    return lhs - abs(rhs);
  }

  Number result;
  if (lhs.negative_ == true && rhs.negative_ == true) {
    result = abs(lhs) + abs(rhs);
    result.negative_ = true;
    return result;
  }

  // lhs and rhs are both positive now, add every corresponding digit

  // add corresponding digits in decimal part, and save them in 'decimal_sum'
  // and 'carry'
  std::vector<Byte> decimal_sum;
  size_t lhs_decimal_magnitude = lhs.digits_->size() - lhs.point_pos_;
  size_t rhs_decimal_magnitude = rhs.digits_->size() - rhs.point_pos_;
  if (lhs_decimal_magnitude < rhs_decimal_magnitude) {
    std::copy(
        rhs.digits_->rbegin(),
        rhs.digits_->rbegin() + rhs_decimal_magnitude - lhs_decimal_magnitude,
        std::back_inserter(decimal_sum));
    rhs_decimal_magnitude = rhs.point_pos_ + lhs_decimal_magnitude - 1;
    lhs_decimal_magnitude = lhs.digits_->size() - 1;
  } else {
    std::copy(
        lhs.digits_->rbegin(),
        lhs.digits_->rbegin() + lhs_decimal_magnitude - rhs_decimal_magnitude,
        std::back_inserter(decimal_sum));
    lhs_decimal_magnitude = lhs.point_pos_ + rhs_decimal_magnitude - 1;
    rhs_decimal_magnitude = rhs.digits_->size() - 1;
  }

  Word carry = 0;
  for (; lhs_decimal_magnitude >= lhs.point_pos_ &&
         rhs_decimal_magnitude >= rhs.point_pos_;
       --lhs_decimal_magnitude, --rhs_decimal_magnitude) {
    Byte sum =
        sum_one_digit_with_carry(lhs.digits_->at(lhs_decimal_magnitude),
                                 rhs.digits_->at(rhs_decimal_magnitude), carry);
    if (decimal_sum.size() > 0 || sum != 0) {  // leading zeros will be omitted
      decimal_sum.push_back(sum);
      std::cout << "insert " << sum << " carry = " << carry << std::endl;
    }
  }

  // now add corresponding digits in integer part
  result.digits_->clear();
  size_t pos = 0;
  for (; pos < lhs.point_pos_ && pos < rhs.point_pos_; ++pos) {
    result.digits_->push_back(sum_one_digit_with_carry(
        lhs.digits_->at(pos), rhs.digits_->at(pos), carry));
  }

  for (; pos < lhs.point_pos_; ++pos) {
    result.digits_->push_back(
        sum_one_digit_with_carry(lhs.digits_->at(pos), 0, carry));
  }

  for (; pos < rhs.point_pos_; ++pos) {
    result.digits_->push_back(
        sum_one_digit_with_carry(rhs.digits_->at(pos), 0, carry));
  }

  result.point_pos_ = result.digits_->size();

  // append decimal part 'decimal_sum' in little endian
  std::copy(decimal_sum.rbegin(), decimal_sum.rend(),
            std::back_inserter(*result.digits_));

  debug_num(result);
  return result;
}

const Number operator-(const Number &lhs, const Number &rhs) {
  Number result;
  return result;
}
const Number operator*(const Number &lhs, const Number &rhs) {
  Number result;
  return result;
}
const Number operator/(const Number &lhs, const Number &rhs) {
  Number result;
  return result;
}
const Number operator%(const Number &lhs, const Number &rhs) {
  Number result;
  return result;
}
}  // namespace abacus

#endif
