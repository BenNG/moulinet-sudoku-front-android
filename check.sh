#! /bin/bash
DIRECTORY=app/src/main/jniLibs
if [ ! -d "$DIRECTORY" ]; then
    echo " please create app/src/main/jniLibs and put open-cv-3-1-0-android-sdk/sdk/native/libs inside"
    exit 1
fi
exit 0