#ifndef SRC_COMMON_HW_INCLUDE_FATFS_H_
#define SRC_COMMON_HW_INCLUDE_FATFS_H_

#include "hw_def.h"


#ifdef _USE_HW_FATFS


bool fatfsInit(void);
bool fatfsMount(void);
bool fatfsUnMount(void);
bool fatfsIsMount(void);
bool fatfsExist(const char *path);
bool fatfsCreateDir(const char *path);
bool fatfsDelete(const char *path);
bool fatfsWrite(const char *path, uint8_t *p_data, uint32_t length);
bool fatfsRead(const char *path, uint8_t *p_data, uint32_t length, uint32_t *p_read_length);
bool fatfsAppend(const char *path, uint8_t *p_data, uint32_t length);


#endif

#endif 
