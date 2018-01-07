#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <ctype.h>
#include "gumbo.h"

#ifndef MANGASTREAM_H
#define MANGASTREAM_H

struct ResponseContent {
  char *content;
  size_t size;
};

struct HttpRequestContent{
    struct ResponseContent body;
    struct ResponseContent header;
};

/** 
 * Check if the string starts with the desired substring
 * Same as Python function str.startswith()
 **/
bool startsWith(const char* str, const char* prefix);
void sanitizeText(char *str); //Replaces all whitespaces with underscore and all uppercase with lowercase
char* getFilename(char* str); //Return the pointer to where the filename exists

void initHttpRequestStruct(struct HttpRequestContent* http);
void freeHttpRequestStruct(struct HttpRequestContent* http);
void initResponseContent(struct ResponseContent *r);
void freeResponseContent(struct ResponseContent *r);

//Callback function to write elements of cURL request
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
struct HttpRequestContent makeRequest(char* url);
void saveImage(char *url, char *path);
void getMangaList(GumboNode *node,int *counter);
void getMangaPageUrl(GumboNode* node,char **url,bool *found);
void getLastEpisodeUrl(GumboNode* node,char **url,bool *found,char *name);
void getEpisodeLists(char* name); //WIP
void incrementPage(char** url);
void downloadEpisode(char* url);


#endif