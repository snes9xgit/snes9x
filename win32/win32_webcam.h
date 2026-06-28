#ifndef WIN32_WEBCAM_H
#define WIN32_WEBCAM_H

#include <vector>
#include <string>

void GBCameraRegister();
void GBCameraEnumerate(std::vector<std::wstring> &names);
bool GBCameraStart(int deviceIndex);
void GBCameraStop();
bool GBCameraIsRunning();

#endif
