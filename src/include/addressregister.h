#ifndef EMU_ADDRESSREGISTER_H
#define EMU_ADDRESSREGISTER_H

#include <string>
#include <utility>
#include "../include/systembus.h"

class AddressRegister : public ConnectedComponent {
private:
  word         value = 0;

public:
                 AddressRegister(int, std::string);
  void           setValue(word val);
  int            getValue() const override { return value; }
  std::ostream & status(std::ostream &) override;
  SystemError    reset() override;
  SystemError    onRisingClockEdge() override;
  SystemError    onHighClock() override;
};

#endif //EMU_ADDRESSREGISTER_H
