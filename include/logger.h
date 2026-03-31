#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

typedef enum {
  LOGLEVEL_DEBUG,
  LOGLEVEL_INFO,
  LOGLEVEL_WARN,
  LOGLEVEL_ERROR,
} LogLevel;

void log_msg(LogLevel level, const char *fmt, ...);
void set_log_level(LogLevel level);
void set_log_file(FILE *file);
void init_logger();
void set_log_filename(const char *file);

#endif  // !LOGGER_H
