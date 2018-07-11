#include <iostream>
#include <string_view>
#include <tuple>
#include<string>

using namespace std::literals;

template<int a, int b>
struct int_power {
  static int constexpr value() {
    if constexpr (b == 0) {
      return 1;
    } else {
      return a * int_power<a, b - 1>::value();
    }
  }
};

template<typename T>
bool constexpr peek_int() {
  if constexpr (T::empty()) {
    return false;
  } else {
    if constexpr ((T::front() >= 48) && (T::front() <= 57)) {
      return true;
    } else {
      return false;
    }
  }
}

template<typename T, char c>
bool constexpr peek_char() {
  if constexpr (T::empty()) {
    return false;
  } else {
    if constexpr (T::front() == c) {
      return true;
    } else {
      return false;
    }
  }
}

template<typename T, typename A>
struct type_and_ast {
  using type = T;
  using ast = A;
};

template<typename T, int i>
struct type_and_int {
  using type = T;
  static int constexpr value() {
    return i;
  }
};


template<typename T>
struct type_holder {
  using type = T;
};

template<int i>
struct ast_int {
  static int constexpr value() {
    return i;
  }
};

template<typename A, typename B>
struct ast_sum {
  static int constexpr value() {
    return A::value() + B::value();
  }
};

template<typename A, typename B>
struct ast_product {
  static int constexpr value() {
    return A::value() * B::value();
  }
};


template<typename T>
auto constexpr pry_int() {
  if constexpr (T::empty()) {
    return type_and_int<T, 0>{};
  } else {
    if constexpr (peek_int<T>()) {
      auto constexpr rem = pry_int<typename T::tail>();
      int constexpr places = T::size() - decltype(rem)::type::size() - 1;
      int constexpr front = (T::front() - 48);
      int constexpr mult = int_power<10, places>::value();
      int constexpr result = front * mult + rem.value();
      return type_and_int<typename decltype(rem)::type, result>{};
    } else {
      return type_and_int<T, 0>{};
    }
  }
}

template<typename T>
auto constexpr expr();

template<typename T>
auto constexpr value() {
  if constexpr (peek_char<T, '('>()) {
    using next = typename T::tail; // consume (

    auto constexpr mid = expr<next>();

    using after = typename decltype(mid)::type;

    //static_assert(peek_char<after, ')'>());
    static_assert(after::front() == ')');

    using beyond = typename after::tail; // consume )
    return type_and_ast<beyond, typename decltype(mid)::ast>{};
  } else {
    static_assert(peek_int<T>());
    auto constexpr result = pry_int<T>();
    return type_and_ast<typename decltype(result)::type, ast_int<result.value()>>{};
  }
}

template<typename T>
auto constexpr factor();

template<typename T>
auto constexpr factor_tail() {
  if constexpr (peek_char<T, '*'>()) {
    using next = typename T::tail; // consume *
    return factor<next>();
  } else {
    return type_and_ast<T, void>{};
  }
}

template<typename T>
auto constexpr factor() {
  auto constexpr left_value = value<T>();
  using next = typename decltype(left_value)::type;

  auto constexpr right_value = factor_tail<next>();
  using after = typename decltype(right_value)::type;

  if constexpr (std::is_same_v<void, typename decltype(right_value)::ast>) {
    return type_and_ast<after, typename decltype(left_value)::ast>{};
  } else {
    return type_and_ast<after, ast_product<typename decltype(left_value)::ast, typename decltype(right_value)::ast>>{};
  }
}

template<typename T>
auto constexpr term();

template<typename T>
auto constexpr term_tail() {
  if constexpr (peek_char<T, '+'>()) {
    using next = typename T::tail; // consume +
    return term<next>();
  } else {
    return type_and_ast<T, void>{};
  }
}

template<typename T>
auto constexpr term() {
  auto constexpr left_value = factor<T>();
  using next = typename decltype(left_value)::type;

  auto constexpr right_value = term_tail<next>();
  using after = typename decltype(right_value)::type;

  if constexpr (std::is_same_v<void, typename decltype(right_value)::ast>) {
    return type_and_ast<after, typename decltype(left_value)::ast>{};
  } else {
    return type_and_ast<after, ast_sum<typename decltype(left_value)::ast, typename decltype(right_value)::ast>>{};
  }
}


template<typename T>
auto constexpr expr() {
  return term<T>();
}

template<typename T>
auto constexpr parse() {
  auto constexpr out = expr<T>();
  using after = typename decltype(out)::type;
  if constexpr (after::empty()) {
    return type_holder<typename decltype(out)::ast>{};
  } else {
    // Basically assert false
    static_assert(std::is_same_v<after, void>);
  }
}

template<typename T>
int constexpr eval() {
  auto constexpr out = parse<T>();
  return decltype(out)::type::value();
}

template<char c, typename next>
struct char_chain {
  using tail = next;
  static size_t constexpr size() {
    return 1 + tail::size();
  }
  static char constexpr front() {
    return c;
  }
  static char constexpr back() {
    if constexpr (tail::empty()) {
      return c;
    } else {
      return tail::back();
    }
  }
  static std::string to_string() {
    return std::string(1, c) + tail::to_string();
  }
  static bool constexpr empty() {
    return false;
  }
};

struct char_chain_terminator {
  static size_t constexpr size() {
    return 0;
  }
  static std::string to_string() {
    return {};
  }
  static bool constexpr empty() {
    return true;
  }
};

template<typename G, size_t idx>
struct chainer {
  using type = char_chain<G::value()[G::value().size() - idx], typename chainer<G, idx - 1>::type>;
};

template<typename G>
struct chainer<G, 0> {
  using type = char_chain_terminator;
};

template<typename G>
struct to_char_chain {
  using type = typename chainer<G, G::value().size()>::type;
};



struct string_holder {
  static std::string_view constexpr value() {
    return "(2+2+3*3+(4*9+3)+3*(2+2))*4+256"sv;
    //return "21+42"sv;
    //return "2*3"sv;
  }
};

int main() {
  using chain = to_char_chain<string_holder>::type;
  //std::cout << "The chain's type name is: " << typeid(chain).name() << "\n";
  //std::cout << "The chain has length: " << chain::size() << "\n";
  //std::cout << "The chain has value [" << chain::to_string() << "]\n";
  //std::cout << "The chain has front: " << chain::front() << "\n";
  //std::cout << "The chain has back: " << chain::back() << "\n";

  constexpr auto parsed = parse<chain>();
  using ast = decltype(parsed)::type;
  constexpr int value = ast::value();
  //std::cout << "The ast type name is: " << typeid(ast).name() << "\n";


  //constexpr int value = eval<chain>();

  std::cout << "The chain is evaluated to: " << value << "\n";
}