#pragma once

#include <Arduino.h>

#ifdef ENABLE_TRACE
#define TRACEF(format, ...)                     \
  Serial.printf("%s(%d) ", __FILE__, __LINE__); \
  Serial.printf(String(F(format)).c_str(), __VA_ARGS__)
#define TRACELN(msg, ...)                                         \
  Serial.printf("%s::%s(%d) ", __FILE__, __FUNCTION__, __LINE__); \
  Serial.println(msg)
#define TRACESTACK                                       \
  Serial.printf("%s(%d) stack %u\n", __FILE__, __LINE__, \
                ESP.getFreeContStack());
#else
#define TRACEF(format, ...) (void)sizeof(__VA_ARGS__)
#define TRACELN(format, ...)
#define TRACESTACK
#endif
