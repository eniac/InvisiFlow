#ifndef MY_CONFIG_H
#define MY_CONFIG_H

#define DEFAULT_PORT 80

double start_time = 2;
double duration = 0.5;

double measureStart = 2.1;
double measureEnd = 2.4;

double thd = 0.4;
uint32_t utilGap = 10000;
uint16_t intSize = 0;
uint32_t OrbWeaver = 0;

uint32_t taskId = 2;

uint32_t topology = 1;

uint32_t ecmpConfig = 0;
uint32_t failConfig = 0;
uint32_t recordConfig = 0;

uint32_t tempConfig = 0;
uint32_t storeConfig = 0;

uint32_t teleThd = 172000;
uint32_t priority = 86000;

int fct_record = 0;
int hG = 0;

int64_t generateBps = 128 * 1024 * 1024L;

std::string file_name;

#endif