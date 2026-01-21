#include <stdlib.h>
#include <string.h>
#include "id3_utils.h"

/**
TODO: Add documention 
 */
TagData* create_tag_data() {
    TagData *data = (TagData *)malloc(sizeof(TagData));
    if (data) {
        data->version = NULL;
        data->title = NULL;
        data->artist = NULL;
        data->album = NULL;
        data->year = NULL;
        data->comment = NULL;
        data->genre = NULL;
        // Initialize other fields as needed
    }
    return data;
}

/**
TODO: Add documention as sample given
 */
void free_tag_data(TagData *data) {
    if (data) {
        free(data->version);
        free(data->title);
        free(data->artist);
        free(data->album);
        free(data->year);
        free(data->comment);
        free(data->genre);
        // Free other fields as needed
        free(data);
    }
}
