cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")

set(pathToSudokuLib /keep/Repo/USELESS/_sandbox/cpp/learning-cpp/sudoku-recognizer/src/lib)
set(pathToProject /keep/Repo/sudoku-front-android)
set(pathToOpenCv /home/benng/Downloads/open-cv-3-1-0-android-sdk)

include_directories( ${pathToOpenCv}/sdk/native/jni/include )

add_library( lib_opencv SHARED IMPORTED )
add_library( lib_sudoku SHARED IMPORTED )

add_library( native-lib SHARED src/main/cpp/native-lib.cpp ${pathToSudokuLib}/sudoku.cpp )

set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${pathToProject}/app/src/main/jniLibs/${ANDROID_ABI}/libopencv_java3.so)

find_library( log-lib log )
target_link_libraries( native-lib ${log-lib} lib_opencv android)
