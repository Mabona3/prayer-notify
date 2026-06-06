#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

LogLevel log_level;

const char *LogName[] = {[LOGLEVEL_DEBUG] = "DEBUG",
                         [LOGLEVEL_INFO]  = "INFO",
                         [LOGLEVEL_WARN]  = "WARN",
                         [LOGLEVEL_ERROR] = "ERROR"};

void init_logger() {
  log_level = LOGLEVEL_ERROR;
}

void _log_msg(LogLevel level, const char *file, int line, const char *fmt,
              ...) {
  if (log_level > level) {
    return;
  }

  FILE *stream = (level >= LOGLEVEL_WARN) ? stderr : stdout;

  va_list ap;
  va_start(ap, fmt);
  fprintf(stream, "[%s] %s:%d: ", LogName[level], file, line);
  vfprintf(stream, fmt, ap);
  fprintf(stream, "\n");
  va_end(ap);
}

inline void set_log_level(LogLevel level) {
  if ((log_level == LOGLEVEL_INFO || level == LOGLEVEL_INFO) &&
      log_level != level) {
    fprintf(stdout, "[INFO] Setting log level into %s\n", LogName[level]);
  } else if (level > LOGLEVEL_ERROR) {
    log_msg(LOGLEVEL_ERROR, "Unknown log level '%d'", level);
  }

  log_level = level;
}
