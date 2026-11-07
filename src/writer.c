#include "writer.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "arena.h"
#include "config.h"
#include "logger.h"
#include "prayerTimes.h"

void close_current_writer(FILE *file)
{
    if (file == NULL || fclose(file) != 0) {
        log_msg(LOGLEVEL_ERROR, "Error closing the file: %s", strerror(errno));
    }

    char *temp_file = NULL;
    ScratchArena scratch;
    arena_scratch_push(&scratch);
    if (get_temp_file(&scratch, &temp_file) ||
        remove(temp_file) != EXIT_SUCCESS) {
        log_msg(LOGLEVEL_ERROR, "Error removing %s: '%s'", temp_file,
                strerror(errno));
        arena_scratch_pop(&scratch);
        return;
    }

    log_msg(LOGLEVEL_DEBUG, "file %s removed successfully", temp_file);
    arena_scratch_pop(&scratch);
    return;
}

int write_current(FILE *file, struct tm *times, TimeID current)
{
    assert(current <= TIMEID_TimesCount);
    if (current >= TIMEID_TimesCount) {
        log_msg(LOGLEVEL_ERROR, "invalid time id: %d", current);
        return -1;
    }

    assert(file != NULL);
    if (file == NULL) {
        log_msg(LOGLEVEL_ERROR, "invalid file argument");
        return -1;
    }

    log_msg(LOGLEVEL_DEBUG, "Writing current json file");

    int buffer_size = 160;
    char buffer[160];
    int count = snprintf(
        buffer, buffer_size, "{\"text\":\"%s %02d:%02d\",\"tooltip\":\"",
        TimeName[current], times[current].tm_hour, times[current].tm_min);
    for (int i = 0; i < TIMEID_TimesCount; i++) {
        count += snprintf(buffer + count, buffer_size - count, "%s %02d:%02d%s",
                          TimeName[i], times[i].tm_hour, times[i].tm_min,
                          i == TIMEID_Isha ? "" : "\\n");
    }

    snprintf(buffer + count, buffer_size - count, "\"}");

    ftruncate(fileno(file), count);

    rewind(file);

    if (fputs(buffer, file) == EOF) {
        log_msg(LOGLEVEL_ERROR, "Error writing to the writer file '%s'",
                strerror(errno));
        return EXIT_FAILURE;
    }

    if (fflush(file) == EOF) {
        log_msg(LOGLEVEL_ERROR, "Error flushing to the writer file '%s'",
                strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

FILE *check_temp_file(void)
{
    char *temp_file = NULL;
    ScratchArena scratch;
    arena_scratch_push(&scratch);
    if (get_temp_file(&scratch, &temp_file)) {
        log_msg(LOGLEVEL_ERROR, "Error retreiving the temp file");
        arena_scratch_pop(&scratch);
        return NULL;
    }

    struct stat file_stat;
    if (!stat(temp_file, &file_stat)) {
        log_msg(LOGLEVEL_DEBUG, "Temp file found another instance is running");
        arena_scratch_pop(&scratch);
        return NULL;
    }

    FILE *file = fopen(temp_file, "w");
    if (file == NULL) {
        log_msg(LOGLEVEL_ERROR, "Couldn't open file %s", temp_file);
        arena_scratch_pop(&scratch);
        return NULL;
    }

    arena_scratch_pop(&scratch);
    return file;
}
