#include "athan.h"

#include <miniaudio/miniaudio.h>

#include "arena.h"
#include "config.h"

static void sound_end_callback(void *pUserData, ma_sound *pSound)
{
    ma_sound_seek_to_pcm_frame(pSound, 0);
    ((Athan_ctx *)pUserData)->status = ATHAN_STOPPED;
}

int init_athan(Athan_ctx *athan)
{
    ScratchArena scratch;
    ma_result result = MA_SUCCESS;

    athan->status = ATHAN_UNINIT;

    result = ma_engine_init(NULL, &athan->engine);
    if (result != MA_SUCCESS) {
        return -1;
    }

    char *output = NULL;

    arena_scratch_push(&scratch);
    if (get_audio_file(&scratch, &output) > 0) {
        ma_engine_uninit(&athan->engine);
        arena_scratch_pop(&scratch);
        return -1;
    }

    result =
        ma_sound_init_from_file(&athan->engine, output, MA_SOUND_FLAG_DECODE,
                                NULL, NULL, &athan->sound);

    if (result != MA_SUCCESS) {
        arena_scratch_pop(&scratch);
        return -1;
    }

    ma_sound_set_end_callback(&athan->sound, sound_end_callback, athan);

    athan->status = ATHAN_INIT;
    arena_scratch_pop(&scratch);
    return 0;
}

int start_athan(Athan_ctx *athan)
{
    if (athan->status == ATHAN_UNINIT) {
        return -1;
    }
    ma_result result = ma_sound_start(&athan->sound);
    if (result != MA_SUCCESS) {
        return -1;
    }
    athan->status = ATHAN_RUNNING;
    return 0;
}

int stop_athan(Athan_ctx *athan)
{
    if (athan->status == ATHAN_UNINIT) {
        return -1;
    }

    ma_result result = ma_sound_stop(&athan->sound);
    if (result != MA_SUCCESS) {
        return -1;
    }

    athan->status = ATHAN_STOPPED;
    return 0;
}

void uninit_athan(Athan_ctx *athan)
{
    if (ma_sound_is_playing(&athan->sound)) {
        ma_sound_stop(&athan->sound);
    }

    if (athan->status != ATHAN_UNINIT) {
        ma_sound_uninit(&athan->sound);
        ma_engine_uninit(&athan->engine);
    }

    athan->status = ATHAN_UNINIT;
}
