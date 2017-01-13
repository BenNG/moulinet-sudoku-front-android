#include <jni.h>
#include <string>
#include <android/asset_manager_jni.h>
#include <opencv2/core.hpp>
#include <sstream>
#include <android/asset_manager.h>
#include "../../../../sudoku-recognizer/src/lib/sudoku.h"

#include <curl/curl.h>


#ifdef ANDROID
#include <android/log.h>
#include <jni.h>
#ifdef __LP64__
#define SIZE_T_TYPE "%lu"
#else
#define SIZE_T_TYPE "%u"
#endif
#endif

#ifdef ANDROID
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "Moulinet.tech/SUDOKU", __VA_ARGS__))
#else
#define LOGI(...) printf(__VA_ARGS__)
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}
using namespace cv;

/**
 * This function will parse the file in assets/puzzles/{fileName}.jpg and return the initial state of the puzzle inside the file
 *
 * */
extern "C" jstring
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_grabNumbers(
    JNIEnv *env,
    jobject javaThis,
    jstring fileName,
    jobject pAssetManager)
{


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



extern "C" jstring
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_sudokuSolver(
        JNIEnv *env,
        jobject javaThis,
        jstring initialStateOfTheSudoku)
{

    const char *nativeP = env->GetStringUTFChars(initialStateOfTheSudoku, JNI_FALSE);

    stringstream ss, url;

    url << "http://moulinet.tech/sudokus?input=";
    url << nativeP;

    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;

    chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.str().c_str());

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* get it! */
    res = curl_easy_perform(curl_handle);

    /* check for errors */
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    else {
        /*
         * Now, our chunk.memory points to a memory block that is chunk.size
         * bytes big and contains the remote file.
         *
         * Do something nice with it!
         */

        ss << chunk.memory;

        printf("%lu bytes retrieved\n", (long)chunk.size);
    }

    curl_easy_cleanup(curl_handle);

    free(chunk.memory);

    curl_global_cleanup();

    return env->NewStringUTF(ss.str().c_str());
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