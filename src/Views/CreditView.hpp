#ifndef CreditView_hpp
#define CreditView_hpp

#include "Credit.hpp"
#include <memory>

class CreditView {
  public:
    static void draw(std::string popupId, std::shared_ptr<Credit> credit);
};

#endif