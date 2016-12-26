#include <jni.h>
#include <string>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>


extern "C"
jstring
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}



extern "C"
jobjectArray
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_getFilenameInAssets(
        JNIEnv* env,
        jobject javaThis,
        jobject pAssetManager)
{

    // Number of file
    const int maxNumFile = 20;
    int numFile = 0;

    // Asset manager
    const char* filename;
    AAssetManager* assetManager = AAssetManager_fromJava(env, pAssetManager);
    AAssetDir* assetFolder = AAssetManager_openDir(assetManager, "");

    // Put all file name into temporary array
    jobjectArray tempArr = (jobjectArray)env->NewObjectArray(
            maxNumFile,
            env->FindClass("java/lang/String"),
            env->NewStringUTF(""));

    while (
            ((filename = AAssetDir_getNextFileName(assetFolder)) != NULL) &&
            (numFile < maxNumFile)
            )
    {
        env->SetObjectArrayElement(
                tempArr,
                numFile,
                env->NewStringUTF(filename));
        numFile++;
    }

    // Copy file name from temporary array to returned array
    jobjectArray returnedArr = (jobjectArray)env->NewObjectArray(
            numFile,
            env->FindClass("java/lang/String"),
            env->NewStringUTF(""));
    for (int i = 0; i < numFile; ++i)
    {
        jstring jstr = (jstring)env->GetObjectArrayElement(tempArr, i);
        env->SetObjectArrayElement(returnedArr, i, jstr);
    }

    // Close assets folder
    AAssetDir_close(assetFolder);

    // Return
    return returnedArr;

}