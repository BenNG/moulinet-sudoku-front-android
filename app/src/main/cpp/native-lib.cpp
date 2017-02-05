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

Ptr<ml::KNearest> knn;
Ptr<ml::SVM> svm;

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
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_prepro(
        JNIEnv *env,
        jobject javaThis,
        jlong addrImage,
        jobject pAssetManager)
{



    if(svm == nullptr){

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

        svm = getSvm(fs);
        // get knn - end --------------------------------

        // LOGI("%s", initialStateOfTheSudoku.c_str());
        LOGI("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");


    }



    Mat &img_input = *(Mat *) addrImage;



    Mat preprocessed = preprocess(img_input.clone());

    vector<Point> bigestApprox = findBiggestBlob(preprocessed, img_input);

    if(!bigestApprox.empty()){

        ExtractionInformation extractInfo, extractInfo2;

        extractInfo = extractPuzzle(img_input, bigestApprox);
        Mat extractedPuzzle = extractInfo.image;

        Mat finalExtraction = recursiveExtraction(extractedPuzzle);
        Mat resized;
        resize(finalExtraction, resized, Size(640,360));


        string initialStateOfTheSudoku = grabNumbers(finalExtraction, svm);


        if(initialStateOfTheSudoku == "009100056000060003005300904030750000520806097000012080601003500800070000740008600"){
            LOGI("%s", "Yessssssssssssss");
            LOGI("%s", initialStateOfTheSudoku.c_str());

        }

stringstream solution;
        std::pair<bool, std::array<int, 81>> pair = solve(initialStateOfTheSudoku.c_str());

        if (pair.first)
        {
            std::array<int, 81> ans = pair.second;
            for (int i = 0; i < 81; i++)
            {
                solution << ans[i];
            }
            LOGI("%s", solution.str().c_str());


            Mat writen = writeOnPuzzle(finalExtraction, initialStateOfTheSudoku, solution.str());

            warpPerspective(writen, img_input, extractInfo.transformation, img_input.size(), WARP_INVERSE_MAP, BORDER_TRANSPARENT);


        }





        // LOGI("%s", initialStateOfTheSudoku.c_str());

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

    ExtractionInformation extractInfo, extractInfo2;



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


    Mat preprocessed = preprocess(img_input.clone());

    vector<Point> bigestApprox = findBiggestBlob(preprocessed, img_input);

    extractInfo = extractPuzzle(img_input, bigestApprox);
    Mat extractedPuzzle = extractInfo.image;

    Mat finalExtraction = recursiveExtraction(extractedPuzzle);

    string initialStateOfTheSudoku = grabNumbers(finalExtraction, knn);

    string solution = askServerForSolution(initialStateOfTheSudoku);

    Mat writen = writeOnPuzzle(finalExtraction, initialStateOfTheSudoku, solution);

    // merge solved sudoku in original image
    warpPerspective(writen, img_input, extractInfo.transformation, img_input.size(), WARP_INVERSE_MAP, BORDER_TRANSPARENT);

}