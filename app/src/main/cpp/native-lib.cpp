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

extern "C"
jstring
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_getFileContent(
        JNIEnv* env,
        jobject javaThis,
        jobject pAssetManager)
{
    const char* filename = "file.txt";
    AAssetManager* assetManager = AAssetManager_fromJava(env, pAssetManager);

    /*
    All mode:
        - AASSET_MODE_UNKNOWN: Not known how the data is to be accessed
        - AASSET_MODE_RANDOM: Read chunks, and seek forward and backward
        - AASSET_MODE_STREAMING: Read sequentially, with an occasional
          forward seek
        - AASSET_MODE_BUFFER: Attempt to load contents into memory,
          for fast small reads
    */
    AAsset* file = AAssetManager_open(
            assetManager,
            filename,
            AASSET_MODE_UNKNOWN);

    if (file == NULL)
    {
        return env->NewStringUTF("ERROR: Can not open file...");
    }

    long size = AAsset_getLength(file);
    char* buffer = new char[size];
    AAsset_read (file, buffer, size);

    AAsset_close(file);

    jstring jstr = env->NewStringUTF(buffer);

    delete[] buffer;
    buffer = NULL;

    return jstr;
}