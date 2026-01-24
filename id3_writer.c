/**
 * @file id3_writer.c
 * @brief Implementation of functions for writing and editing ID3 tags in MP3 files.
 * 
 * This module handles the complete process of:
 * - Calculating new tag sizes
 * - Writing ID3v2.4 headers and frames
 * - Preserving audio data during tag updates
 * - Managing temporary files for safe updates
 * - Providing high-level editing functions
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "id3_writer.h"
#include "id3_reader.h"
#include "id3_utils.h"
#include "error_handling.h"

/**
 * @brief Calculates the total size needed for the new ID3 tag.
 * 
 * Computes the complete tag size including:
 * - Frame headers (10 bytes each)
 * - Encoding bytes (1 byte per frame)
 * - Text content (variable length)
 * - Padding (1024 bytes for future edits)
 * - ID3 header (10 bytes)
 * 
 * For each text frame:
 * Size = 10 (frame header) + 1 (encoding) + strlen(text)
 * 
 * Note: The text length is measured from (data->field + 1) because
 * the stored strings have a 0x00 encoding byte prefix that we skip.
 * 
 * @param data Pointer to TagData structure containing the metadata.
 * @return Total size in bytes needed for the complete tag.
 */
uint32_t calculate_new_tag_size(const TagData *data) {
    uint32_t size = 0;

    /* Calculate size for each text frame if present */
    /* Format: 10 (Frame Header) + 1 (Encoding Byte) + Text Length */
    /* We check 'data->field + 1' to skip the 0x00 prefix and measure actual text */
    if (data->title)   size += 10 + 1 + strlen(data->title + 1);
    if (data->artist)  size += 10 + 1 + strlen(data->artist + 1);
    if (data->album)   size += 10 + 1 + strlen(data->album + 1);
    if (data->year)    size += 10 + 1 + strlen(data->year + 1);
    if (data->genre)   size += 10 + 1 + strlen(data->genre + 1);
    if (data->comment) size += 10 + 1 + strlen(data->comment + 1);

    /* Add padding for future edits without file resize */
    size += 1024;
    
    /* Add ID3 header size */
    size += 10;
    
    return size;
}

/**
 * @brief Writes the ID3 tags to an MP3 file.
 * 
 * Complete tag writing process:
 * 
 * 1. Calculate new tag size
 * 2. Open source file and create temporary file
 * 3. Write ID3v2.4 header to temp file
 * 4. Write all text frames to temp file
 * 5. Add padding bytes
 * 6. Copy audio data from source to temp
 * 7. Replace original file with temp file
 * 
 * The ID3v2.4 header structure:
 * - Bytes 0-2: "ID3"
 * - Bytes 3-4: Version (0x04 0x00 for v2.4)
 * - Byte 5: Flags (0x00 = no flags)
 * - Bytes 6-9: Tag size (synchsafe integer, excluding header)
 * 
 * @param filename The name of the MP3 file.
 * @param data Pointer to the TagData structure containing the ID3 tags.
 * @return 0 on success, non-zero on failure.
 */
int write_id3_tags(const char *filename, const TagData *data)
{
    /* Calculate the total size needed for the new tag */
    uint32_t new_tag_size = calculate_new_tag_size(data);
    
    /* Open source file for reading and temp file for writing */
    FILE *fp_src = fopen(filename, "rb");
    FILE *fp_temp = fopen("temp.mp3", "wb");
    
    if (fp_src == NULL || fp_temp == NULL) {
        display_error("File Could Not open");
        if (fp_src) fclose(fp_src);
        if (fp_temp) fclose(fp_temp);
        return 1;
    }
    
    /* ===== Write ID3v2.4 Header ===== */
    
    /* Write file identifier "ID3" */
    fwrite("ID3", 1, 3, fp_temp);
    
    /* Write version: 2.4 (0x04 0x00) */
    uint8_t ver[2] = {0x04, 0x00};
    fwrite(ver, 1, 2, fp_temp);
    
    /* Write flags byte (no flags set) */
    uint8_t flags = 0;
    fwrite(&flags, 1, 1, fp_temp);
    
    /* Write tag size (excluding 10-byte header) as synchsafe integer */
    uint8_t size_bytes[4];
    int_to_synchsafe(new_tag_size - 10, size_bytes);
    fwrite(size_bytes, 1, 4, fp_temp);
    
    /* ===== Write Text Frames ===== */
    
    /* Define frame IDs and corresponding data fields */
    char *frame_ids[] = {"TIT2", "TPE1", "TALB", "TYER", "TCON", "COMM"};
    char *values[] = {data->title, data->artist, data->album, 
                      data->year, data->genre, data->comment};

    /* Iterate through each frame type */
    for (int i = 0; i < 6; i++) {
        if (values[i] != NULL) {
            /* Write 4-byte frame ID */
            fwrite(frame_ids[i], 1, 4, fp_temp);
            
            /* Calculate frame content size: 1 (encoding) + text length */
            /* Skip the [0x00] byte at start to count only the text */
            uint32_t frame_len = 1 + strlen(values[i] + 1);
            
            /* Write frame size as synchsafe integer */
            int_to_synchsafe(frame_len, size_bytes);
            fwrite(size_bytes, 1, 4, fp_temp);
            
            /* Write frame flags (no flags) */
            uint8_t frame_flags[2] = {0, 0};
            fwrite(frame_flags, 1, 2, fp_temp);
            
            /* Write frame content (encoding byte + text) */
            fwrite(values[i], 1, frame_len, fp_temp);
        }
    }

    /* ===== Write Padding ===== */
    
    /* Add 1024 null bytes for padding (allows future edits) */
    uint8_t null_byte = 0;
    for (int i = 0; i < 1024; i++) {
        fwrite(&null_byte, 1, 1, fp_temp);
    }

    /* ===== Copy Audio Data ===== */
    
    /* Read original file's ID3 header to find where audio starts */
    unsigned char old_header[10];
    if (fread(old_header, 1, 10, fp_src) == 10) {
        /* Extract old tag size from header */
        uint32_t old_tag_size = synchsafe_to_int(&old_header[6]);
        
        /* Seek to start of audio data (after old tag) */
        fseek(fp_src, old_tag_size + 10, SEEK_SET);
        
        /* Copy audio data in chunks */
        char buffer[4096];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), fp_src)) > 0) {
            fwrite(buffer, 1, n, fp_temp);
        }
    }

    /* Close both files */
    fclose(fp_src);
    fclose(fp_temp);

    /* Replace original file with updated version */
    remove(filename);
    rename("temp.mp3", filename);
    
    return 0;
}

/**
 * @brief Edits a specific tag field in an MP3 file.
 * 
 * Workflow:
 * 1. Read current tags from the file
 * 2. Map the tag option to the correct field
 * 3. Allocate memory for new value with encoding prefix
 * 4. Update the field in the TagData structure
 * 5. Write all tags back to the file
 * 
 * The new value is stored with the format: [0x00][text][\0]
 * - 0x00: ISO-8859-1 encoding indicator
 * - text: The actual text content
 * - \0: Null terminator
 * 
 * @param filename Path to the MP3 file to edit.
 * @param tag Tag option ("-t", "-a", "-A", "-y", "-c", "-g").
 * @param value New text value for the tag (without encoding prefix).
 * @return 0 on success, non-zero on failure.
 */
int edit_tag(const char *filename, const char *tag, const char *value)
{
    /* Read current tags from the file */
    TagData *data = read_id3_tags(filename);
    if (!data) return 1;

    /* Pointer to the field we'll update */
    char **field_ptr = NULL;
    
    /* Map command-line option to struct field */
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

    /* Allocate memory for new content */
    /* Size: text length + 1 (encoding byte) + 1 (null terminator) */
    int len = strlen(value);
    char *new_content = malloc(len + 2);
    
    if (new_content == NULL) {
        display_error("Memory allocation failed");
        free_tag_data(data);
        return 1;
    }

    /* Format the new content: [0x00][text][\0] */
    new_content[0] = 0x00;              /* Set encoding to ISO-8859-1 */
    strcpy(new_content + 1, value);     /* Copy text after encoding byte */

    /* Update the field in the TagData structure */
    if (*field_ptr != NULL) {
        free(*field_ptr);  /* Free old value if it exists */
    }
    *field_ptr = new_content;

    /* Write updated tags back to file */
    if (write_id3_tags(filename, data) != 0) {
        free_tag_data(data);
        return 1;
    }

    printf("Tag Edited Successfully.\n");
    free_tag_data(data);
    return 0;
}