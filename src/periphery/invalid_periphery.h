#ifndef BERND_BOX_PERIPHERY_INVALID_PERIPHERY_H
#define BERND_BOX_PERIPHERY_INVALID_PERIPHERY_H

#include "abstractPeriphery.h"
#include "peripheryFactory.h"

namespace bernd_box {
namespace periphery {

class InvalidPeriphery : public AbstractPeriphery {
 public:
  virtual ~InvalidPeriphery() = default;

  const __FlashStringHelper* getType() final;
  static const __FlashStringHelper* type();

 private:
  static std::shared_ptr<Periphery> factory(const JsonObjectConst&);
  
  static bool registered_;
};

}  // namespace periphery
}  // namespace bernd_box

#endif
