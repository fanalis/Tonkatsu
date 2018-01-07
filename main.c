#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mangastream.h"



int main(){
    char buffer[256] = "/r/"; //read the input
    scanf("%[^\n]",buffer+3);
    sanitizeText(buffer);
    char* folder = malloc(6);
    strcpy(folder,"output");
    //folder[6] = '\0';
    struct stat s = {0};
    //If "output" folder does not exists, create it
    if(stat(folder,&s))
        mkdir(folder,S_IRWXU);
    char *manga_list_url = "https://readms.net/manga";
    bool found = false;
    char *last_episode = malloc(strlen("https://readms.net"));
    strcpy(last_episode,"https://readms.net");
    struct HttpRequestContent manga_list = makeRequest(manga_list_url);
    GumboOutput* output = gumbo_parse(manga_list.body.content);
    //freeHttpRequestStruct(&manga_list); //Free the manga list content, since it's not necessary anymore
    getLastEpisodeUrl(output->root,&last_episode,&found,buffer); //get the last episode url
    if(found){
        //Create the folder
        folder = realloc(folder,strlen(folder)+strlen(buffer));
        strcat(folder,buffer+2);
        //folder[strlen(folder)] = '\0';
        struct stat s2 = {0};
        if(stat(folder,&s2)){
            mkdir(folder,S_IRWXU);
        }

        struct HttpRequestContent manga_page;
        bool invalid = false;
        char *page = NULL;
        char *fullpath = NULL;
        char *name;
        //While there are manga pages
        while(!invalid){
            found = false;
            page = malloc(6);
            strcpy(page,"https:");
            gumbo_destroy_output(&kGumboDefaultOptions, output); //Destroy previous output
            manga_page = makeRequest(last_episode);
            invalid = strstr(manga_page.header.content,"/error/notfound");
            if(!invalid){
                output = gumbo_parse(manga_page.body.content);
                getMangaPageUrl(output->root,&page,&found);
                name = getFilename(page);
                printf("Downloading %s....\n",name);
                fullpath = malloc(strlen(folder)+strlen(name));
                //fullpath[strlen(folder)+strlen(name)] = '\0';
                strcpy(fullpath,folder);
                strcat(fullpath,name);
                saveImage(page,fullpath);
                free(fullpath);
                fullpath = NULL;
                name = NULL;
                incrementPage(&last_episode);
                printf("Done!\n");
            }
            freeHttpRequestStruct(&manga_page);
            free(page);
            page = NULL;
        }
        
    }
    return 0;
}