cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

set(pathToSudokuLib /keep/Repo/USELESS/_sandbox/cpp/learning-cpp/sudoku-recognizer/src/lib)
set(pathToProject /keep/Repo/moulinet-sudoku-app)
set(pathToOpenCv /home/benng/Downloads/open-cv-3-1-0-android-sdk)
# from here: https://github.com/gcesarmza/curl-android-ios

set(pathToCurlPrebuilt /keep/Downloads/libcurl_OpenSSL1.0.1s/libs)

include_directories( ${pathToOpenCv}/sdk/native/jni/include /keep/Downloads/libcurl_OpenSSL1.0.1s/include )

find_package(ZLIB REQUIRED) #FOR CURL

add_library( lib_opencv SHARED IMPORTED )
add_library( lib_sudoku SHARED IMPORTED )
add_library( lib_curl STATIC IMPORTED )

set_target_properties(lib_curl PROPERTIES IMPORTED_LOCATION ${pathToCurlPrebuilt}/${ANDROID_ABI}/libcurl.a)

add_library( native-lib SHARED src/main/cpp/native-lib.cpp ${pathToSudokuLib}/sudoku.cpp )

set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${pathToProject}/app/src/main/jniLibs/${ANDROID_ABI}/libopencv_java3.so)

find_library( log-lib log )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
target_link_libraries( native-lib ${log-lib} lib_opencv android  ${ZLIB_LIBRARIES} lib_curl)
