/**
 * @file id3_writer.c
 * @brief Implementation of functions for writing and editing ID3 tags in MP3 files.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "id3_writer.h"
#include "id3_reader.h"
#include "id3_utils.h"
#include "error_handling.h"

/**
 * @brief Writes the ID3 tags to an MP3 file.
 * 
 * @param filename The name of the MP3 file.
 * @param data Pointer to the TagData structure containing the ID3 tags.
 * @return 0 on success, non-zero on failure.
 */

uint32_t calculate_new_tag_size(const TagData *data) {
    uint32_t size = 0;

    /* Logic: 10 (Header) + 1 (Encoding Byte) + Text Length */
    /* We check 'data->title + 1' to measure the text, ignoring the 0x00 prefix */
    if (data->title)   size += 10 + 1 + strlen(data->title + 1);
    if (data->artist)  size += 10 + 1 + strlen(data->artist + 1);
    if (data->album)   size += 10 + 1 + strlen(data->album + 1);
    if (data->year)    size += 10 + 1 + strlen(data->year + 1);
    if (data->genre)   size += 10 + 1 + strlen(data->genre + 1);
    if (data->comment) size += 10 + 1 + strlen(data->comment + 1);

    size += 1024; // Padding
    size += 10;   // Header
    return size;
}

int write_id3_tags(const char *filename, const TagData *data)
{
    uint32_t new_tag_size = calculate_new_tag_size(data);
    FILE *fp_src = fopen(filename, "rb");
    FILE *fp_temp = fopen("temp.mp3", "wb");
    if(fp_src == NULL || fp_temp == NULL)
    {
        display_error("File Could Not open");
        if(fp_src) fclose(fp_src);
        if(fp_temp) fclose(fp_temp);
        return 1;
    }
    fwrite("ID3", 1, 3, fp_temp);
    uint8_t ver[2] = {0x04, 0x00};
    fwrite(ver, 1 , 2, fp_temp);
    uint8_t flags = 0;
    fwrite(&flags, 1, 1, fp_temp);
    uint8_t size_bytes[4];
    int_to_synchsafe(new_tag_size - 10, size_bytes);
    fwrite(size_bytes, 1, 4, fp_temp);
    
    char *frame_ids[] = {"TIT2", "TPE1", "TALB", "TYER", "TCON", "COMM"};
    char *values[] = {data->title, data->artist, data->album, data->year, data->genre, data->comment};

    for(int i = 0; i < 6; i++)
    {
        if(values[i] != NULL)
        {
            fwrite(frame_ids[i], 1, 4, fp_temp);
            
            /* FIXED: Skip the [0x00] byte to count text, then add 1 back */
            uint32_t frame_len = 1 + strlen(values[i] + 1);
            
            int_to_synchsafe(frame_len, size_bytes);
            fwrite(size_bytes, 1, 4, fp_temp);
            
            uint8_t frame_flags[2] = {0,0};
            fwrite(frame_flags, 1, 2, fp_temp);
            
            /* Write the full buffer (Encoding + Text) */
            fwrite(values[i], 1, frame_len, fp_temp);
        }
    }

    uint8_t null_byte = 0;
    for(int i = 0; i < 1024; i++)
    {
        fwrite(&null_byte, 1, 1, fp_temp);
    }

    unsigned char old_header[10];
    if(fread(old_header, 1, 10, fp_src) == 10)
    {
        uint32_t old_tag_size = synchsafe_to_int(&old_header[6]);
        
        fseek(fp_src, old_tag_size + 10, SEEK_SET);
        char buffer[4096];
        size_t n;
        while((n = fread(buffer, 1, sizeof(buffer), fp_src)) > 0)
        {
            fwrite(buffer, 1, n, fp_temp);
        }
    }

    fclose(fp_src);
    fclose(fp_temp);

    remove(filename);
    rename("temp.mp3", filename);
    return 0;
}

/** TODO: Add documentation as sample given above */
int edit_tag(const char *filename, const char *tag, const char *value)
{
    TagData *data = read_id3_tags(filename);
    if (!data) return 1;

    char **field_ptr = NULL;
    /* Map option to struct field */
    if (strcmp(tag, "-t") == 0)      field_ptr = &data->title;
    else if (strcmp(tag, "-a") == 0) field_ptr = &data->artist;
    else if (strcmp(tag, "-A") == 0) field_ptr = &data->album;
    else if (strcmp(tag, "-y") == 0) field_ptr = &data->year;
    else if (strcmp(tag, "-c") == 0) field_ptr = &data->comment;
    else if (strcmp(tag, "-g") == 0) field_ptr = &data->genre;
    else {
        display_error("Unknown Tag Option");
        free_tag_data(data);
        return 1;
    }

    /* ALLOCATE: Length + 1 (Encoding) + 1 (Null Terminator) */
    int len = strlen(value);
    char *new_content = malloc(len + 2); 
    if (new_content == NULL) {
        display_error("Memory allocation failed");
        free_tag_data(data);
        return 1;
    }

    /* FORMAT: [0x00] [Text...] [\0] */
    new_content[0] = 0x00;          // <--- FIXED: Set the first byte, don't null the pointer
    strcpy(new_content + 1, value); // Copy text after the encoding byte

    /* UPDATE STRUCT */
    if (*field_ptr != NULL) free(*field_ptr);
    *field_ptr = new_content;

    /* WRITE */
    if (write_id3_tags(filename, data) != 0) {
        free_tag_data(data);
        return 1;
    }

    printf("Tag Edited Successfully.\n");
    free_tag_data(data);
    return 0;
}
