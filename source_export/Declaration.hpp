#ifndef MPD_DECLARATION_H
#define MPD_DECLARATION_H

#include <string>
#include <functional>

namespace MPD
{
  class Declaration
  {
  public:
    Declaration(std::string name, std::string parent);

    ~Declaration();
    
    operator std::string() const{ return toString()};

    std::string toString() const;
    
  private:
    std::string _name, _parent;
  };

  struct QuantityDeclParams
  {
    std::string name;
    std::string parent;
    std::string dimension;
    std::function<double(MPDState&)> compute;
  };
  
  class QuantityDecl : public Declaration
  {
  public:
    QuantityDecl(QuantityDeclParams &args);

    ~QuantityDecl();

    operator std::string() const{ return toString()};

    std::string toString() const;
    
  private:
    std::string _name, _parent, _dimension;
  };

  class DerivedQuantityDecl : public QuantityDecl
  {
  public:
    QuantityDecl(QuantityDeclParams &args);

    ~QuantityDecl();

    operator std::string() const{ return toString()};

    std::string toString() const;

    double compute(MPDState &state); 
    
    void update(MPDState &state);

    
  private:
    std::string _name, _parent, _dimension;
    std::function<double(MPDState&)> _compute;
  };

  struct LawParams
  {
    std::string name;
    std::string parent;
    std::vector<std::string> variables;
    std::unordered_map<std::string, std::function<double(MPDState&)>> rules;
  };
  
  class Law : public Declaration
  {
  public:
    Law(LawParams &args);

    ~Law();

    operator std::string() const{ return toString()};

    std::string toString() const;

    std::function<double(MPDState&)>> getRule(std::string ruleSubject) const;

    std::function<double(MPDState&)>> operator[](std::string ruleSubject) const { getRule(ruleSubject); };

    
  private:
    std::string _name, _parent;
    std::vector<std::string> _variables;
    std::unordered_map<std::string, std::function<double(MPDState&)>> _rules;
  };
  
}


#endif // MPD_DECLARATION_H
