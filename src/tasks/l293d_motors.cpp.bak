#include "l293d_motors.h"

namespace bernd_box {
namespace tasks {

L293dMotors::L293dMotors(Scheduler* scheduler, Io& io, Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt) {
  setIterations(TASK_FOREVER);
  setInterval(TASK_SECOND);  // All logic is in OnEnable and OnDisable

  mqtt_.addAction(name_,
                  std::bind(&L293dMotors::mqttCallback, this, _1, _2, _3));
}
L293dMotors::~L293dMotors() {}

bool L293dMotors::OnEnable() {
  std::chrono::milliseconds next_timeout = std::chrono::milliseconds::max();
  // Enable all runs that have not been enabled and find the time point of the
  // next soonest timeout
  for (auto& run : runs_) {
    if (!run.enabled) {
      enableRun(run);
    }
    if (next_timeout > run.duration + run.start_time) {
      next_timeout = run.duration + run.start_time;
    }
  }

  // Calculate the duration of the shortest timeout and set it.
  setTimeout((next_timeout - std::chrono::milliseconds(millis())).count());

  return true;
}

bool L293dMotors::Callback() { return true; }

void L293dMotors::OnDisable() {
  // If we are disabled by a timeout, other runs might still need to continue.
  // If not, disable() was called and all runs should be stopped
  bool disable_all = !timedOut();

  if (disable_all) {
    // Disable all runs and then remove them from the run list
    for (auto& run : runs_) {
      if (run.enabled) {
        disableRun(run, true);
      }
    }
    runs_.clear();

  } else {
    // Find the runs that should be disabled, disable and remove them. If any
    // runs remain, reenable the task
    const std::chrono::milliseconds now(millis());
    bool remaining_runs = false;

    // Find runs that can be disabled, call enable() if there are remaining
    // runs, but only after removing finished runs
    for (auto& run : runs_) {
      if (run.enabled) {
        // Disable if the time now has passed the end time (start + duration)
        if (now >= run.start_time + run.duration) {
          disableRun(run, false);
        } else {
          remaining_runs = true;
        }
      }
    }

    // Clear all disabled runs
    runs_.erase(std::remove_if(runs_.begin(), runs_.end(),
                               [](const Run& run) { return !run.enabled; }),
                runs_.end());

    // If there are any remaining runs still active, re-enable the task
    if (remaining_runs) {
      enable();
    }
  }
}

void L293dMotors::mqttCallback(char* topic, uint8_t* payload,
                               unsigned int length) {
  const char* who = __PRETTY_FUNCTION__;

  // Try to deserialize the message
  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  const DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    mqtt_.sendError(who, String(F("Deserialize failed: ")) + error.c_str());
    return;
  }

  // Handle all known actions
  bool action_found = false;
  JsonVariantConst add_doc = doc[F("add")];
  if (!add_doc.isNull()) {
    addDevice(add_doc);
    action_found = true;
  }
  JsonVariantConst remove_doc = doc[F("remove")];
  if (!remove_doc.isNull()) {
    removeDevice(remove_doc);
    action_found = true;
  }
  JsonVariantConst enable_doc = doc[F("enable")];
  if (!enable_doc.isNull()) {
    enableDevice(enable_doc);
    action_found = true;
  }
  JsonVariantConst disable_doc = doc[F("disable")];
  if (!disable_doc.isNull()) {
    disableDevice(disable_doc);
    action_found = true;
  }

  // If no known action is included, send an error
  if (!action_found) {
    mqtt_.sendError(who, F("No known action found [add, remove, enable, disable]"));
  }
}

Result L293dMotors::addRun(int id, std::chrono::milliseconds duration,
                           uint8_t power_percent, bool forward,
                           bool call_enable) {
  const char* who = __PRETTY_FUNCTION__;

  // Check if an existing run is already using the motor
  for (const auto& run : runs_) {
    if (run.l293d_motor_id == id) {
      mqtt_.sendError(who, String(F("Another run is already using: ")) + io_.getName(id));
      return Result::kFailure;
    }
  }

  runs_.emplace_back(Run{.l293d_motor_id = id,
                         .enabled = false,
                         .start_time = std::chrono::milliseconds::zero(),
                         .duration = duration,
                         .power_percent = power_percent,
                         .forward = forward});
  if (call_enable) {
    enable();
  }
  return Result::kSuccess;
}

void L293dMotors::enableRun(Run& run) {
  // Get the respective IO definition and enable writing to the control pins
  const auto& pins = io_.l293d_motors_[run.l293d_motor_id];
  io_.enableOutput(pins.pin_forward);
  io_.enableOutput(pins.pin_reverse);
  io_.enableOutput(pins.pin_enable);

  // Enable the motor. Set the direction, then PWM enable for the speed
  if (run.forward) {
    io_.setPinState(pins.pin_forward, true);
    io_.setPinState(pins.pin_reverse, false);
  } else {
    io_.setPinState(pins.pin_forward, false);
    io_.setPinState(pins.pin_reverse, true);
  }
  io_.setPinPwm(pins.pin_enable, run.power_percent);

  // Mark the run as started
  run.enabled = true;
  run.start_time = std::chrono::milliseconds(millis());
}

Result L293dMotors::disableRun(Run& run, bool abort) {
  const char* who = __PRETTY_FUNCTION__;

  // Detatch the enable pin from the PWM signal to disable the motor
  io_.removePinPwm(io_.l293d_motors_[run.l293d_motor_id].pin_enable);
  run.enabled = false;

  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  JsonObject result = doc.createNestedObject(F("result"));
  std::vector<const char*> result_keys;
  result_keys.reserve(7);  // Approx. # of keys

  const char* name_key = "name";
  result_keys.push_back(name_key);
  result[name_key] = io_.getName(run.l293d_motor_id);

  // All MQTT times are sent in UTC time. Calculate the real run duration with
  // (millis() - run.start_time) with the uptime epoch (time since boot) and
  // convert to UTC time as time() returns seconds in Unix epoch (since 1970).
  std::chrono::seconds actual_duration =
      std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::milliseconds(millis()) - run.start_time);
  const std::chrono::seconds start_time_utc =
      std::chrono::seconds(time(nullptr)) - actual_duration;
  const char* start_time_key = "start_time";
  result_keys.push_back(start_time_key);
  result[start_time_key] = start_time_utc.count();

  const char* planned_duration_s_key = "planned_duration_s";
  result_keys.push_back(planned_duration_s_key);
  result[planned_duration_s_key] =
      std::chrono::duration_cast<std::chrono::seconds>(run.duration).count();

  const char* actual_duration_s_key = "actual_duration_s";
  result_keys.push_back(actual_duration_s_key);
  result[actual_duration_s_key] = actual_duration.count();

  const char* forward_key = "forward";
  result_keys.push_back(forward_key);
  result[forward_key] = run.forward;

  const char* power_percent_key = "power_percent";
  result_keys.push_back(power_percent_key);
  result[power_percent_key] = run.power_percent;

  const char* abort_key = "abort";
  result_keys.push_back(abort_key);
  result[abort_key] = abort;

  // Check if all required properties have been defined
  for (const auto& result_key : result_keys) {
    if (!result.containsKey(result_key)) {
      mqtt_.sendError(who, String(F("Failed serializing: ")) + result_key);
      return Result::kFailure;
    }
  }

  mqtt_.send(name_, doc);
  return Result::kSuccess;
}

Result L293dMotors::addDevice(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  // Try creating a new l239d motor device
  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, "Missing property: name (string)");
    return Result::kFailure;
  }

  JsonVariantConst pin_forward = doc[F("pin_forward")];
  if (!pin_forward.is<int>()) {
    mqtt_.sendError(who, F("Missing property: pin_forward (int)"));
    return Result::kFailure;
  }
  if (pin_forward < 0) {
    mqtt_.sendError(who, F("pin_forward has to be >= 0"));
    return Result::kFailure;
  }

  JsonVariantConst pin_reverse = doc[F("pin_reverse")];
  if (!pin_reverse.is<int>()) {
    mqtt_.sendError(who, F("Missing property: pin_reverse (int)"));
    return Result::kFailure;
  }
  if (pin_reverse < 0) {
    mqtt_.sendError(who, F("pin_reverse has to be >= 0"));
    return Result::kFailure;
  }

  JsonVariantConst pin_enable = doc[F("pin_enable")];
  if (!pin_enable.is<int>()) {
    mqtt_.sendError(who, F("Missing property: pin_enable (int)"));
    return Result::kFailure;
  }
  if (pin_enable < 0) {
    mqtt_.sendError(who, F("pin_enable has to be >= 0"));
    return Result::kFailure;
  }

  int id;
  Result result = io_.addDevice(name, id);
  if (result != Result::kSuccess) {
    mqtt_.sendError(who, String(F("Name already exists: ")) + name.as<char*>());
    return Result::kFailure;
  }

  io_.l293d_motors_[id] = L293dMotor{.pin_forward = pin_forward,
                                     .pin_reverse = pin_reverse,
                                     .pin_enable = pin_enable};

  return Result::kSuccess;
}

Result L293dMotors::removeDevice(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, "Missing property: name (string)");
    return Result::kFailure;
  }

  // If there are active runs, disable them
  const int id = io_.getId(name);
  const auto run =
      std::find_if(runs_.begin(), runs_.end(),
                   [id](const Run& run) { return run.l293d_motor_id == id; });
  if (run != runs_.end()) {
    disableRun(*run, true);
  }

  // Remove the device
  io_.removeDevice(id);
  return Result::kSuccess;
}

Result L293dMotors::enableDevice(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  // Check if the name matches a registered l293D motor
  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, F("Missing property: name (string)"));
    return Result::kFailure;
  }
  const int id = io_.getId(name.as<char*>());
  if (id < 0 || io_.l293d_motors_.find(id) == io_.l293d_motors_.end()) {
    mqtt_.sendError(who, F("L293D motor name not found"));
    return Result::kFailure;
  }

  // Check that the duration in seconds is greater than 0
  JsonVariantConst duration_s = doc[F("duration_s")];
  if (!duration_s.is<int>()) {
    mqtt_.sendError(who, F("Missing property: duration_s (int)"));
    return Result::kFailure;
  }
  const std::chrono::seconds duration(duration_s);
  if (duration <= std::chrono::seconds::zero()) {
    mqtt_.sendError(
        who, String(F("Duration has to be > 0: ")) + duration_s.as<int>());
    return Result::kFailure;
  }

  // Check that the power percent is between 0 - 100
  JsonVariantConst power_percent = doc[F("power_percent")];
  if (!power_percent.is<int>()) {
    mqtt_.sendError(who, F("Missing property: power_percent (int)"));
    return Result::kFailure;
  }
  if (power_percent < 0 || power_percent > 100) {
    mqtt_.sendError(who,
                    String(F("Power percent has to be between 0 - 100: ")) +
                        String(power_percent.as<int>()));
    return Result::kFailure;
  }

  // Check the direction of the motor
  JsonVariantConst forward = doc[F("forward")];
  if (!forward.is<bool>()) {
    mqtt_.sendError(who, F("Missing property: power_percent (bool)"));
    return Result::kFailure;
  }

  addRun(id, duration, power_percent, forward);

  return Result::kSuccess;
}

Result L293dMotors::disableDevice(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  // Check if the property exists
  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, "Missing property: name (string)");
    return Result::kFailure;
  }

  // Find the device ID for a given name --> find the corresponding run -->
  // disable it and remove the run
  const int id = io_.getId(name);
  auto run = std::find_if(runs_.begin(), runs_.end(), [id](const Run& run) {
    return run.l293d_motor_id == id;
  });
  if (run != runs_.end()) {
    disableRun(*run, true);
    runs_.erase(run);
  } else {
    mqtt_.sendError(who, "Run not found");
    return Result::kFailure;
  }

  return Result::kSuccess;
}

}  // namespace tasks
}  // namespace bernd_box
