#ifndef ATHAN_H
#define ATHAN_H

#include <miniaudio/miniaudio.h>

typedef enum {
    ATHAN_UNINIT,
    ATHAN_INIT,
    ATHAN_RUNNING,
    ATHAN_STOPPED,
} Athan_Status;

typedef struct {
    ma_engine engine;
    ma_sound sound;
    Athan_Status status;
} Athan_ctx;

int init_athan(Athan_ctx *athan);

int start_athan(Athan_ctx *athan);

int stop_athan(Athan_ctx *athan);

void uninit_athan(Athan_ctx *athan);

#endif  // ATHAN_H
