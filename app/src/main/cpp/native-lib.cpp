#include <jni.h>
#include <string>

extern "C"
jstring
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
