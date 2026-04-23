#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
  LOGLEVEL_DEBUG,
  LOGLEVEL_INFO,
  LOGLEVEL_WARN,
  LOGLEVEL_ERROR,
} LogLevel;

#define log_msg(level, fmt, ...) \
  _log_msg(level, fmt, __FILE__, __LINE__, ##__VA_ARGS__)

void _log_msg(LogLevel level, const char *fmt, const char *file, int line, ...);
void set_log_level(LogLevel level);
void init_logger();

#endif  // !LOGGER_H
