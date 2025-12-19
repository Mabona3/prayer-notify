#ifndef CONFIG_H
#define CONFIG_H

// return the config dir file
const char *get_config_file();

// return the config dir file
const char *get_temp_file();

// remove the config file cache from the memory.
void free_config_file();

// return the config dir file
void free_temp_file();

#endif // CONFIG_H
