#include "writer.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include "arena.h"
#include "config.h"
#include "logger.h"
#include "prayerTimes.h"

void close_current_writer(int fd)
{
    if (close(fd) != 0) {
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

// TODO: handle deleted files as well
int write_current(int fd, struct tm *times, TimeID current)
{
    assert(current <= TIMEID_TimesCount);
    if (current >= TIMEID_TimesCount) {
        log_msg(LOGLEVEL_ERROR, "invalid time id: %d", current);
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

    count += snprintf(buffer + count, buffer_size - count, "\"}");

    ftruncate(fd, count);
    lseek(fd, 0, SEEK_SET);

    if (write(fd, buffer, count) == -1) {
        log_msg(LOGLEVEL_ERROR, "Error writing to the writer file '%s'",
                strerror(errno));
        return EXIT_FAILURE;
    }

    fsync(fd);

    return EXIT_SUCCESS;
}

int check_temp_file(void)
{
    char *temp_file = NULL;
    ScratchArena scratch;
    arena_scratch_push(&scratch);
    if (get_temp_file(&scratch, &temp_file)) {
        log_msg(LOGLEVEL_ERROR, "Error retreiving the temp file");
        arena_scratch_pop(&scratch);
        return -1;
    }

    int fd = open(temp_file, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        log_msg(LOGLEVEL_ERROR, "Couldn't open file %s", temp_file);
        arena_scratch_pop(&scratch);
        return -1;
    }

    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        if (errno != EWOULDBLOCK) {
            perror("flock");
        }
        close(fd);
        return -1;
    }

    arena_scratch_pop(&scratch);
    return fd;
}
