#include <jni.h>
#include <string>
#include <android/asset_manager_jni.h>
#include <opencv2/core.hpp>
#include <sstream>
#include <android/asset_manager.h>
#include "../../../../sudoku-recognizer/src/lib/sudoku.h"

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

Ptr<ml::KNearest> knn;
Ptr<ml::SVM> svm;

using namespace cv;

extern "C" void
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_loadImage(
    JNIEnv *env,
    jobject javaThis,
    jstring fileName,
    jlong addrImage,
    jobject pAssetManager)
{

    Mat &img_input = *(Mat *)addrImage;
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

    if (svm == nullptr)
    {
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
    }

    Mat &img_input = *(Mat *)addrImage;

    Mat preprocessed = preprocess(img_input.clone(), true);

    vector<Point> bigestApprox = findBiggestBlob(preprocessed, img_input);

    if (!bigestApprox.empty())
    {

        ExtractionInformation extractInfo;

        extractInfo = extractPuzzle(img_input, bigestApprox);
        Mat extractedPuzzle = extractInfo.image;

        Mat finalExtraction = recursiveExtraction(extractedPuzzle);
        Mat resized;
        resize(finalExtraction, resized, Size(640, 360));

        string resultTab[81];

        string initialStateOfTheSudoku = grabNumbers(finalExtraction, svm);


        stringstream solution, ss;


        if (initialStateOfTheSudoku.find("000000000") != 0)
        {
            stringstream solution;
            std::pair<bool, std::array<int, 81>> pair = solve(initialStateOfTheSudoku.c_str());

            if (pair.first)
            {
                std::array<int, 81> ans = pair.second;
                for (int i = 0; i < 81; i++)
                {
                    solution << ans[i];
                }
                // LOGI("%s", solution.str().c_str());

                if (areSameNonZeroValues(initialStateOfTheSudoku, solution.str()))
                {
                    Mat writen = writeOnPuzzle(finalExtraction, initialStateOfTheSudoku, solution.str());
                    warpPerspective(writen, img_input, extractInfo.transformation, img_input.size(), WARP_INVERSE_MAP, BORDER_TRANSPARENT);
                }
            }
        }
        // LOGI("%s", initialStateOfTheSudoku.c_str());
    }
}