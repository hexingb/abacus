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

const Number operator+(const Number &lhs, const Number &rhs) {
  Number result;
  if (lhs.negative_ == true && rhs.negative_ == false) {
    return rhs - abs(lhs);
  }

  if (lhs.negative_ == false && rhs.negative_ == true) {
    return lhs - abs(rhs);
  }
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
