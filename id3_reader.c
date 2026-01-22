/**
 * @file id3_reader.c
 * @brief Implementation of functions for reading ID3 tags from MP3 files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3_reader.h"
#include "error_handling.h"

/**
TODO: Add documention as sample given
 */
TagData* read_id3_tags(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        display_error("File Could Not Open");
        return NULL;
    }

    unsigned char header[10];
    fread(header, 1, 10, fp);
    printf("Debug: Header Flags: 0x%02X\n", header[5]);
    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3')
    {
        fclose(fp);
        return NULL;
    }
    printf("[Info]: ID3 Tag Found\n");
    TagData *data = create_tag_data();
    data->tag_version = header[3];
    if(data->tag_version != 0x03 && data->tag_version != 0x04)
    {
        display_error("Version not supported ");
        free_tag_data(data);
        return NULL;
    }

    uint32_t tag_size = synchsafe_to_int(&header[6]);

    int bytes_read = 0;
    while(bytes_read < tag_size)
    {
        unsigned char frame_header[10];
        if(fread(frame_header, 1, 10, fp) != 10)
        {
            break;
        }
        bytes_read += 10;
        if(frame_header[0] == 0)
        {
            data->padding_size = tag_size - bytes_read + 10;
            break;
        }

        uint32_t frame_size;
        if(data->tag_version == 3)
        {
            frame_size = big_endian_to_host(&frame_header[4]);
        }
        else if(data->tag_version == 4)
        {
            frame_size = synchsafe_to_int(&frame_header[4]);
        }
        /* DEBUG: Print the Frame ID being processed */
        printf("Debug: Found Frame ID: %.4s, Size: %d\n", frame_header, frame_size);

        char *content = malloc(frame_size + 1);
        fread(content, 1, frame_size, fp);
        content[frame_size] = '\0';
        bytes_read += frame_size;

        if(strncmp(frame_header, "TIT2", 4) == 0) data->title = content;
        else if(strncmp(frame_header, "TPE1", 4) == 0) data->artist = content;
        else if(strncmp(frame_header, "TALB", 4) == 0) data->album = content;
        else if(strncmp(frame_header, "TYER", 4) == 0) data->year = content;
        else if(strncmp(frame_header, "TCON", 4) == 0) data->genre = content;
        else if(strncmp(frame_header, "COMM", 4) == 0) data->comment = content;
        else free(content);
    }

    fclose(fp);
    return data;
}

/**
TODO: Add documention as sample given
 */
void display_metadata(const TagData *data) {
    if(data == NULL) return;
    printf("Mp3 Tag Reader & Editor:\n");
    printf("---------------------\n");

    printf("Version ID : 2.%d\n", data->tag_version);
    if (data->title)   printf("Title      : %s\n", data->title + 1);
    if (data->artist)  printf("Artist     : %s\n", data->artist + 1);
    if (data->album)   printf("Album      : %s\n", data->album + 1);
    if (data->year)    printf("Year       : %s\n", data->year + 1);
    if (data->genre)   printf("Genre      : %s\n", data->genre + 1);
    if (data->comment) printf("Comment    : %s\n", data->comment + 1);
}

/**
TODO: Add documention as sample given
 */
void view_tags(const char *filename) {
    TagData *data = read_id3_tags(filename);
    if (!data) {
        display_error("Failed to read ID3 tags.");
        return;
    }
    display_metadata(data);
    free_tag_data(data);
}
