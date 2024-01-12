/**
 * @brief: 	config_internal.h - Internal definitions for the configuration datastore
 * 
 * @note:   Only to be included by the config module. If you are including from 
 *          another module then you are doing something wrong OR we need to enhance
 *          the design. The goal is to keep the internal helper functions private to the 
 *          config module. 
 * 
 *           Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */
#ifndef CONFIG_INTERN_H_
#define CONFIG_INTERN_H_

void config_fs_init(void);
bool is_file_exists(char *file_name);
int save_to_file(char *content, size_t content_len, char *file_name);
int read_file(char *output_buf, int output_buf_len, char *file_name);

#endif  // CONFIG_INTERN_H_
