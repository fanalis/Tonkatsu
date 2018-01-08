#include "mangastream.h"

bool startsWith(const char* str, const char* prefix){
    return strncmp(prefix,str,strlen(prefix)) == 0;
}

/**
 * This will convert something like
 * "ShoKuGeKi no SouMa" to "shokugeki_no_souma"
 **/
void sanitizeText(char *str){
    while(*str){
        if(isspace(*str))
            *str = '_';
        else
            *str = tolower(*str);
        str++;
    }
}

char* getFilename(char* str){
    str = str+strlen(str)-1;
    while(*str != '/')
        str--;
    return str;
}

void initHttpRequestStruct(struct HttpRequestContent* http){
    initResponseContent(&((*http).body));
    initResponseContent(&((*http).header));
}

void freeHttpRequestStruct(struct HttpRequestContent* http){
    freeResponseContent(&((*http).body));
    freeResponseContent(&((*http).header));
}


void initResponseContent(struct ResponseContent *r){
    (*r).content = malloc(1); //It'll expand later with the realloc
    (*r).size = 0;
}

void freeResponseContent(struct ResponseContent *r){
    free((*r).content);
    (*r).size = 0;
}

/**
 * Callback function to write in desired element
 * Used in the "make_request" function to get the body
 * and header of the pages
 **/
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
    size_t realsize = size * nmemb;
    struct ResponseContent *mem = (struct ResponseContent *)userp;

    mem->content = realloc(mem->content, mem->size + realsize + 1);
    if(mem->content == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->content[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->content[mem->size] = 0;

    return realsize;
}

/**
 * Just, make an HTTP request with cURL
 **/

struct HttpRequestContent makeRequest(char* url){
    struct HttpRequestContent response;
    initHttpRequestStruct(&response);

    CURL* mangastream;
    mangastream = curl_easy_init();
    if(mangastream){
        struct curl_slist *header_list = NULL;

        //Add headers, could be more, but for now, we'll only use User-Agent
        header_list = curl_slist_append(header_list,"User-Agent: c_mangastream_downloader (https://github.com/fanalis/c_mangastream_downloader)");

        curl_easy_setopt(mangastream, CURLOPT_URL, url); //Set url
        curl_easy_setopt(mangastream, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); //Callback function
        curl_easy_setopt(mangastream, CURLOPT_WRITEHEADER, (void *) &(response.header)); //Write headers
        curl_easy_setopt(mangastream, CURLOPT_WRITEDATA, (void *)&(response.body)); //Write the data to this var
        curl_easy_setopt(mangastream, CURLOPT_HTTPHEADER, header_list); //Http headers

        curl_easy_perform(mangastream);
        curl_easy_cleanup(mangastream);     
    }
    return response;

}

/**
 * Makes a request to a url, then saves the image in the desired location
 **/

void saveImage(char *url, char *path){
    CURL *image;
    CURLcode imgresult;
    FILE *fp;

    image = curl_easy_init();
    if(image){
        fp = fopen(path,"wb"); //Open the file

        //Configure curl
        curl_easy_setopt(image, CURLOPT_URL,url);
        curl_easy_setopt(image, CURLOPT_WRITEFUNCTION,NULL); //No callback function, default if fwrite
        curl_easy_setopt(image, CURLOPT_WRITEDATA, fp);

        imgresult = curl_easy_perform(image);

        fclose(fp); //Close the file
    }
    //clean the var
    curl_easy_cleanup(image);
    
}

/**
 * Get all manga list
**/
void getMangaList(GumboNode *node,int *counter){
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    (*counter)++;
    GumboAttribute *href;
    if(node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes,"href"))){
        if(startsWith(href->value,"/manga/")){
            printf("Level %d\n",*counter);
            printf("%s\n",href->value+7);
        }
        else if(startsWith(href->value,"/r/")){
            printf("Level %d\n",*counter);
            printf("%s\n",href->value);
        }
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        getMangaList(children->data[i],counter);
    }
}

/** 
 * Returns the manga page url, should be something like:
 * "https://img.mangastream.com/cdn/...."
**/ 
void getMangaPageUrl(GumboNode* node,char **url,bool *found) {
    if (node->type != GUMBO_NODE_ELEMENT || *found) {
        return;
    }
    GumboAttribute* src;
    if (node->v.element.tag == GUMBO_TAG_IMG &&
        (src = gumbo_get_attribute(&node->v.element.attributes, "src"))
        && startsWith(src->value,"//")) {
            int len = strlen(src->value) + 6 + 1; //url length + "https" length + extra for "\0"
            *url = realloc(*url,len); //Realloc the memory
            strcat(*url,src->value); //Concatenate "https:" with the url
            //*url[len-1] = '\0'; //Null byte
            *found = true; //We don't need to continue searching
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        getMangaPageUrl(children->data[i],url,found);
    }
}


void getLastEpisodeUrl(GumboNode* node,char **url,bool *found,char *name){
    if (node->type != GUMBO_NODE_ELEMENT || *found) {
        return;
    }
    GumboAttribute *href;
    if(node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes,"href"))
        && startsWith(href->value,name)){
            int len = strlen(href->value) + strlen(*url);
            *url = realloc(*url,len);
            //*url = malloc(strlen(href->value)+1); //Allocate the memory
            //*url[strlen(href->value)] = '\0';
            strcat(*url,href->value);
            //*url[len] = '\0'; //Null byte
            //strcpy(*url,href->value);
            *found = true;
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        getLastEpisodeUrl(children->data[i],url,found,name);
    }
}

void incrementPage(char **url){
    char* temp = *url;
    temp = temp+strlen(temp)-1;
    while(*(temp-1) != '/')
        temp--;
    char buffer[5]; //have you seen a manga of 100000 pages?
    int num = atoi(temp)+1; //Get the page number and increment it
    int prev_len = strlen(temp);
    sprintf(buffer,"%d",num);
    temp = malloc(strlen(*url) + strlen(buffer));
    strncpy(temp,*url,strlen(*url)-prev_len);
    strcat(temp,buffer);
	free(*url);
    *url = temp;
}
