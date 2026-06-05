#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
  LOGLEVEL_DEBUG,
  LOGLEVEL_INFO,
  LOGLEVEL_WARN,
  LOGLEVEL_ERROR,
} LogLevel;

#define log_msg(level, ...) _log_msg(level, __FILE__, __LINE__, __VA_ARGS__)

void _log_msg(LogLevel level, const char *file, int line, const char *fmt, ...);
void set_log_level(LogLevel level);
void init_logger();

#endif  // !LOGGER_H
