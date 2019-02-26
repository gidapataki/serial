#pragma once
#include <memory>
#include <vector>


namespace serial {

struct Header;

class Reader;
class Writer;
class ReferableBase;
class FactoryBase;
class Registry;
class BasicRef;
class TypedRefBase;

template<typename T> class Referable;
template<typename T> class Factory;
template<typename T> using Array = std::vector<T>;
template<typename... Ts> class TypedRef;

using UniqueRef = std::unique_ptr<ReferableBase>;
using RefContainer = std::vector<UniqueRef>;


} // namespace serial
