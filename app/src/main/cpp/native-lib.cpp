#include <jni.h>
#include <string>
#include <android/asset_manager_jni.h>
#include <opencv2/core.hpp>
#include <sstream>
#include <android/asset_manager.h>
#include "../../../../sudoku-recognizer/src/lib/sudoku.h"

#include <curl/curl.h>

using namespace cv;






const char data[]="input=000604700706000009000005080070020093800000005430010070050200000300000208002301000";

struct WriteThis {
    const char *readptr;
    long sizeleft;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    struct WriteThis *pooh = (struct WriteThis *)userp;

    if(size*nmemb < 1)
        return 0;

    if(pooh->sizeleft) {
        *(char *)ptr = pooh->readptr[0]; /* copy one single byte */
        pooh->readptr++;                 /* advance pointer */
        pooh->sizeleft--;                /* less data left */
        return 1;                        /* we return 1 byte at a time! */
    }

    return 0;                          /* no more data left to deliver */
}





extern "C" jstring
Java_moulinet_tech_moulinet_1sudoku_1app_MainActivity_grab(
    JNIEnv *env,
    jobject javaThis,
    jstring fileName,
    jobject pAssetManager)
{







    CURL *curl;
    CURLcode res;

    struct WriteThis pooh;

    pooh.readptr = data;
    pooh.sizeleft = (long)strlen(data);

    /* In windows, this will init the winsock stuff */
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    /* Check for errors */
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_global_init() failed: %s\n",
                curl_easy_strerror(res));
        // return 1;
    }

    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) {
        /* First set the URL that is about to receive our POST. */
        curl_easy_setopt(curl, CURLOPT_URL, "http://moulinet.tech/sudokus");

        /* Now specify we want to POST data */
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        /* we want to use our own read function */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

        /* pointer to pass to our read function */
        curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);

        /* get verbose debug output please */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        /*
          If you use POST to a HTTP 1.1 server, you can send data without knowing
          the size before starting the POST if you use chunked encoding. You
          enable this by adding a header like "Transfer-Encoding: chunked" with
          CURLOPT_HTTPHEADER. With HTTP 1.0 or without chunked transfer, you must
          specify the size in the request.
        */
#ifdef USE_CHUNKED
        {
      struct curl_slist *chunk = NULL;

      chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
      res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
      /* use curl_slist_free_all() after the *perform() call to free this
         list again */
    }
#else
        /* Set the expected POST size. If you want to POST large amounts of data,
           consider CURLOPT_POSTFIELDSIZE_LARGE */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, pooh.sizeleft);
#endif

#ifdef DISABLE_EXPECT
        /*
      Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue"
      header.  You can disable this header with CURLOPT_HTTPHEADER as usual.
      NOTE: if you want chunked transfer too, you need to combine these two
      since you can only set one list of headers with CURLOPT_HTTPHEADER. */

    /* A less good option would be to enforce HTTP 1.0, but that might also
       have other implications. */
    {
      struct curl_slist *chunk = NULL;

      chunk = curl_slist_append(chunk, "Expect:");
      res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
      /* use curl_slist_free_all() after the *perform() call to free this
         list again */
    }
#endif

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();















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