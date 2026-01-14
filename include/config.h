#ifndef CONFIG_H
#define CONFIG_H

// return the config file
int get_config_file(char **config_file);

// return the icon from the config file.
int get_icon_file(char **output);

// return the icon from the config file.
int get_audio_file(char **output);

// return the config dir file
int get_temp_file(char **temp_dir);

#endif  // CONFIG_H
