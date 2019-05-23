#include <iostream>
#include <string_view>
#include <tuple>
#include <string>

#define GET_TDEF(x, y) typename decltype(x)::y

template<typename T>
inline constexpr bool is_void = std::is_same_v<void, T>;


template<bool b>
struct static_asserter {
  static_asserter() {
    static_assert(b);
  }
};

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
bool constexpr peek_uint() {
  if constexpr (T::empty()) {
    return false;
  } else {
    if constexpr ((T::front() >= '0') && (T::front() <= '9')) {
      return true;
    } else {
      return false;
    }
  }
}

template<typename T>
bool constexpr peek_var() {
  if constexpr (T::empty()) {
    return false;
  } else {
    if constexpr ((T::front() >= 'a') && (T::front() <= 'z')) {
      return true;
    } else {
      return false;
    }
  }
}

template<typename T>
bool constexpr peek_int() {
  if constexpr (T::empty()) {
    return false;
  } else {
    if constexpr (T::front() == '-') {
      return true;
    } else {
      return peek_uint<T>();
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
struct chain_and_ast {
  using chain = T;
  using ast = A;
};

template<typename T, int i>
struct chain_and_int {
  using chain = T;
  static int constexpr value() {
    return i;
  }
};

template<typename T, char c>
struct chain_and_char {
  using chain = T;
  static char constexpr value() {
    return c;
  }
};

template<typename C, typename V>
struct chain_and_var {
  using chain = C;
  using var = V;
};

template<typename T>
struct ast_holder {
  using ast = T;
};

template<typename T>
struct chain_holder {
  using chain = T;
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

  template <typename T>
  static chain_holder<T> constexpr inner_reverse(chain_holder<T> before) {
    return chain_holder<T>{};
  }

  static chain_holder<char_chain_terminator> constexpr reverse() {
    return chain_holder<char_chain_terminator>{};
  }
};

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

  static auto constexpr reverse() {
    return tail::inner_reverse(chain_holder<char_chain<c, char_chain_terminator>>{});
  }

  template <typename T>
  static auto constexpr inner_reverse(chain_holder<T> before) {
    return tail::inner_reverse(chain_holder<char_chain<c, T>>{});
  }
};

template<int i>
struct ast_int {
  static int constexpr value() {
    return i;
  }

  template<typename C, int j>
  static auto constexpr substitute() {
    return ast_holder<ast_int<i>>{};
  }
};

template<typename A, typename B>
struct ast_sum {
  static int constexpr value() {
    return A::value() + B::value();
  }

  template<typename C, int i>
  static auto constexpr substitute() {
    auto constexpr a_sub = A::template substitute<C, i>();
    auto constexpr b_sub = B::template substitute<C, i>();
    return ast_holder<ast_sum<GET_TDEF(a_sub, ast), GET_TDEF(b_sub, ast)>>{};
  }
};

template<typename A, typename B>
struct ast_sub {
  static int constexpr value() {
    return A::value() - B::value();
  }

  template<typename C, int i>
  static auto constexpr substitute() {
    auto constexpr a_sub = A::template substitute<C, i>();
    auto constexpr b_sub = B::template substitute<C, i>();
    return ast_holder<ast_sub<GET_TDEF(a_sub, ast), GET_TDEF(b_sub, ast)>>{};
  }
};

template<typename A, typename B>
struct ast_div {
  static int constexpr value() {
    return A::value() / B::value();
  }

  template<typename C, int i>
  static auto constexpr substitute() {
    auto constexpr a_sub = A::template substitute<C, i>();
    auto constexpr b_sub = B::template substitute<C, i>();
    return ast_holder<ast_div<GET_TDEF(a_sub, ast), GET_TDEF(b_sub, ast)>>{};
  }
};

template<typename A, typename B>
struct ast_product {
  static int constexpr value() {
    return A::value() * B::value();
  }

  template<typename C, int i>
  static auto constexpr substitute() {
    auto constexpr a_sub = A::template substitute<C, i>();
    auto constexpr b_sub = B::template substitute<C, i>();
    return ast_holder<ast_product<GET_TDEF(a_sub, ast), GET_TDEF(b_sub, ast)>>{};
  }
};

template<typename V>
struct ast_var {
  using chain = V;

  template<typename C, int i>
  static auto constexpr substitute() {
    if constexpr (std::is_same_v<V, C>) {
      return ast_holder<ast_int<i>>{};
    } else {
      return ast_holder<ast_var<V>>{};
    }
  }
};

template<typename T>
auto constexpr pry_uint() {
  if constexpr (T::empty()) {
    return chain_and_int<T, 0>{};
  } else {
    if constexpr (peek_uint<T>()) {
      auto constexpr rem = pry_uint<typename T::tail>();
      int constexpr places = T::size() - decltype(rem)::chain::size() - 1;
      int constexpr front = (T::front() - '0');
      int constexpr mult = int_power<10, places>::value();
      int constexpr result = front * mult + rem.value();
      return chain_and_int<GET_TDEF(rem, chain), result>{};
    } else {
      return chain_and_int<T, 0>{};
    }
  }
}

template<typename T>
auto constexpr pry_int() {
  if constexpr (T::empty()) {
    return chain_and_int<T, 0>{};
  } else {
    if constexpr (T::front() == '-') {
      auto constexpr rem = pry_uint<typename T::tail>();
      return chain_and_int<GET_TDEF(rem, chain), -rem.value()>{};
    } else {
      return pry_uint<T>();
    }
  }
}

template<typename T>
auto constexpr pry_var() {
  if constexpr (T::empty()) {
    return chain_and_var<T, char_chain_terminator>{};
  } else {
    if constexpr (peek_var<T>()) {
      auto constexpr rem = pry_var<typename T::tail>();
      return chain_and_var<GET_TDEF(rem, chain), char_chain<T::front(), GET_TDEF(rem, var)>>{};
    } else {
      return chain_and_var<T, char_chain_terminator>{};
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

    using after = GET_TDEF(mid, chain);

    //static_assert(peek_char<after, ')'>());
    static_assert(after::front() == ')');

    using beyond = typename after::tail; // consume )
    return chain_and_ast<beyond, GET_TDEF(mid, ast)>{};
  } else {
    if constexpr (peek_int<T>()) {
      auto constexpr result = pry_int<T>();
      return chain_and_ast<GET_TDEF(result, chain), ast_int<result.value()>>{};
    } else {
      static_assert(peek_var<T>());
      auto constexpr result = pry_var<T>();
      return chain_and_ast<GET_TDEF(result, chain), ast_var<GET_TDEF(result, var)>>{};
    }
  }
}

template<typename T, typename A>
auto constexpr divterm_tail() {
  if constexpr (peek_char<T, '/'>()) {
    using next = typename T::tail; // consume -
    auto constexpr right_value = value<next>();
    using next2 = GET_TDEF(right_value, chain);

    using value = ast_div<A, GET_TDEF(right_value, ast)>;

    return divterm_tail<next2, value>();

  } else {
    return chain_and_ast<T, A>{};

  }
}

template<typename T>
auto constexpr divterm() {
  auto constexpr left_value = value<T>();
  using next = GET_TDEF(left_value, chain);

  return divterm_tail<next, GET_TDEF(left_value, ast)>();
}


template<typename T>
auto constexpr multterm();

template<typename T>
auto constexpr multterm_tail() {
  if constexpr (peek_char<T, '*'>()) {
    using next = typename T::tail; // consume *
    return multterm<next>();
  } else {
    return chain_and_ast<T, void>{};
  }
}

template<typename T>
auto constexpr multterm() {
  auto constexpr left_value = divterm<T>();
  using next = GET_TDEF(left_value, chain);

  auto constexpr right_value = multterm_tail<next>();
  using after = GET_TDEF(right_value, chain);

  if constexpr (is_void<GET_TDEF(right_value, ast)>) {
    return chain_and_ast<after, GET_TDEF(left_value, ast)>{};
  } else {
    return chain_and_ast<after, ast_product<GET_TDEF(left_value, ast), GET_TDEF(right_value, ast)>>{};
  }
}

template<typename T, typename A>
auto constexpr subterm_tail() {
  if constexpr (peek_char<T, '-'>()) {
    using next = typename T::tail; // consume -
    auto constexpr right_value = multterm<next>();
    using next2 = GET_TDEF(right_value, chain);

    using value = ast_sub<A, GET_TDEF(right_value, ast)>;

    return subterm_tail<next2, value>();

  } else {
    return chain_and_ast<T, A>{};

  }
}

template<typename T>
auto constexpr subterm() {
  auto constexpr left_value = multterm<T>();
  using next = GET_TDEF(left_value, chain);

  return subterm_tail<next, GET_TDEF(left_value, ast)>();
}


template<typename T>
auto constexpr addterm();

template<typename T>
auto constexpr addterm_tail() {
  if constexpr (peek_char<T, '+'>()) {
    using next = typename T::tail; // consume +
    return addterm<next>();
  } else {
    return chain_and_ast<T, void>{};
  }
}

template<typename T>
auto constexpr addterm() {
  auto constexpr left_value = subterm<T>();
  using next = GET_TDEF(left_value, chain);

  auto constexpr right_value = addterm_tail<next>();
  using after = GET_TDEF(right_value, chain);

  if constexpr (is_void<GET_TDEF(right_value, ast)>) {
    return chain_and_ast<after, GET_TDEF(left_value, ast)>{};
  } else {
    return chain_and_ast<after, ast_sum<GET_TDEF(left_value, ast), GET_TDEF(right_value, ast)>>{};
  }
}


template<typename T>
auto constexpr expr() {
  return addterm<T>();
}

template<typename T>
auto constexpr parse() {
  auto constexpr out = expr<T>();
  using after = GET_TDEF(out, chain);
  if constexpr (after::empty()) {
    return ast_holder<GET_TDEF(out, ast)>{};
  } else {
    // Basically assert false
    static_assert(is_void<after>);
  }
}

template<typename T>
int constexpr eval() {
  auto constexpr out = parse<T>();
  return decltype(out)::type::value();
}

template<typename G, size_t idx>
struct chainer {
  using next = char_chain<G::value()[G::value().size() - idx], typename chainer<G, idx - 1>::next>;
};

template<typename G>
struct chainer<G, 0> {
  using next = char_chain_terminator;
};

template<typename G>
struct to_char_chain {
  using chain = typename chainer<G, G::value().size()>::next;
};

struct expression_holder {
  static std::string_view constexpr value() {
    using namespace std::literals;
    return "-2*(2048/4-((8/2/2+2+3*3+(4*x+3-1-1+2)+3*(2--2))*favorite+256)-512)"sv; //1024
    //return "-2*(2048/4-((8/2/2+2+3*3+(4*x+3-1-1+2)+3*(2--2))*4+256)-512)"sv; //1024
    //return "-2*(2048/4-((8/2/2+2+3*3+(4*9+3-1-1+2)+3*(2--2))*4+256)-512)"sv; //1024
    //return "8/(2/2)"sv; //8
    //return "8/2/2"sv; //2
    //return "21+42"sv; //63
    //return "2*3"sv; //6
    //return "2--2"sv; //4
    //return "8-2-3-4"sv; //-1
    //return "6-3-3"sv; //0
  }
};

struct x_holder {
  static std::string_view constexpr value() {
    using namespace std::literals;
    return "x"sv;
  }
};

struct favorite_holder {
  static std::string_view constexpr value() {
    using namespace std::literals;
    return "favorite"sv;
  }
};

int main() {
  using chain = to_char_chain<expression_holder>::chain;
  using x = to_char_chain<x_holder>::chain;
  using favorite = to_char_chain<favorite_holder>::chain;

  /*std::cout << "The chain's type name is: " << typeid(chain).name() << "\n";
  std::cout << "The chain has length: " << chain::size() << "\n";
  std::cout << "The chain has value [" << chain::to_string() << "]\n";

  using reversed = GET_TDEF(chain::reverse(), chain);
  std::cout << "The chain's reversed type name is: " << typeid(reversed).name() << "\n";
  std::cout << "The chain's reveresed value [" << reversed::to_string() << "]\n";

  std::cout << "The chain has front: " << chain::front() << "\n";
  std::cout << "The chain has back: " << chain::back() << "\n";*/

  using ast = GET_TDEF(parse<chain>(), ast);
  auto constexpr sub_ast_holder = ast::substitute<x, 9>();
  using ast_sub = GET_TDEF(sub_ast_holder, ast);
  auto constexpr sub_ast_holder2 = ast_sub::substitute<favorite, 4>();
  using ast_sub2 = GET_TDEF(sub_ast_holder2, ast);
  constexpr int value = ast_sub2::value();

  /*
  std::cout << "The ast type name is: " << typeid(ast).name() << "\n";
  */

  std::cout << "The chain is evaluated to: " << value << "\n";

  using other_ast = ast_sum<ast_int<4>, ast_var<x>>;
  auto constexpr sub_other_ast_holder = other_ast::substitute<x, 3>();
  using sub_other_ast = GET_TDEF(sub_other_ast_holder, ast);
  constexpr int other_value = sub_other_ast::value();
  std::cout << "The other substituted ast is evaluated to: " << other_value << "\n";

  return 0;
}