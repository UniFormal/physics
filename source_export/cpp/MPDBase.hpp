#ifndef MPD_BASE_HPP
#define MPD_BASE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

namespace MPD
{
  class MPDBase
  {
  public:
    MPDBase(std::string name, std::string parent, unsigned regions);

    ~MPDBase();

    virtual void initLaws() = 0;

    virtual void initQuantityDecls() = 0;

    Law operator[](std::string lawKey) const;

    QuantityDecl &operator()(std::string quantityKey) const;

    operator std::string() const{ return toString()};

    std::string toString() const;
    
  private:
    std::string _name, _parent;
    unsigned _regions;
    std::unordered_map<std::string, QuantityDecl> _quantityDecls;
    std::unordered_map<std::string, Law> _laws;

    std::vector<std::pair<std::string, std::string>> _graph;
  };
}

#endif // MPD_BASE_HPP
