#include "peripheryTask.h"

namespace bernd_box {
namespace periphery {

PeripheryTask::PeripheryTask(std::shared_ptr<Periphery> periphery)
    : periphery_(periphery) {}

  std::shared_ptr<Periphery> PeripheryTask::getPeriphery(){return periphery_;}

}  // namespace periphery
}  // namespace bernd_box