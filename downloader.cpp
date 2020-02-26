/*
 * downloader.cpp
 * Copyright (C) 2020 Haoyang <peter@peterchen.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include <curl/curl.h>
#include <pthread.h>
#include <string>
#include <unistd.h>

using namespace std;

int nThread = 1, runingThread = 0;
string url = "http://example.com/";

// each thread is associated with one node to track the progress of download
struct Node {
    FILE* fp;
    long startPos;
    long endPos;
    void* curl;
    pthread_t tid;
};

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

// write userdata into ptr with size, and update node pointer with nmemb
static size_t writeFunc(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    Node* node = (Node*)userdata;
    size_t written = 0;
    pthread_mutex_lock(&g_mutex);
    if (node->startPos + size * nmemb <= node->endPos) {
        fseek(node->fp, node->startPos, SEEK_SET);
        written = fwrite(ptr, size, nmemb, node->fp);
        node->startPos += size * nmemb;
    } else {
        fseek(node->fp, node->startPos, SEEK_SET);
        written = fwrite(ptr, 1, node->endPos - node->startPos + 1, node->fp);
        node->startPos = node->endPos;
    }
    pthread_mutex_unlock(&g_mutex);
    return written;
}

// simple printf to cmd for showing the downloading progress of each thread
// to scale: I have included upload interface for future
int progressFunc(void* ptr, double totalToDownload, double nowDownloaded,
    double totalToUpLoad, double nowUpLoaded)
{
    int percent = 0;
    if (totalToDownload > 0) {
        percent = (int)(nowDownloaded / totalToDownload * 100);
    }

    if (percent % 20 == 0)
        printf("download progress %0d%%\n", percent);
    return 0;
}

// get the file size by the given url
long getFileSize(const char* url)
{
    double downloadFileLenth = 0;
    CURL* handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_HEADER, 1);
    curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
    if (curl_easy_perform(handle) == CURLE_OK) {
        curl_easy_getinfo(
            handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
    } else {
        downloadFileLenth = -1;
    }
    return downloadFileLenth;
}

// main curl perform function for each thread, take in thread node
void* workThread(void* pData)
{
    Node* node = (Node*)pData;

    curl_easy_perform(node->curl);

    curl_easy_cleanup(node->curl);

    pthread_mutex_lock(&g_mutex);
    runingThread--;
    printf("thread %ld exit\n", (long)node->tid);
    pthread_mutex_unlock(&g_mutex);
    delete node;
    pthread_exit(0);

    return NULL;
}

// driver function for handling the entire downloading process
bool download(string path, string fileName)
{
    long fileLength = getFileSize(url.c_str());
    if (fileLength <= 0) {
        printf("cannot get the correct file lenghth, please check your url");
        return false;
    } else
        printf("file size:%ld", fileLength);
    const string outFileName = path + fileName;
    FILE* fp = fopen(outFileName.c_str(), "wb");
    if (!fp) {
        printf("cannot open the file, please check the url");
        return false;
    }

    long pSize = fileLength / nThread;
    for (int i = 0; i <= nThread; ++i) {
        Node* node = new Node();
        if (i < nThread) {
            node->startPos = i * pSize;
            node->endPos = (i + 1) * pSize - 1;
        } else if (fileLength % nThread) {
            node->startPos = i * pSize;
            node->endPos = fileLength - 1;
        } else
            break;
        CURL* curl = curl_easy_init();

        node->curl = curl;
        node->fp = fp;

        char range[64] = { 0 };
        snprintf(range, sizeof(range), "%ld-%ld", node->startPos, node->endPos);

        // Download options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)node);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressFunc);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5L);
        curl_easy_setopt(curl, CURLOPT_RANGE, range);

        pthread_mutex_lock(&g_mutex);
        runingThread++;
        pthread_mutex_unlock(&g_mutex);
        int rc = pthread_create(&node->tid, NULL, workThread, node);
    }
    while (runingThread > 0)
        usleep(1000000L);

    fclose(fp);

    printf("download succeed......\n");

    return true;
}

int main(int argc, char** argv)
{

    if (argc == 4) {
        url = argv[1];
        nThread = atoi(argv[3]);
        if (nThread <= 0)
            nThread = 1;
        download("./", "file");
    } else
        printf("Invalid argument, please use ./downloader <url> -c <nThread>");
    return 0;
}
