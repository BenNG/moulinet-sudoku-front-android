## Moulinet sudoku app

- mettre la lib c en tant que submodule
- copy open-cv-3-1-0-android-sdk/sdk/native/libs in Android-app/app/src/main/jniLibs/

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
 

