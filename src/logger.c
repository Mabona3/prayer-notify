#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

typedef struct {
  LogLevel log_level;
  FILE *file;
} Log;

Log logger;

const char *LogName[] = {[LOGLEVEL_DEBUG]  = "DEBUG",
                         [LOGLEVEL_INFO]  = "INFO",
                         [LOGLEVEL_WARN]  = "WARN",
                         [LOGLEVEL_ERROR] = "ERROR"};

void init_logger() {
  logger = (Log){
      .log_level = LOGLEVEL_ERROR,
      .file      = stdout,
  };
}

void log_msg(LogLevel level, const char *fmt, ...) {
  if (logger.log_level > level) {
    return;
  }

  va_list ap;
  va_start(ap, fmt);
  fprintf(logger.file, "[%s]: ", LogName[level]);
  vfprintf(logger.file, fmt, ap);
  va_end(ap);
}

void set_log_level(LogLevel level) {
  if ((logger.log_level == LOGLEVEL_INFO || level == LOGLEVEL_INFO) &&
      logger.log_level != level) {
    fprintf(logger.file, "[INFO] Setting log level into %s\n", LogName[level]);
  } else if (level > LOGLEVEL_ERROR) {
    log_msg(LOGLEVEL_ERROR, "Unknown log level '%d'\n", level);
  }

  logger.log_level = level;
}

void set_log_file(FILE *file) {
  logger.file = file;
}

void set_log_filename(const char *file) {
  FILE *log_file = fopen(file, "w");
  logger.file    = log_file;
}
