#pragma once

#include <vector>
#include <tuple>

template<typename... Values>
using TupleVector = std::vector<std::tuple<Values...>>;

template<class T>
using RawPtr = T*;
