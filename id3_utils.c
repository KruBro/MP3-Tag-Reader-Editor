#include <stdlib.h>
#include <string.h>
#include "id3_utils.h"

/**
TODO: Add documention 
 */
TagData* create_tag_data() {
    TagData *data = (TagData *)calloc(1, sizeof(TagData));
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
        if((data->album_art.mime_type) != NULL) free(data->album_art.mime_type);
        if((data->album_art.image_data) != NULL) free(data->album_art.image_data);
        free(data);
    }
}

uint32_t big_endian_to_host(uint8_t *bytes)
{
    uint32_t val = 0;
    for(int i = 0; i < 4; i++)
    {
        // Shift starts at 24 and drops by 8 each time: 24, 16, 8, 0
        val |= (uint32_t)bytes[i] << (24 - (i * 8));
    }
    return val;
}

uint32_t synchsafe_to_int(uint8_t *bytes)
{
    uint32_t val = 0;
    for(int i = 0; i < 4; i++)
    {
        // Shift starts at 21 and drops by 7 each time: 21, 14, 7, 0
        val |= (uint32_t)bytes[i] << (21 - (i * 7));
    }
    return val;
}

void int_to_synchsafe(uint32_t num, uint8_t* out_bytes)
{
    unsigned int mask = 0x7F;
    for(int i = 0; i < 4; i++)
    {
        out_bytes[i] = (num >> (21 - (i * 7))) & mask;
    }
}