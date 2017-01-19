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

extern "C" void
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_loadImage(
        JNIEnv *env,
        jobject javaThis,
        jstring fileName,
        jlong addrImage,
        jobject pAssetManager)
{


    Mat &img_input = *(Mat *) addrImage;
    const char *nativeFileName = env->GetStringUTFChars(fileName, JNI_FALSE);
    stringstream fileName_ss;

    fileName_ss << "puzzles/";
    fileName_ss << nativeFileName;
    fileName_ss << ".jpg";

    AAssetManager *assetManager = AAssetManager_fromJava(env, pAssetManager);

    AAsset *file = AAssetManager_open(
            assetManager,
            fileName_ss.str().c_str(),
            AASSET_MODE_UNKNOWN);

    long size = AAsset_getLength(file);
    char *buffer = new char[size];
    AAsset_read(file, buffer, size);

    Mat rawData = Mat(1, size, CV_8UC1, buffer);

    img_input = imdecode(rawData, CV_8UC1);
    if (img_input.data == NULL)
    {
        throw std::logic_error("jpg decoding error");
    }

}



extern "C" void
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_solve(
        JNIEnv *env,
        jobject javaThis,
        jlong addrImage,
        jobject pAssetManager)
{

    Mat &img_input = *(Mat *) addrImage;

    extractionInformation extractInfo, extractInfo2;



    // get knn --------------------------------

    /*
    All mode:
        - AASSET_MODE_UNKNOWN: Not known how the data is to be accessed
        - AASSET_MODE_RANDOM: Read chunks, and seek forward and backward
        - AASSET_MODE_STREAMING: Read sequentially, with an occasional
          forward seek
        - AASSET_MODE_BUFFER: Attempt to load contents into memory,
          for fast small reads
    */
    AAssetManager *assetManager = AAssetManager_fromJava(env, pAssetManager);
    const char *filename = "raw-features.yml";
    AAsset *file = AAssetManager_open(
            assetManager,
            filename,
            AASSET_MODE_UNKNOWN);

    if (file == NULL)
    {
        throw std::logic_error("ERROR: Can not open file...");
    }

    long size = AAsset_getLength(file);
    char *buffer = new char[size];
    AAsset_read(file, buffer, size);

    FileStorage fs(buffer, FileStorage::READ | FileStorage::MEMORY);

    delete[] buffer;
    buffer = NULL;

    Ptr<ml::KNearest> knn = getKnn(fs);
    // get knn - end --------------------------------



    vector<Point> bigestApprox = findBigestBlob(img_input);

    extractInfo = extractPuzzle(img_input, bigestApprox);
    Mat extractedPuzzle = extractInfo.image;

    Mat finalExtraction = recursiveExtraction(extractedPuzzle);

    string initialStateOfTheSudoku = grabNumbers(finalExtraction, knn);









    // request the server for solution --------------------------------

    stringstream ss, url;

    url << "http://moulinet.tech/sudokus?input=";
    url << initialStateOfTheSudoku;

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

    string solution = ss.str();

    // request the server for solution - end --------------------------------

    Mat writen = writeOnPuzzle(finalExtraction, solution);

    // merge solved sudoku in original image
    warpPerspective(writen, img_input, extractInfo.transformation, img_input.size(), WARP_INVERSE_MAP, BORDER_TRANSPARENT);

}