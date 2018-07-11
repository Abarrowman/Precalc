#include <iostream>
#include <string_view>
#include <tuple>
#include<string>

using namespace std::literals;

template<int a, int b>
struct IntPower {
  static int constexpr value() {
    if constexpr (b == 0) {
      return 1;
    } else {
      return a * IntPower<a, b - 1>::value();
    }
  }
};

template<typename T>
int constexpr parse_int(T msg) {
  if constexpr (msg().empty()) {
    return 0;
  } else {
    return (msg().back() - 48) + 10 * parse_int([msg] {
      return msg().substr(0, msg().size() - 1);
    });
  }
}

template<typename T>
bool constexpr peek_int(T msg) {
  if constexpr (msg().empty()) {
    return false;
  } else {
    if constexpr ((msg().front() >= 48) && (msg().front() <= 57)) {
      return true;
    } else {
      return false;
    }
  }
}

template<typename T, char c>
bool constexpr peek_char(T msg) {
  if constexpr (msg().empty()) {
    return false;
  } else {
    if constexpr (msg().front() == c) {
      return true;
    } else {
      return false;
    }
  }
}

template<typename T>
auto constexpr skip_char(T msg) {
  return [msg] { return msg().substr(1); };
}

template<typename T>
auto constexpr pry_int(T msg) {
  if constexpr (msg().empty()) {
    return std::make_tuple(msg, 0);
  } else {
    if constexpr (peek_int(msg)) {
      auto constexpr rem = pry_int([msg] { return msg().substr(1); });
      int constexpr places = msg().size() - std::get<0>(rem)().size() - 1;
      int constexpr front = (msg().front() - 48);
      int constexpr mult = IntPower<10, places>::value();
      int constexpr result = front * mult + std::get<1>(rem);
      return std::make_tuple(std::get<0>(rem), result);
    } else {
      return std::make_tuple(msg, 0);
    }
  }
}

template<typename T>
auto constexpr expr(T msg);

template<typename T>
auto constexpr value(T msg) {
  if constexpr (peek_char<T, '('>(msg)) {
    auto constexpr next = skip_char(msg);

    auto constexpr mid = expr(next);

    auto constexpr after = std::get<0>(mid);
    static_assert(peek_char<decltype(after), ')'>(after));
    auto constexpr beyond = skip_char(after);

    return std::make_tuple(beyond, std::get<1>(mid));
  } else {
    static_assert(peek_int(msg));
    return pry_int(msg);
  }
}

template<typename T>
auto constexpr factor_tail(T msg) {
  if constexpr (peek_char<T, '*'>(msg)) {
    auto constexpr next = skip_char(msg);
    return factor(next);
  } else {
    return std::make_tuple(msg, 1);
  }
}

template<typename T>
auto constexpr factor(T msg) {
  auto constexpr leftTup = value(msg);
  auto constexpr rightTup = factor_tail(std::get<0>(leftTup));
  int constexpr product = std::get<1>(leftTup) * std::get<1>(rightTup);
  return std::make_tuple(std::get<0>(rightTup), product);
}

template<typename T>
auto constexpr term_tail(T msg) {
  if constexpr (peek_char<T, '+'>(msg)) {
    auto constexpr next = skip_char(msg);
    return term(next);
  } else {
    return std::make_tuple(msg, 0);
  }
}

template<typename T>
auto constexpr term(T msg) {
  auto constexpr leftTup = factor(msg);
  auto constexpr rightTup = term_tail(std::get<0>(leftTup));
  int constexpr sum = std::get<1>(leftTup) + std::get<1>(rightTup);
  return std::make_tuple(std::get<0>(rightTup), sum);
}


template<typename T>
auto constexpr expr(T msg) {
  return term(msg);
}

template<typename T>
int constexpr eval(T msg) {
  auto constexpr out = expr(msg);
  if constexpr (std::get<0>(out)().empty()) {
    return std::get<1>(out);
  } else {
    return -1;
  }
}

int main() {
  int constexpr evaled = eval([]() { return "(2+2+3*3+(4*9+3)+3*(2+2))*4+256"sv; });
  //int constexpr evaled = eval([] { return "2+2"sv; });
  std::cout << "Evaled: " << evaled << "\n";
}