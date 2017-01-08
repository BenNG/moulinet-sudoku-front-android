#include <jni.h>
#include <string>
#include <android/asset_manager_jni.h>
#include <opencv2/core.hpp>
#include <sstream>
#include <android/asset_manager.h>
#include "../../../../sudoku-recognizer/src/lib/sudoku.h"

#include <curl/curl.h>

using namespace cv;

extern "C" jstring
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_grab(
    JNIEnv *env,
    jobject javaThis,
    jstring fileName,
    jobject pAssetManager)
{

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");
    }

    std::string hello = "Hello from C++";

    const char *nativeFileName = env->GetStringUTFChars(fileName, JNI_FALSE);
    stringstream ss, fileName_ss;

    const char *filename = "raw-features.yml";
    fileName_ss << "puzzles/";
    fileName_ss << nativeFileName;
    fileName_ss << ".jpg";
    AAssetManager *assetManager = AAssetManager_fromJava(env, pAssetManager);

    /*
    All mode:
        - AASSET_MODE_UNKNOWN: Not known how the data is to be accessed
        - AASSET_MODE_RANDOM: Read chunks, and seek forward and backward
        - AASSET_MODE_STREAMING: Read sequentially, with an occasional
          forward seek
        - AASSET_MODE_BUFFER: Attempt to load contents into memory,
          for fast small reads
    */
    AAsset *file = AAssetManager_open(
        assetManager,
        filename,
        AASSET_MODE_UNKNOWN);

    AAsset *s0 = AAssetManager_open(
        assetManager,
        fileName_ss.str().c_str(),
        AASSET_MODE_UNKNOWN);

    if (file == NULL)
    {
        return env->NewStringUTF("ERROR: Can not open file...");
    }
    if (s0 == NULL)
    {
        return env->NewStringUTF(fileName_ss.str().c_str());
    }

    long size = AAsset_getLength(file);
    char *buffer = new char[size];
    AAsset_read(file, buffer, size);

    int size_s0 = AAsset_getLength(s0);
    char *buffer_s0 = new char[size_s0];
    AAsset_read(s0, buffer_s0, size_s0);

    Mat rawData = Mat(1, size_s0, CV_8UC1, buffer_s0);

    Mat decodedImage = imdecode(rawData, CV_8UC1);
    if (decodedImage.data == NULL)
    {
        return env->NewStringUTF("jpg decoding error");
        // Error reading raw image data
    }

    AAsset_close(file);
    AAsset_close(s0);

    ss << size_s0;

    FileStorage fs(buffer, FileStorage::READ | FileStorage::MEMORY);

    // Mat rawData  =  Mat( 1, size_s0, CV_8UC1, buffer_s0);

    // jstring jstr = env->NewStringUTF(buffer);

    // delete[] buffer;
    // buffer = NULL;

    Ptr<ml::KNearest> knn = getKnn(fs);

    string result = grabNumbers(decodedImage, knn);

    return env->NewStringUTF(result.c_str());
    // return env->NewStringUTF(ss.str().c_str()); // OK
}

extern "C" jobjectArray
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_getFilenameInAssets(
    JNIEnv *env,
    jobject javaThis,
    jobject pAssetManager)
{

    // Number of file
    const int maxNumFile = 20;
    int numFile = 0;

    // Asset manager
    const char *filename;
    AAssetManager *assetManager = AAssetManager_fromJava(env, pAssetManager);
    AAssetDir *assetFolder = AAssetManager_openDir(assetManager, "");

    // Put all file name into temporary array
    jobjectArray tempArr = (jobjectArray)env->NewObjectArray(
        maxNumFile,
        env->FindClass("java/lang/String"),
        env->NewStringUTF(""));

    while (
        ((filename = AAssetDir_getNextFileName(assetFolder)) != NULL) &&
        (numFile < maxNumFile))
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

extern "C" jstring
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_getFileContent(
    JNIEnv *env,
    jobject javaThis,
    jobject pAssetManager)
{
    const char *filename = "raw-features.yml";
    AAssetManager *assetManager = AAssetManager_fromJava(env, pAssetManager);

    /*
    All mode:
        - AASSET_MODE_UNKNOWN: Not known how the data is to be accessed
        - AASSET_MODE_RANDOM: Read chunks, and seek forward and backward
        - AASSET_MODE_STREAMING: Read sequentially, with an occasional
          forward seek
        - AASSET_MODE_BUFFER: Attempt to load contents into memory,
          for fast small reads
    */
    AAsset *file = AAssetManager_open(
        assetManager,
        filename,
        AASSET_MODE_UNKNOWN);

    if (file == NULL)
    {
        return env->NewStringUTF("ERROR: Can not open file...");
    }

    long size = AAsset_getLength(file);
    char *buffer = new char[size];
    AAsset_read(file, buffer, size);

    AAsset_close(file);

    jstring jstr = env->NewStringUTF(buffer);

    delete[] buffer;
    buffer = NULL;

    return jstr;
}