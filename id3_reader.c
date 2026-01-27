/**
 * ============================================================================
 * @file id3_reader.c
 * @brief Implementation of functions for reading ID3 tags from MP3 files
 * 
 * This module handles the complete process of:
 *   - Opening and reading MP3 files
 *   - Parsing ID3v2 headers and frames
 *   - Extracting text metadata (title, artist, album, etc.)
 *   - Handling different text encodings (ISO-8859-1, UTF-16)
 *   - Displaying formatted metadata output
 * 
 * @details
 * ID3v2 Tag Structure:
 *   ┌────────────────────┐
 *   │ ID3v2 Header       │ 10 bytes
 *   ├────────────────────┤
 *   │ Frame 1 Header     │ 10 bytes
 *   │ Frame 1 Data       │ Variable
 *   ├────────────────────┤
 *   │ Frame 2 Header     │ 10 bytes
 *   │ Frame 2 Data       │ Variable
 *   ├────────────────────┤
 *   │ ...                │
 *   ├────────────────────┤
 *   │ Padding (optional) │ Variable
 *   └────────────────────┘
 * 
 * Supported ID3 Versions:
 *   - ID3v2.3 (most common)
 *   - ID3v2.4 (newer standard)
 * 
 * Key Differences Between Versions:
 *   - Frame size encoding:
 *     * v2.3: Standard big-endian integers
 *     * v2.4: Synchsafe integers
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3_reader.h"
#include "id3_utils.h"
#include "error_handling.h"

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Duplicates a string (custom implementation of strdup)
 * 
 * @details
 * This function provides a custom implementation of strdup() which may not
 * be available on all systems (it's POSIX but not C standard).
 * 
 * Process:
 *   1. Calculate length of source string (including null terminator)
 *   2. Allocate memory for destination
 *   3. Copy source to destination using memcpy
 *   4. Return pointer to new string
 * 
 * @param[in] src Source string to duplicate
 * 
 * @return Pointer to newly allocated duplicate string, or NULL if allocation fails
 * 
 * @note The caller is responsible for freeing the returned string
 */
char *my_strdup(const char *src) {
    /* Calculate length including null terminator */
    size_t len = strlen(src) + 1;
    
    /* Allocate memory for the duplicate */
    char *dst = malloc(len);
    
    /* If allocation succeeded, copy the string */
    if (dst) {
        memcpy(dst, src, len);
    }
    
    return dst;
}

/**
 * @brief Converts UTF-16 (LE) buffer to ISO-8859-1 (ASCII) in place
 * 
 * @details
 * This function fixes the "strlen stops at null byte" bug that occurs when
 * processing UTF-16 encoded text with standard C string functions.
 * 
 * UTF-16 Little Endian Structure:
 *   Byte 0:    0x01 (encoding flag)
 *   Bytes 1-2: 0xFF 0xFE (BOM - Byte Order Mark)
 *   Bytes 3+:  Character data: [char] [0x00] [char] [0x00]...
 * 
 * The Problem:
 *   - strlen() stops at the first 0x00 byte
 *   - UTF-16 has 0x00 bytes after each ASCII character
 *   - This causes incorrect length calculations
 * 
 * The Solution:
 *   - Create a new buffer
 *   - Extract only the character bytes (every other byte)
 *   - Set encoding to 0x00 (ISO-8859-1)
 *   - Replace the original buffer
 * 
 * @param[in,out] buffer Pointer to pointer to the UTF-16 buffer
 * @param[in]     size   Size of the original buffer in bytes
 * 
 * @note The original buffer is freed and the pointer is updated to point
 *       to the new ASCII buffer
 */
void convert_utf16_to_ascii(char **buffer, int size) {
    char *original = *buffer;
    
    /* Allocate new buffer for the clean ASCII string */
    char *ascii = malloc(size + 1);
    if (!ascii) {
        /* If allocation fails, leave original buffer unchanged */
        return;
    }
    
    /* Initialize write position
     * Start at index 1 because index 0 is reserved for encoding flag */
    int j = 1;
    
    /* Set encoding flag to 0x00 (ISO-8859-1) */
    ascii[0] = 0x00;

    /* Extract ASCII characters from UTF-16 buffer
     * 
     * Skip: Index 0 (Old encoding flag)
     * Skip: Index 1-2 (BOM bytes)
     * Start: Index 3 (actual text begins here)
     * 
     * Process every 2nd byte (Little Endian: [char] [0x00] pattern) */
    for (int i = 3; i < size; i += 2) {
        /* Only copy non-null bytes (ignore the 0x00 padding) */
        if (original[i] != 0) {
            ascii[j++] = original[i];
        }
    }
    
    /* Null-terminate the new ASCII string */
    ascii[j] = '\0';
    
    /* Clean up: Free the old buffer and update the pointer */
    free(original);
    *buffer = ascii;
}

/* ============================================================================
 * PRIMARY READING FUNCTION
 * ============================================================================
 */

/**
 * @brief Reads and parses all ID3 tags from an MP3 file
 * 
 * @details
 * This function implements the complete ID3v2 tag parsing algorithm:
 * 
 * Algorithm Flow:
 *   ┌─────────────────────────────────┐
 *   │ 1. Open file and read header    │
 *   ├─────────────────────────────────┤
 *   │ 2. Validate ID3 signature        │
 *   ├─────────────────────────────────┤
 *   │ 3. Check version (v2.3 or v2.4) │
 *   ├─────────────────────────────────┤
 *   │ 4. Extract tag size              │
 *   ├─────────────────────────────────┤
 *   │ 5. Loop through frames:          │
 *   │    a. Read frame header          │
 *   │    b. Calculate frame size       │
 *   │    c. Read frame content         │
 *   │    d. Process encoding           │
 *   │    e. Store in TagData           │
 *   ├─────────────────────────────────┤
 *   │ 6. Handle padding                │
 *   ├─────────────────────────────────┤
 *   │ 7. Return TagData structure      │
 *   └─────────────────────────────────┘
 * 
 * @param[in] filename Path to the MP3 file
 * 
 * @return Pointer to TagData structure with parsed metadata, or NULL on failure
 */
TagData* read_id3_tags(const char *filename)
{
    /* =========================================================================
     * STEP 1: Open the file in binary read mode
     * ========================================================================= */
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        display_error("File Could Not Open");
        return NULL;
    }

    /* =========================================================================
     * STEP 2: Read the 10-byte ID3v2 header
     * 
     * Header Structure:
     *   Bytes 0-2: "ID3" (identifier)
     *   Byte 3:    Major version (e.g., 0x03 for v2.3)
     *   Byte 4:    Revision number
     *   Byte 5:    Flags
     *   Bytes 6-9: Tag size (synchsafe integer)
     * ========================================================================= */
    unsigned char header[10];
    fread(header, 1, 10, fp);
    
    /* Debug output: Display header flags for troubleshooting */
    printf("Debug: Header Flags: 0x%02X\n", header[5]);
    
    /* =========================================================================
     * STEP 3: Validate the ID3 signature
     * ========================================================================= */
    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3') {
        /* Not a valid ID3 tag */
        fclose(fp);
        return NULL;
    }
    
    printf("[Info]: ID3 Tag Found\n");
    
    /* =========================================================================
     * STEP 4: Create a new TagData structure to hold parsed information
     * ========================================================================= */
    TagData *data = create_tag_data();
    
    /* =========================================================================
     * STEP 5: Extract and validate version
     * ========================================================================= */
    data->tag_version = header[3];  /* Byte 3 contains major version */
    
    /* Verify we support this version (only 2.3 and 2.4) */
    if (data->tag_version != 0x03 && data->tag_version != 0x04) {
        display_error("Version not supported ");
        free_tag_data(data);
        return NULL;
    }

    /* =========================================================================
     * STEP 6: Extract total tag size from bytes 6-9
     * 
     * The tag size is stored as a synchsafe integer (28 usable bits)
     * This size does NOT include the 10-byte header itself
     * ========================================================================= */
    uint32_t tag_size = synchsafe_to_int(&header[6]);

    /* =========================================================================
     * STEP 7: Iterate through all frames in the tag
     * 
     * Continue until we've read all bytes or hit padding
     * ========================================================================= */
    int bytes_read = 0;
    
    while (bytes_read < tag_size) {
        /* ---------------------------------------------------------------------
         * Read the 10-byte frame header
         * 
         * Frame Header Structure:
         *   Bytes 0-3: Frame ID (e.g., "TIT2", "TPE1")
         *   Bytes 4-7: Frame size (4 bytes)
         *   Bytes 8-9: Frame flags (2 bytes)
         * --------------------------------------------------------------------- */
        unsigned char frame_header[10];
        if (fread(frame_header, 1, 10, fp) != 10) {
            /* End of file or read error */
            break;
        }
        bytes_read += 10;
        
        /* ---------------------------------------------------------------------
         * Check for padding (frame ID starts with null byte)
         * 
         * When we hit padding, remaining bytes are all zeros
         * --------------------------------------------------------------------- */
        if (frame_header[0] == 0) {
            /* Calculate remaining padding size
             * +10 accounts for the frame header we just read */
            data->padding_size = tag_size - bytes_read + 10;
            break;
        }

        /* ---------------------------------------------------------------------
         * Determine frame size based on ID3 version
         * 
         * ID3v2.3: Standard big-endian integer (bytes 4-7)
         * ID3v2.4: Synchsafe integer (bytes 4-7)
         * --------------------------------------------------------------------- */
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

        /* ---------------------------------------------------------------------
         * Allocate memory for frame content
         * 
         * +1 for null terminator (for text frames)
         * --------------------------------------------------------------------- */
        char *content = malloc(frame_size + 1);
        fread(content, 1, frame_size, fp);
        content[frame_size] = '\0';  /* Null-terminate the string */
        bytes_read += frame_size;

        /* ---------------------------------------------------------------------
         * UTF-16 SANITIZATION
         * 
         * If this is a text frame (starts with 'T' or is 'COMM') AND
         * the encoding byte is 0x01 (UTF-16), convert to ASCII
         * 
         * This fixes the strlen() bug where null bytes in UTF-16 cause
         * premature string termination
         * --------------------------------------------------------------------- */
        if ((frame_header[0] == 'T' || strncmp((char*)frame_header, "COMM", 4) == 0) 
             && content[0] == 0x01) {
            convert_utf16_to_ascii(&content, frame_size);
        }

        /* ---------------------------------------------------------------------
         * Match frame ID and store content in appropriate TagData field
         * 
         * Frame IDs are 4-character codes defined by ID3v2 specification
         * --------------------------------------------------------------------- */
        
        /* Standard text frames */
        if (strncmp((char*)frame_header, "TIT2", 4) == 0) {
            data->title = content;     /* Title/Song name */
        }
        else if (strncmp((char*)frame_header, "TPE1", 4) == 0) {
            data->artist = content;    /* Lead artist/performer */
        }
        else if (strncmp((char*)frame_header, "TALB", 4) == 0) {
            data->album = content;     /* Album/Movie/Show title */
        }
        else if (strncmp((char*)frame_header, "TYER", 4) == 0) {
            data->year = content;      /* Year (v2.3) */
        }
        else if (strncmp((char*)frame_header, "TRCK", 4) == 0) {
            data->track = content;     /* Track number */
        }
        else if (strncmp((char*)frame_header, "TCON", 4) == 0) {
            data->genre = content;     /* Content type/Genre */
        }
        else if (strncmp((char*)frame_header, "COMM", 4) == 0) {
            data->comment = content;   /* Comments */
        }
        
        /* Extended text frames */
        else if (strncmp((char*)frame_header, "TPE2", 4) == 0) {
            data->album_artist = content;  /* Band/Orchestra/Accompaniment */
        }
        else if (strncmp((char*)frame_header, "TCOM", 4) == 0) {
            data->composer = content;      /* Composer */
        }
        else if (strncmp((char*)frame_header, "TCOP", 4) == 0) {
            data->copyright = content;     /* Copyright message */
        }
        else if (strncmp((char*)frame_header, "TPUB", 4) == 0) {
            data->publisher = content;     /* Publisher */
        }
        
        /* Album artwork frame (binary data) */
        else if(strncmp((char*)frame_header, "APIC", 4) == 0){
            /* Store the size of the image data */
            data->album_art.image_size = frame_size;
            
            /* Transfer ownership of the buffer to the album_art structure */
            data->album_art.image_data = (unsigned char*)content;
            
            /* Create a string copy of the MIME type
             * Skip the encoding byte (content[0]) and copy from content[1] */
            data->album_art.mime_type = my_strdup(content + 1);
        }
        
        /* Unknown or unsupported frame */
        else {
            /* Free the memory since we're not storing this frame */
            free(content);
        }
    }

    /* =========================================================================
     * STEP 8: Close the file and return the populated TagData structure
     * ========================================================================= */
    fclose(fp);
    return data;
}

/* ============================================================================
 * DISPLAY FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Displays all metadata from a TagData structure in formatted output
 * 
 * @details
 * This function creates a user-friendly display of all ID3 tag information.
 * It handles two text encoding types:
 *   - 0x00: ISO-8859-1 (Latin-1) - printed directly
 *   - 0x01: UTF-16 with BOM - decoded using utf_16_decoder()
 * 
 * @param[in] data Pointer to TagData structure containing metadata
 */
void display_metadata(const TagData *data) {
    /* Guard against NULL pointer */
    if (data == NULL) {
        return;
    }
    
    /* =========================================================================
     * Print header section
     * ========================================================================= */
    printf("Mp3 Tag Reader & Editor:\n");
    printf("---------------------\n");
    printf("Version ID : 2.%d\n", data->tag_version);

    /* =========================================================================
     * Helper Macro: PRINT_TAG
     * 
     * This macro handles:
     *   1. Checking if the field exists (not NULL)
     *   2. Printing the label with consistent 10-character width
     *   3. Detecting encoding byte and choosing appropriate decoder
     * 
     * Logic:
     *   - If field is NULL, skip it
     *   - If field[0] == 0x01 → UTF-16, use decoder
     *   - If field[0] == 0x00 → ISO-8859-1, print directly (skip encoding byte)
     * ========================================================================= */
    #define PRINT_TAG(label, field) \
        if (field) { \
            printf("%-10s : ", label); \
            if (field[0] == 0x01) { \
                utf_16_decoder(field); \
            } else { \
                printf("%s\n", field + 1); \
            } \
        }

    /* =========================================================================
     * Display standard text tags
     * ========================================================================= */
    PRINT_TAG("Title",      data->title);
    PRINT_TAG("Artist",     data->artist);
    PRINT_TAG("Album",      data->album);
    PRINT_TAG("Year",       data->year);
    PRINT_TAG("Track",      data->track);
    PRINT_TAG("Genre",      data->genre);
    PRINT_TAG("Comment",    data->comment);
    
    /* =========================================================================
     * Display extended text tags
     * ========================================================================= */
    PRINT_TAG("Alb.Artist", data->album_artist);
    PRINT_TAG("Composer",   data->composer);
    PRINT_TAG("Copyright",  data->copyright);
    PRINT_TAG("Publisher",  data->publisher);

    /* Clean up macro definition (good practice) */
    #undef PRINT_TAG

    /* =========================================================================
     * Display album artwork information (if present)
     * ========================================================================= */
    if (data->album_art.image_data != NULL) {
        printf("Image Type : %s\n", data->album_art.mime_type ? data->album_art.mime_type : "Unknown");
        printf("Image Size : %u bytes\n", data->album_art.image_size);
    } else {
        printf("Image      : No embedded image found.\n");
    }
}

/**
 * @brief Convenience function to read and display tags in one operation
 * 
 * @details
 * This function combines reading and displaying tags, with automatic
 * memory cleanup. Ideal for simple "view tags" operations.
 * 
 * @param[in] filename Path to the MP3 file
 */
void view_tags(const char *filename) {
    /* Read the tags from the file */
    TagData *data = read_id3_tags(filename);
    
    /* Check if reading was successful */
    if (!data) {
        display_error("Failed to read ID3 tags.");
        return;
    }
    
    /* Display the metadata to the user */
    display_metadata(data);
    
    /* Clean up allocated memory */
    free_tag_data(data);
}

/* ============================================================================
 * TEXT ENCODING FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Decodes and prints UTF-16 Little Endian encoded text
 * 
 * @details
 * ID3 tags can use UTF-16LE encoding for international characters.
 * 
 * UTF-16LE Structure:
 *   Byte 0:    0x01 (encoding flag)
 *   Bytes 1-2: BOM (0xFF 0xFE for Little Endian)
 *   Bytes 3+:  Character data (2 bytes per character)
 * 
 * Decoding Process:
 *   - Start at index 3 (skip encoding flag and BOM)
 *   - Read every other byte (UTF-16LE: [char] [0x00] pattern for ASCII)
 *   - Print printable characters
 *   - Stop at double null (0x00 0x00)
 * 
 * @param[in] content Pointer to UTF-16 encoded string buffer
 * 
 * @note This is a simplified decoder that only handles ASCII characters
 *       within UTF-16. Full Unicode support would require proper UTF-16 decoding.
 * 
 * @note Terminates when double null (0x00 0x00) is encountered
 */
void utf_16_decoder(char *content)
{
    char *ptr = content;
    
    /* Start at index 3 to skip:
     *   - Index 0: Encoding flag (0x01)
     *   - Index 1-2: BOM (Byte Order Mark) */
    int i = 3;
    
    /* Loop until we hit a double null terminator (end of UTF-16 string)
     * Check both current byte and next byte for null */
    while (ptr[i] || ptr[i + 1]) {
        /* If the byte contains a printable ASCII character, output it
         * 
         * UTF-16LE stores ASCII as: [char, 0x00], so we read every 2nd byte
         * We only print if the character is non-null (ptr[i] > 0) */
        if (ptr[i] > 0) {
            printf("%c", ptr[i]);
        }
        
        /* Move to next character (skip 2 bytes in UTF-16) */
        i += 2;
    }
    
    /* Add newline after the decoded text */
    printf("\n");
}