#include <iostream>
#include "HCNetSDK.h"
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <ctime>
#include <time.h>
using namespace std;

/**
 * download record files to local  by start time and end time
 *  
 */
int main(int argc, char *argv[])
{

  //-----------------
  //get params
  char *host = argv[1];
  int port = atoi(argv[2]);
  char *user = argv[3];
  char *password = argv[4];
  DWORD channel = atoi(argv[5]);
  time_t start = atoll(argv[6]);
  time_t end = atoll(argv[7]);
  char *path = argv[8];

  // time process
  struct tm timePtr_start, timePtr_end, *ti;
  ti = localtime(&start);
  memcpy(&timePtr_start, ti, sizeof(*ti));
  ti = localtime(&end);
  memcpy(&timePtr_end, ti, sizeof(*ti));

  NET_DVR_Init();
  NET_DVR_SetConnectTime(2000, 1);
  NET_DVR_SetReconnect(10000, true);

  // ---------------------------------
  //login
  NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
  struLoginInfo.bUseAsynLogin = 0;
  strcpy(struLoginInfo.sDeviceAddress, host);
  struLoginInfo.wPort = port;
  strcpy(struLoginInfo.sUserName, user);
  strcpy(struLoginInfo.sPassword, password);
  NET_DVR_DEVICEINFO_V40 struDeviceInfoV40;
  LONG lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
  if (lUserID < 0)
  {
    printf("Login error, %d\n", NET_DVR_GetLastError());
    NET_DVR_Cleanup();
    return -1;
  }
  NET_DVR_DEVICEINFO_V30 struDeviceInfo = struDeviceInfoV40.struDeviceV30;
  printf("======= success get nvr info========");
  printf("UserID: %d\n", lUserID);
  printf("DevType: %d n", struDeviceInfo.wDevType);
  printf("IPcamNum: %d \n", struDeviceInfo.byIPChanNum);
  printf("StartDChan: %d \n", struDeviceInfo.byStartDChan);
  printf("HighDChanNum: %d \n", struDeviceInfo.byHighDChanNum);
  printf("SerialNumber: %s\n", struDeviceInfo.sSerialNumber);

  // we only proccess ip channel
  if (struDeviceInfo.byIPChanNum <= 0)
  {
    printf("NVR's IP has no IP chanNum");
    return -1;
  }

  // ---------------------------------
  //get nvr ip camera info
  DWORD dwReturned = 0;
  DWORD iGroupNO = 0;
  NET_DVR_IPPARACFG_V40 IPAccessCfgV40;
  memset(&IPAccessCfgV40, 0, sizeof(NET_DVR_IPPARACFG));
  if (!NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_IPPARACFG_V40, iGroupNO, &IPAccessCfgV40, sizeof(NET_DVR_IPPARACFG_V40), &dwReturned))
  {
    printf("NET_DVR_GET_IPPARACFG_V40 error, %d\n", NET_DVR_GetLastError());
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();
    return -1;
  }
  printf("toal group num: %d, DChanNum:%d, StartDChan:%d \n ", IPAccessCfgV40.dwGroupNum, IPAccessCfgV40.dwDChanNum, IPAccessCfgV40.dwStartDChan);

  for (int i = 0; i < IPAccessCfgV40.dwDChanNum; i++)
  {
    int ch = IPAccessCfgV40.dwStartDChan + i;
    printf("channel: %d,IP: %s, enable: %d \n", ch, IPAccessCfgV40.struIPDevInfo[i].struIP.sIpV4, IPAccessCfgV40.struIPDevInfo[i].byEnable);
  }

  //---------------------------------------
  //Save record by Time
  NET_DVR_PLAYCOND struDownloadCond = {0};
  struDownloadCond.dwChannel = channel;
  struDownloadCond.struStartTime.dwYear = timePtr_start.tm_year + 1900;
  struDownloadCond.struStartTime.dwMonth = timePtr_start.tm_mon + 1;
  struDownloadCond.struStartTime.dwDay = timePtr_start.tm_mday;
  struDownloadCond.struStartTime.dwHour = timePtr_start.tm_hour;
  struDownloadCond.struStartTime.dwMinute = timePtr_start.tm_min;
  struDownloadCond.struStartTime.dwSecond = timePtr_start.tm_sec;
  struDownloadCond.struStopTime.dwYear = timePtr_end.tm_year + 1900;
  struDownloadCond.struStopTime.dwMonth = timePtr_end.tm_mon + 1;
  struDownloadCond.struStopTime.dwDay = timePtr_end.tm_mday;
  struDownloadCond.struStopTime.dwHour = timePtr_end.tm_hour;
  struDownloadCond.struStopTime.dwMinute = timePtr_end.tm_min;
  struDownloadCond.struStopTime.dwSecond = timePtr_end.tm_sec;

  printf("start to save file at channel:%d at start_time: %d-%d-%d %d:%d:%d, end_time: %d-%d-%d %d:%d:%d \n", channel,
         struDownloadCond.struStartTime.dwYear, struDownloadCond.struStartTime.dwMonth, struDownloadCond.struStartTime.dwDay, struDownloadCond.struStartTime.dwHour, struDownloadCond.struStartTime.dwMinute, struDownloadCond.struStartTime.dwSecond,
         struDownloadCond.struStopTime.dwYear, struDownloadCond.struStopTime.dwMonth, struDownloadCond.struStopTime.dwDay, struDownloadCond.struStopTime.dwHour, struDownloadCond.struStopTime.dwMinute, struDownloadCond.struStopTime.dwSecond);
  int hPlayback;
  hPlayback = NET_DVR_GetFileByTime_V40(lUserID, path, &struDownloadCond);
  if (hPlayback < 0)
  {
    printf("NET_DVR_GetFileByTime_V40 fail,last error %d\n", NET_DVR_GetLastError());
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();
    return -1;
  }

  //---------------------------------------
  //start downloading
  if (TRUE != NET_DVR_PlayBackControl_V40(hPlayback, NET_DVR_PLAYSTART, NULL, 0, NULL, NULL))
  {
    printf("Play back control failed [%d]\n", NET_DVR_GetLastError());
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();
    return -1;
  }

  int nPos = 0;
  for (nPos = 0; nPos < 100 && nPos >= 0; nPos = NET_DVR_GetDownloadPos(hPlayback))
  {
    printf("Downloading ... %d %%\n", nPos);
    usleep(1000000); //micro seconds
  }
  if (TRUE != NET_DVR_StopGetFile(hPlayback))
  {
    printf("failed to stop get file [%d]\n", NET_DVR_GetLastError());
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();
    return -1;
  }

  printf("finished save file to  %s\n", path);
  NET_DVR_Logout(lUserID);
  NET_DVR_Cleanup();
  return 0;
}
