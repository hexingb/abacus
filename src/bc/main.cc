#include <abacus/abacus.hh>
#include <abacus/arithmetic.hh>
#include <abacus/version.hh>
#include <iostream>

int main(int argc, char **argv) {
  std::cout << abacus::version() << std::endl;
  abacus::Number a(argv[1]);
  abacus::Number b(argv[2]);

  abacus::out_base(10);
  bool less_than = a < b;
  std::cout << a << (less_than ? " < " : " >= ") << b << std::endl;

  bool equal = a == b;
  std::cout << a << (equal ? " == " : " != ") << b << std::endl;

  bool abs_equal = a == abs(a);
  std::cout << a << (abs_equal ? " == " : " != ") << abs(a) << std::endl;

  abacus::Number c;
  bool null_equal = c == abs(c);
  std::cout << c << (null_equal ? " == " : " != ") << abs(c) << std::endl;

  abacus::Number d = -37;
  bool primitive_equal = d == abs(d);
  std::cout << d << (primitive_equal ? " == " : " != ") << abs(d) << std::endl;
  return 0;
}
