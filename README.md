## sudoku-front-android

- mettre la lib c en tant que submodule
- if you have this error: 
```
Error:error: '/keep/Repo/sudoku-front-android/app/src/main/jniLibs/armeabi/libopencv_java3.so', needed by '../../../../build/intermediates/cmake/debug/obj/armeabi/libnative-lib.so', missing and no known rule to make it
```
then copy open-cv-3-1-0-android-sdk/sdk/native/libs in sudoku-front-android/app/src/main/jniLibs/

## Tips
- DO NOT forget to add internet permission to you app in the manifest
```
<uses-permission android:name="android.permission.INTERNET" />
```
- networking and ndk:
  - Volley (java)
  - Retrofit (java)
  - https://github.com/xxDroid/libmicrohttpd-android
  - https://curl.haxx.se/docs/install.html
- curl for android
 - this project use the prebuild libcurl from [here](https://rubentorresbonet.wordpress.com/2016/05/19/curl-for-android-with-openssl-1-0-1s/) and not from [here](https://github.com/gcesarmza/curl-android-ios) I save it in my google drive just in case [here](https://drive.google.com/drive/u/0/folders/0B4M1HXbdsyQnalRiRmcyUUFtUDg)

16/1/2017 bad day ! when commiting on sudoku-recognizer test has to run if not it's not going to work on android for sure
create the script check.sh in order to not loose the entire afternoon
this script is usefull when invoking just before launching the app in android studio run -> edit config -> external tool

- when importing a module (opencv for java) seems like android studio do NOT like symbolic link so:
 - copy opencv-3.1-android-sdk somewhere
 - in android studio: import module -> opencv-3.1-android-sdk.sdk/java
 - change the build.gradle to match the app/build.gradle
 - in app go to open module settings and and the opencv lib as a dependency to your app