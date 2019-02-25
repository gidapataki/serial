#pragma once
#include <memory>
#include <vector>


namespace serial {

class Reader;
class Writer;
class ReferableBase;
class FactoryBase;
class Registry;

template<typename T> class Referable;
template<typename T> class Factory;
template<typename T> using Array = std::vector<T>;

using Ref = const ReferableBase*;
using UniqueRef = std::unique_ptr<ReferableBase>;
using TypeId = int*;

struct Header;
struct DeserializeResult;

} // namespace serial
