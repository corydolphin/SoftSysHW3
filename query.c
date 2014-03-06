#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFER_SIZE 256 * 1024 //256 KB, should be large enough
#include <curl/curl.h>
#include <jansson.h>

typedef struct string
{
    char *ptr;
    size_t len;
} post_write_buffer;

void init_string(post_write_buffer *s)
{
    s->len = 0;
    s->ptr = malloc(s->len + 1);
    if (s->ptr == NULL)
    {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, post_write_buffer *s)
{
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL)
    {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

char *post(char *url, char *data, post_write_buffer *s)
{
    CURL *curl;
    CURLcode res;
    // char data[200] = "{ \"query\" : \"MATCH (n {name: {name}}) RETURN n\", \"params\": { \"name\" : \"Kevin Bacon\" } }";
    // char url[50] = "http://localhost:7474/db/data/cypher";

    struct curl_slist *headers = NULL;
    // post_write_buffer s;
    init_string(s);

    curl = curl_easy_init();
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_slist_append(headers, "charsets: utf-8");

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return s->ptr;
}

char *get_node_url(char *actor_name)
{
    post_write_buffer s;
    json_error_t error;
    json_t *root, *data, *node_url;
    char *query = (char *) malloc(sizeof(char) * 200);
    char *ret = (char *) malloc(sizeof(char) * 100);
    sprintf(query, "{ \"query\" : \"MATCH (n {name: {name}}) RETURN n\", \"params\": { \"name\" : \"%s\" } }", actor_name);
    char url[50] = "http://localhost:7474/db/data/cypher";

    char *text = post(url, query, &s);
    root = json_loads(text, 0, &error);

    data = json_array_get(json_array_get(json_object_get(root, "data"), 0), 0);
    node_url = json_object_get(data, "self");

    char *d = json_string_value(node_url);
    if (d == NULL)
    {
        fprintf(stderr, "Failed to get node_url. Did you type the actor name correctly?\n");
        exit(EXIT_FAILURE);
    }

    strcpy(ret, d);

    free(text); //same as free(s->ptr)
    free(query);
    json_decref(root);

    return ret;
}

int get_shortest_path(char *origin_node_url, char *dest_node_url)
{
    post_write_buffer s;
    json_error_t error;
    json_t *root, *path_length;
    int ret;
    char *post_data = (char *) malloc(sizeof(char) * 500);
    char *url = (char *) malloc(sizeof(char) * 200);

    sprintf(post_data, "{ \"to\" : \"%s\", \"max_depth\" : 100, \"algorithm\" : \"shortestPath\" }", dest_node_url);

    sprintf(url, "%s/path", origin_node_url);
    char *text = post(url, post_data, &s);
    root = json_loads(text, 0, &error);

    path_length = json_object_get(root, "length");
    ret = (int) json_integer_value(path_length);

    free(post_data); //same as free(s->ptr)
    free(url);
    free(text);
    json_decref(root);

    return ret;

}


// Truncates the string at the first newline, if there is one.
void rstrip(char s[])
{
    //*ptr points to the location in s where the first \n  occurs
    char *ptr = strchr(s, '\n');
    //if \n occurs, replace it with a null terminator to end the string
    if (ptr)
    {
        *ptr = '\0';
    }
}

int main(int argc, char *argv[])
{
    char origin_actor[80];
    printf("Enter the full name of the actor for whom you would like the Bacon path:\n");
    fgets(origin_actor, 80, stdin); //read 80 characters from stdin and store them in search_for
    rstrip(origin_actor); //strip the trailing line break from the string

    char *origin = get_node_url(origin_actor);
    char *KevinBacon = get_node_url("Bacon, Kevin (I)");
    
    int r = get_shortest_path(origin, KevinBacon);
    printf("%s is %i hops away from Kevin Bacon\n", origin_actor,r);
    return 0;
}
