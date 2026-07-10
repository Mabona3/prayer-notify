#ifndef CONFIG_H
#define CONFIG_H

#include "arena.h"

// return the config file
int get_config_file(ScratchArena *scratch, char **config_file);

// return the icon from the config file.
int get_icon_file(ScratchArena *scratch, char **output);

// return the icon from the config file.
int get_audio_file(ScratchArena *scratch, char **output);

// return the config dir file
int get_temp_file(ScratchArena *scratch, char **temp_dir);

#endif  // CONFIG_H
