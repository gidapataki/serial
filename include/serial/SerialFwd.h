#pragma once
#include <memory>
#include <vector>
#include <boost/optional.hpp>


namespace serial {

struct Header;

class Reader;
class Writer;
class ReferableBase;
class FactoryBase;
class Registry;
class Registrator;
class RefBase;

template<typename T> class Referable;
template<typename T> class Factory;
template<typename T> using Array = std::vector<T>;
template<typename T> using Optional = boost::optional<T>;
template<typename... Ts> class Ref;

using UniqueRef = std::unique_ptr<ReferableBase>;
using RefContainer = std::vector<UniqueRef>;

struct noasserts_t {};
extern noasserts_t noasserts;

struct UserPrimitive {};
struct Enum {};

} // namespace serial
