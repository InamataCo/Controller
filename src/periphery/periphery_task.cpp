#include "periphery_task.h"

#include "managers/services.h"

namespace bernd_box {
namespace periphery {

PeripheryTask::PeripheryTask(std::shared_ptr<Periphery> periphery)
    : Task(&Services::getScheduler()), periphery_(periphery), scheduler_(Services::getScheduler()) {}

  std::shared_ptr<Periphery> PeripheryTask::getPeriphery(){return periphery_;}

void PeripheryTask::OnDisable(){
  OnTaskDisable();
  scheduler_.deleteTask(*this);
  delete this;
}

void PeripheryTask::OnTaskDisable(){
  
}

}  // namespace periphery
}  // namespace bernd_box
