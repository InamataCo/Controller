#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/service_getters.h"
#include "peripheral/capabilities/start_measurement.h"
#include "tasks/get_values_task/get_values_task.h"

namespace inamata {
namespace tasks {
namespace read_sensor {

/**
 * Read a single value from a sensor and return it to the server.
 *
 * Uses the GetValues capability to read values and supports the
 * StartMeasurement capability to perform measurement processes.
 */
class ReadSensor : public get_values_task::GetValuesTask {
 public:
  /**
   * Creates a peripheral to read peripheral values via the GetValues
   * capability.
   *
   * All required parameters are covered by the parent GetValuesTask(). If the
   * peripheral supports the StartMeasurement capability, it starts a
   * measurement.
   *
   * \param parameters The JSON parameters to construct the task
   * \param scheduler The scheduler to which the task is assigned to
   */
  ReadSensor(const ServiceGetters& services, const JsonObjectConst& parameters,
             Scheduler& scheduler);
  virtual ~ReadSensor() = default;

  const String& getType() const final;
  static const String& type();

  /**
   * Handles the measurement process and then reads the values.
   *
   * The StartMeasurement capability is handled according to the returned wait
   * time, if supported. Once the measurement is ready, read the values via the
   * GetValues capability and then send them to the server.
   *
   * \return True if no error has occured
   */
  bool TaskCallback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const ServiceGetters& services,
                           const JsonObjectConst& parameters,
                           Scheduler& scheduler);
  std::shared_ptr<peripheral::capabilities::StartMeasurement>
      start_measurement_peripheral_ = nullptr;

  std::shared_ptr<WebSocket> web_socket_;
};

}  // namespace read_sensor
}  // namespace tasks
}  // namespace inamata
