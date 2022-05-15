//
// Created by Norman on 2021/9/27.
//

#ifndef CSLH_DOWNLOADER_H
#define CSLH_DOWNLOADER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
void download(char *url, char *file_name);


#endif //CSLH_DOWNLOADER_H
