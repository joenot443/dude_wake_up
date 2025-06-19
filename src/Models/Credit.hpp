#ifndef Credit_hpp
#define Credit_hpp

#include <string>

class Credit {
  public:
    std::string name;
    std::string credit;
    std::string description;

    Credit(std::string name, std::string credit, std::string description) : name(name), credit(credit), description(description) {
    }
};

#endif