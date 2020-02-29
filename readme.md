# Save Hikvision Record Files

Save record files by start time and end time from hikvision nvr.

## How to compile

```bash
# workspace
mkdir -p /home/nvr/lib
cd /home/nvr

# download sdk
wget http://download.hikvision.com/UploadFile/SDK/CH-HCNetSDKV6.0.2.2_build20181213_Linux64.zip
unzip CH-HCNetSDKV6.0.2.2_build20181213_Linux64.zip

#copy the so lib we need
cp CH-HCNetSDKV6.0.2.2_build20181213_Linux64/lib/lib*  lib/
cp -r CH-HCNetSDKV6.0.2.2_build20181213_Linux64/lib/HCNetSDKCom/   lib/

# modify lb_library_path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/nvr/lib:/home/nvr/lib/HCNetSDKCom
source ~/.bashrc

# compile
 g++ -o yohonvr yohonvr.cpp  -lhcnetsdk -lHCCore -lhpr -lHCCoreDevCfg  -L ./lib -L ./lib/HCNetSDKCom
```

## How to Use

```bash
# example: ./yohonvr 127.0.0.1 admin 123456  33 1553045839  1553045939  /tmp/save.mp4
./yohonvr [IP] [PORT] [USER] [PASSWORD]  [CHANNELNO.]   [START_TIMESTAMP]  [END_TIMESTAMP]  [SAVE_PATH]
```

>
> Please notice that hikvision  IPCamera's start channal number is 33.
>


## Encode

### Using `ffmpeg`

```bash
# using ffmpeg to encode as mp4. 
# see as: https://trac.ffmpeg.org/wiki/Encode/H.264
# remove the audio by [-an]
# crf: The range of the CRF scale is 0–51, where 0 is lossless, 23 is the default, and 51 is worst quality possible. A lower value generally leads to higher quality, and a subjectively sane range is 17–28. Consider 17 or 18 to be visually lossless or nearly so; it should look the same or nearly the same as the input but it isn't technically lossless. 
# A preset is a collection of options that will provide a certain encoding speed to compression ratio. A slower preset will provide better compression (compression is quality per filesize).

# example: 
ffmpeg -i test.mp4  -preset fast -crf 19 -an -codec:v libx264 -maxrate 4M -bufsize 8M encoded_test.mp4

```

 
 author: andybrier@qq.com
