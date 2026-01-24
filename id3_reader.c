/**
 * @file id3_reader.c
 * @brief Implementation of functions for reading ID3 tags from MP3 files.
 * 
 * This module handles the complete process of:
 * - Opening and reading MP3 files
 * - Parsing ID3v2 headers and frames
 * - Extracting text metadata (title, artist, album, etc.)
 * - Handling different text encodings (ISO-8859-1, UTF-16)
 * - Displaying formatted metadata output
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3_reader.h"
#include "id3_utils.h"
#include "error_handling.h"

/**
 * @brief Reads and parses all ID3 tags from an MP3 file.
 * 
 * This function implements the complete ID3v2 tag parsing algorithm:
 * 
 * 1. File validation and header reading
 * 2. Version detection (v2.3 or v2.4)
 * 3. Frame-by-frame iteration
 * 4. Content extraction and storage
 * 
 * Frame size calculation differs by version:
 * - ID3v2.3: Uses standard big-endian integers
 * - ID3v2.4: Uses synchsafe integers
 * 
 * @param filename Path to the MP3 file.
 * @return Pointer to TagData structure with parsed metadata, or NULL on failure.
 */
TagData* read_id3_tags(const char *filename)
{
    /* Attempt to open the file in binary read mode */
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        display_error("File Could Not Open");
        return NULL;
    }

    /* Read the 10-byte ID3 header */
    unsigned char header[10];
    fread(header, 1, 10, fp);
    
    /* Debug output: Display header flags for troubleshooting */
    printf("Debug: Header Flags: 0x%02X\n", header[5]);
    
    /* Validate the ID3 signature ("ID3") */
    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3') {
        fclose(fp);
        return NULL;
    }
    
    printf("[Info]: ID3 Tag Found\n");
    
    /* Create a new TagData structure to hold parsed information */
    TagData *data = create_tag_data();
    
    /* Extract version from header (byte 3) */
    data->tag_version = header[3];
    
    /* Verify we support this version (only 2.3 and 2.4) */
    if (data->tag_version != 0x03 && data->tag_version != 0x04) {
        display_error("Version not supported ");
        free_tag_data(data);
        return NULL;
    }

    /* Extract total tag size from bytes 6-9 (synchsafe integer) */
    uint32_t tag_size = synchsafe_to_int(&header[6]);

    /* Iterate through all frames in the tag */
    int bytes_read = 0;
    while (bytes_read < tag_size) {
        /* Read the 10-byte frame header */
        unsigned char frame_header[10];
        if (fread(frame_header, 1, 10, fp) != 10) {
            /* End of file or read error */
            break;
        }
        bytes_read += 10;
        
        /* Check for padding (frame ID starts with null byte) */
        if (frame_header[0] == 0) {
            /* Calculate remaining padding size */
            data->padding_size = tag_size - bytes_read + 10;
            break;
        }

        /* Determine frame size based on ID3 version */
        uint32_t frame_size;
        if (data->tag_version == 3) {
            /* ID3v2.3: Standard big-endian integer */
            frame_size = big_endian_to_host(&frame_header[4]);
        }
        else if (data->tag_version == 4) {
            /* ID3v2.4: Synchsafe integer */
            frame_size = synchsafe_to_int(&frame_header[4]);
        }
        
        /* Debug output: Show which frame is being processed */
        printf("Debug: Found Frame ID: %.4s, Size: %d\n", frame_header, frame_size);

        /* Allocate memory for frame content (+ 1 for null terminator) */
        char *content = malloc(frame_size + 1);
        fread(content, 1, frame_size, fp);
        content[frame_size] = '\0';  /* Null-terminate the string */
        bytes_read += frame_size;

        /* Match frame ID and store content in appropriate field */
        /* Frame IDs are 4-character codes defined by ID3v2 spec */
        if (strncmp(frame_header, "TIT2", 4) == 0) {
            data->title = content;     /* Title/Song name */
        }
        else if (strncmp(frame_header, "TPE1", 4) == 0) {
            data->artist = content;    /* Lead artist/performer */
        }
        else if (strncmp(frame_header, "TALB", 4) == 0) {
            data->album = content;     /* Album/Movie/Show title */
        }
        else if (strncmp(frame_header, "TYER", 4) == 0) {
            data->year = content;      /* Year (v2.3) */
        }
        else if (strncmp(frame_header, "TCON", 4) == 0) {
            data->genre = content;     /* Content type/Genre */
        }
        else if (strncmp(frame_header, "COMM", 4) == 0) {
            data->comment = content;   /* Comments */
        }
        else {
            /* Unrecognized or unsupported frame - free the memory */
            free(content);
        }
    }

    fclose(fp);
    return data;
}

/**
 * @brief Displays all metadata from a TagData structure in formatted output.
 * 
 * This function creates a user-friendly display of all ID3 tag information.
 * It handles two text encoding types:
 * - 0x00: ISO-8859-1 (Latin-1) - printed directly
 * - 0x01: UTF-16 with BOM - decoded using utf_16_decoder()
 * 
 * @param data Pointer to TagData structure containing metadata.
 */
void display_metadata(const TagData *data) {
    if (data == NULL) return;
    
    /* Print header */
    printf("Mp3 Tag Reader & Editor:\n");
    printf("---------------------\n");

    /* Display ID3 version */
    printf("Version ID : 2.%d\n", data->tag_version);

    /* Title - check encoding byte (first byte of content) */
    if (data->title && data->title[0] == 0x01) {
        /* UTF-16 encoded */
        printf("Title      : ");
        utf_16_decoder(data->title);
    } else if (data->title) {
        /* ISO-8859-1 encoded (skip encoding byte at index 0) */
        printf("Title      : %s\n", data->title + 1);
    }

    /* Artist */
    if (data->artist && data->artist[0] == 0x01) {
        printf("Artist     : ");
        utf_16_decoder(data->artist);
    } else if (data->artist) {
        printf("Artist     : %s\n", data->artist + 1);
    }

    /* Album */
    if (data->album && data->album[0] == 0x01) {
        printf("Album      : ");
        utf_16_decoder(data->album);
    } else if (data->album) {
        printf("Album      : %s\n", data->album + 1);
    }

    /* Year */
    if (data->year && data->year[0] == 0x01) {
        printf("Year       : ");
        utf_16_decoder(data->year);
    } else if (data->year) {
        printf("Year       : %s\n", data->year + 1);
    }

    /* Genre */
    if (data->genre && data->genre[0] == 0x01) {
        printf("Genre      : ");
        utf_16_decoder(data->genre);
    } else if (data->genre) {
        printf("Genre      : %s\n", data->genre + 1);
    }

    /* Comment */
    if (data->comment && data->comment[0] == 0x01) {
        printf("Comment    : ");
        utf_16_decoder(data->comment);
    } else if (data->comment) {
        printf("Comment    : %s\n", data->comment + 1);
    }
}

/**
 * @brief Convenience function to read and display tags in one operation.
 * 
 * This function combines reading and displaying tags, with automatic
 * memory cleanup. Ideal for simple "view tags" operations.
 * 
 * @param filename Path to the MP3 file.
 */
void view_tags(const char *filename) {
    /* Read the tags */
    TagData *data = read_id3_tags(filename);
    if (!data) {
        display_error("Failed to read ID3 tags.");
        return;
    }
    
    /* Display the metadata */
    display_metadata(data);
    
    /* Clean up */
    free_tag_data(data);
}

/**
 * @brief Decodes and prints UTF-16 Little Endian encoded text.
 * 
 * ID3 tags can use UTF-16LE encoding for international characters.
 * Structure:
 * - Byte 0: 0x01 (encoding flag)
 * - Bytes 1-2: BOM (0xFF 0xFE for Little Endian)
 * - Bytes 3+: Character data (2 bytes per character)
 * 
 * This implementation extracts ASCII characters by reading every other byte,
 * starting from index 3 (after encoding flag and BOM).
 * 
 * @param content Pointer to UTF-16 encoded string buffer.
 * 
 * @note Terminates when double null (0x00 0x00) is encountered.
 */
void utf_16_decoder(char *content)
{
    char *ptr = content;
    
    /* Start at index 3 to skip:
     * - Index 0: Encoding flag (0x01)
     * - Index 1-2: BOM (Byte Order Mark) */
    int i = 3;
    
    /* Loop until we hit a double null terminator (end of UTF-16 string) */
    /* Note: This assumes frame size is accurate */
    while (ptr[i] || ptr[i + 1]) {
        /* If the byte contains a printable ASCII character, output it */
        /* UTF-16LE stores ASCII as: [char, 0x00], so we read every 2nd byte */
        if (ptr[i] > 0) {
            printf("%c", ptr[i]);
        }
        
        /* Move to next character (skip 2 bytes) */
        i += 2;
    }
    printf("\n");
}