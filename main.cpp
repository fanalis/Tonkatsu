#include <iostream>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
using namespace std;

size_t callbackFunction(void *contents, size_t size,size_t nmemb, string *s){
    /*
    size_t newLength = size*nmemb;
    size_t oldLength = (string*)s.size();
    try
    {
        s->resize(oldLength + newLength);
    }
    catch(std::bad_alloc &e)
    {
        //handle memory problem
        return 0;
    }

    std::copy((char*)contents,(char*)contents+newLength,(string*)s.begin()+oldLength);*/
    *s += (char*) contents;
    return size * nmemb;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream){
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

void manga_id(string name){
    int id = -1; //Manga id
    char* url = "https://readms.net/manga"; //Manga list url
    //char* url = "http://httpbin.org/get";
    curl_global_init(CURL_GLOBAL_ALL);
 

    static const char *headerfilename = "head.out";
    FILE *headerfile;
    static const char *bodyfilename = "body.out";
    FILE *bodyfile;

    CURL* mangastream;
    CURLcode result;

    mangastream = curl_easy_init();
    if(mangastream){
        string body;

        printf("header list done\n");
        curl_easy_setopt(mangastream, CURLOPT_URL, url); //Set url
        printf("set url done\n");
        curl_easy_setopt(mangastream, CURLOPT_WRITEFUNCTION, callbackFunction); //No callback
        curl_easy_setopt(mangastream, CURLOPT_WRITEDATA, &body); //Write the data to this var
        printf("destination done\n");
        //curl_easy_setopt(mangastream, CURLOPT_HTTPHEADER, header_list); //Http headers
        curl_easy_setopt(mangastream,CURLOPT_FOLLOWLOCATION,1);
/*
        headerfile = fopen(headerfilename,"w");
        if (headerfile == NULL) {
            curl_easy_cleanup(mangastream);
            return;
        }
        bodyfile = fopen(bodyfilename,"w");
        if (bodyfile == NULL) {
            curl_easy_cleanup(mangastream);
            return;
        }

        curl_easy_setopt(mangastream, CURLOPT_HEADERDATA, headerfile); //Write the data to this var
        curl_easy_setopt(mangastream, CURLOPT_WRITEDATA, bodyfile); //Write the data to this var

        curl_easy_perform(mangastream);


        fclose(bodyfile);
        fclose(headerfile);

    
        curl_easy_cleanup(mangastream);*/


        curl_easy_perform(mangastream);
        printf("request done\n");
        //printf("%s\n",body);
        cout << body.size() << endl;
        curl_easy_cleanup(mangastream);
    }
}


int main(){
    /*
    CURL *image; 
    CURLcode imgresult; 
    FILE *f = fopen("image.png","wb");

    image = curl_easy_init();
    if(image){
        curl_easy_setopt(image, CURLOPT_URL, "https://img.mangastream.com/cdn/manga/112/4797/01.png"); 
        curl_easy_setopt(image, CURLOPT_WRITEFUNCTION, NULL); 
        curl_easy_setopt(image, CURLOPT_WRITEDATA, f);

        imgresult = curl_easy_perform(image);
    }
    curl_easy_cleanup(image);
    fclose(f);*/
    manga_id("none");
    return 0;
}