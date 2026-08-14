#pragma once
#include <cstdint>
#include <string>

int S9xSoftwareFilterCount();
const char *S9xSoftwareFilterName(int filter);
int S9xSoftwareFilterFromName(const std::string &name);
void S9xSoftwareFilterScale(int filter, int &width, int &height);
void S9xApplySoftwareFilter(int filter,
                            uint8_t *src, int src_pitch,
                            uint8_t *dst, int dst_pitch,
                            int width, int height);
