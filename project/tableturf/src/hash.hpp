#pragma once
#include <tuple>
#include <functional>
template<class T> size_t HashCombine(const size_t seed,const T &v){
    return seed^(std::hash<T>()(v)+0x9e3779b9+(seed<<6)+(seed>>2));
}
template<int N> struct HashTupleCore{
    template<class Tuple> size_t operator()(const Tuple &keyval) const noexcept{
        size_t s=HashTupleCore<N-1>()(keyval);
        return HashCombine(s,std::get<N-1>(keyval));
    }
};
template <> struct HashTupleCore<0>{
    template<class Tuple> size_t operator()(const Tuple &keyval) const noexcept{ return 0; }
};
struct hash_tuple{
  template<class... Args> size_t operator()(const std::tuple<Args...> &keyval) const noexcept {
      return HashTupleCore<std::tuple_size<std::tuple<Args...>>::value>()(keyval);
  }
};