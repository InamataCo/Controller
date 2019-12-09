#ifndef BERND_BOX_TASK_MEASUREMENT_PROTOCOL_H
#define BERND_BOX_TASK_MEASUREMENT_PROTOCOL_H

#include "Arduino.h"

#include "chrono"
#include "vector"
#include "map"

#include "acidity_sensor.h"
#include "dallas_temperature.h"
#include "dissolved_oxygen_sensor.h"
#include "mqtt.h"
#include "pump.h"
#include "task.h"

namespace bernd_box {
namespace tasks {

enum class Action {
  kPump,
  kWaterTemperature,
  kDissolvedOxygen,
  kTotalDissolvedSolids,
  kAcidity,
  kTurbidity,
  kSleep,
};

const std::map<Action, const char*> actionName = {
  {Action::kPump, "pump"},
  {Action::kWaterTemperature, "measure water temperature"},
  {Action::kDissolvedOxygen, "measure dissolved oxygen"},
  {Action::kTotalDissolvedSolids, "measure total dissolved solids"},
  {Action::kAcidity, "measure acidity"},
  {Action::kTurbidity, "measure turbidity"},
  {Action::kSleep, "sleep"},
};

struct ReportItem {
  /// Required item to select which tasks to enable
  Action action;

  /// Optional and used for some report items
  std::chrono::milliseconds duration;
};

class MeasurementProtocol : public Task {
 public:
  const std::chrono::milliseconds default_period_{1000};

  MeasurementProtocol(Scheduler* scheduler, Mqtt& mqtt, Io& io,
                      std::vector<bernd_box::tasks::ReportItem>& report_list,
                      Pump& pump_task,
                      DallasTemperature& dallas_temperature_task,
                      DissolvedOxygenSensor& dissolved_oxygen_sensor_taks,
                      AciditySensor& acidity_sensor_task);
  virtual ~MeasurementProtocol();

 private:
  bool OnEnable();

  /**
   * Processes the report items one by one
   *
   * Each report item has to define an enable and finish section in its
   * switch-case. The enable code is called when isReportItemEnabled is false.
   * A subsequent if checks if the task is still active. This allows the task to
   * end itself depending on required measurements or a timeout.
   */
  bool Callback();

  void OnDisable();

  const char* getCurrentActionName();

  Mqtt& mqtt_;
  Io& io_;

  std::vector<bernd_box::tasks::ReportItem>& report_list_;
  uint report_index_ = 0;
  bool is_report_item_enabled;
  std::chrono::milliseconds start_time_;

  // Known tasks
  Pump& pump_task_;
  DallasTemperature& dallas_temperature_task_;
  DissolvedOxygenSensor& dissolved_oxygen_sensor_task_;
  AciditySensor& acidity_sensor_task_;

  std::chrono::milliseconds sleep_start_time_;
};

}  // namespace tasks
}  // namespace bernd_box

#endif  // BERND_BOX_TASK_MEASUREMENT_PROTOCOL_H