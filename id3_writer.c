/**
 * ============================================================================
 * @file id3_writer.c
 * @brief Implementation of functions for writing and editing ID3 tags in MP3 files
 * 
 * This module handles the complete process of:
 *   - Calculating new tag sizes
 *   - Writing ID3v2.4 headers and frames
 *   - Preserving audio data during tag updates
 *   - Managing temporary files for safe updates
 *   - Providing high-level editing functions
 * 
 * @details
 * Write Strategy:
 *   All write operations use temporary files to ensure data safety:
 *   1. Create temp.mp3 with new tags
 *   2. Copy audio data from original
 *   3. Replace original only if successful
 *   4. This prevents corruption if write fails mid-operation
 * 
 * Tag Format:
 *   - All tags written in ID3v2.4 format
 *   - Frame sizes use synchsafe integers
 *   - Text encoding defaults to ISO-8859-1 (0x00)
 *   - 1024 bytes of padding added for future edits
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "id3_writer.h"
#include "id3_reader.h"
#include "id3_utils.h"
#include "error_handling.h"

/* ============================================================================
 * SIZE CALCULATION FUNCTION
 * ============================================================================
 */

/**
 * @brief Calculates the total size needed for the new ID3 tag
 * 
 * @details
 * Computes the complete tag size including all components:
 * 
 * Component Size Breakdown:
 *   ┌─────────────────────┬──────────┬──────────────────────────┐
 *   │ Component           │ Size     │ Formula                  │
 *   ├─────────────────────┼──────────┼──────────────────────────┤
 *   │ Frame Header        │ 10 bytes │ Per frame                │
 *   │ Encoding Byte       │ 1 byte   │ Per text frame           │
 *   │ Text Content        │ Variable │ strlen(field + 1)        │
 *   │ APIC Frame          │ Variable │ Complex (see below)      │
 *   │ Padding             │ 1024 b   │ Fixed                    │
 *   │ ID3 Header          │ 10 bytes │ Fixed                    │
 *   └─────────────────────┴──────────┴──────────────────────────┘
 * 
 * Text Frame Size Calculation:
 *   For each text frame:
 *     Total = 10 (header) + 1 (encoding) + strlen(text)
 * 
 *   Why strlen(field + 1)?
 *     - Stored strings have format: [0x00][text][\0]
 *     - We skip the encoding byte (field[0])
 *     - We measure the actual text (field + 1)
 * 
 * APIC Frame Size Calculation:
 *   If album art exists:
 *     Total = 10 (header) + 1 (encoding) + strlen(mime) + 1 (null) +
 *             1 (pic type) + 1 (desc null) + image_size
 * 
 * @param[in] data Pointer to TagData structure containing the metadata
 * 
 * @return Total size in bytes needed for the complete tag
 * 
 * @note The returned size includes the 10-byte ID3 header
 */
uint32_t calculate_new_tag_size(const TagData *data) {
    uint32_t size = 0;

    /* =========================================================================
     * Helper Macro: ADD_SIZE
     * 
     * For each text field that exists:
     *   - Add 10 bytes for frame header
     *   - Add 1 byte for encoding flag
     *   - Add strlen(field + 1) for actual text length
     *     (skip the encoding byte at field[0])
     * ========================================================================= */
    #define ADD_SIZE(field) \
        if(field) { \
            size += 10 + 1 + strlen(field + 1); \
        }

    /* =========================================================================
     * Calculate size for standard text frames
     * ========================================================================= */
    ADD_SIZE(data->title);        /* TIT2 - Title */
    ADD_SIZE(data->artist);       /* TPE1 - Artist */
    ADD_SIZE(data->album);        /* TALB - Album */
    ADD_SIZE(data->year);         /* TYER - Year */
    ADD_SIZE(data->genre);        /* TCON - Genre */
    ADD_SIZE(data->comment);      /* COMM - Comment */
    
    /* =========================================================================
     * Calculate size for extended text frames
     * ========================================================================= */
    ADD_SIZE(data->track);        /* TRCK - Track number */
    ADD_SIZE(data->album_artist); /* TPE2 - Album artist */
    ADD_SIZE(data->composer);     /* TCOM - Composer */
    ADD_SIZE(data->copyright);    /* TCOP - Copyright */
    ADD_SIZE(data->publisher);    /* TPUB - Publisher */

    /* Clean up macro */
    #undef ADD_SIZE

    /* =========================================================================
     * Calculate size for APIC (album art) frame if present
     * 
     * APIC Frame Structure:
     *   - Frame header: 10 bytes
     *   - Encoding byte: 1 byte
     *   - MIME type: variable + 1 (null terminator)
     *   - Picture type: 1 byte
     *   - Description: 1 byte (empty description = just null)
     *   - Image data: image_size bytes
     * ========================================================================= */
    if(data->album_art.image_data) {
        const char *mime = data->album_art.mime_type ? 
                          data->album_art.mime_type : "image/jpeg";
        
        size += 10;                         /* Frame header */
        size += 1;                          /* Encoding byte */
        size += strlen(mime) + 1;           /* MIME type + null */
        size += 1;                          /* Picture type */
        size += 1;                          /* Description null terminator */
        size += data->album_art.image_size; /* Image data */
    }
    
    /* =========================================================================
     * Add padding and header size
     * 
     * Padding: 1024 bytes
     *   - Allows future edits without complete rewrite
     *   - Standard practice in ID3 tag writing
     * 
     * Header: 10 bytes
     *   - ID3v2.4 header that precedes all frames
     * ========================================================================= */
    size += 1024;  /* Padding */
    size += 10;    /* ID3 header itself */
    
    return size;
}

/* ============================================================================
 * PRIMARY WRITING FUNCTION
 * ============================================================================
 */

/**
 * @brief Writes the ID3 tags to an MP3 file
 * 
 * @details
 * This function implements the complete tag writing algorithm:
 * 
 * Algorithm Steps:
 *   ┌──────────────────────────────────────┐
 *   │ 1. Calculate new tag size            │
 *   ├──────────────────────────────────────┤
 *   │ 2. Open files (source + temp)        │
 *   ├──────────────────────────────────────┤
 *   │ 3. Write ID3v2.4 header              │
 *   │    - "ID3" signature                 │
 *   │    - Version bytes (0x04 0x00)       │
 *   │    - Flags (0x00)                    │
 *   │    - Tag size (synchsafe)            │
 *   ├──────────────────────────────────────┤
 *   │ 4. Write all text frames             │
 *   │    (only non-NULL fields)            │
 *   ├──────────────────────────────────────┤
 *   │ 5. Write APIC frame (if exists)      │
 *   ├──────────────────────────────────────┤
 *   │ 6. Write padding (1024 null bytes)   │
 *   ├──────────────────────────────────────┤
 *   │ 7. Copy audio data from original     │
 *   ├──────────────────────────────────────┤
 *   │ 8. Replace original with temp        │
 *   └──────────────────────────────────────┘
 * 
 * ID3v2.4 Header Format (10 bytes):
 *   Bytes 0-2: "ID3"        (File identifier)
 *   Byte 3:    0x04         (Major version = 4)
 *   Byte 4:    0x00         (Revision = 0)
 *   Byte 5:    0x00         (Flags = none)
 *   Bytes 6-9: tag_size     (Synchsafe integer, excludes header)
 * 
 * @param[in] filename The name of the MP3 file
 * @param[in] data     Pointer to the TagData structure containing the ID3 tags
 * 
 * @return 0 on success, non-zero on failure
 */
int write_id3_tags(const char *filename, const TagData *data)
{
    /* =========================================================================
     * STEP 1: Calculate the total size needed for the new tag
     * ========================================================================= */
    uint32_t new_tag_size = calculate_new_tag_size(data);
    
    /* =========================================================================
     * STEP 2: Open source file for reading and temp file for writing
     * ========================================================================= */
    FILE *fp_src = fopen(filename, "rb");
    FILE *fp_temp = fopen("temp.mp3", "wb");
    
    /* Validate both files opened successfully */
    if (fp_src == NULL || fp_temp == NULL) {
        display_error("File Could Not open");
        if (fp_src) fclose(fp_src);
        if (fp_temp) fclose(fp_temp);
        return 1;
    }
    
    /* =========================================================================
     * STEP 3: Write ID3v2.4 Header (10 bytes)
     * ========================================================================= */
    
    /* Bytes 0-2: "ID3" identifier */
    fwrite("ID3", 1, 3, fp_temp);
    
    /* Bytes 3-4: Version (0x04 0x00 for ID3v2.4) */
    uint8_t ver[2] = {0x04, 0x00};
    fwrite(ver, 1, 2, fp_temp);
    
    /* Byte 5: Flags (0x00 = no flags set) */
    uint8_t flags = 0;
    fwrite(&flags, 1, 1, fp_temp);
    
    /* Bytes 6-9: Tag size (synchsafe integer)
     * Note: Subtract 10 because size excludes the header itself */
    uint8_t size_bytes[4];
    int_to_synchsafe(new_tag_size - 10, size_bytes);
    fwrite(size_bytes, 1, 4, fp_temp);
    
    /* =========================================================================
     * STEP 4: Write Text Frames
     * 
     * For each non-NULL text field, write:
     *   1. Frame header (10 bytes)
     *   2. Frame content (encoding byte + text)
     * ========================================================================= */
    
    /* Define frame IDs and their corresponding values */
    char *frame_ids[] = {
        "TIT2",  /* Title */
        "TPE1",  /* Artist */
        "TALB",  /* Album */
        "TYER",  /* Year */
        "TCON",  /* Genre */
        "COMM",  /* Comment */
        "TRCK",  /* Track */
        "TPE2",  /* Album Artist */
        "TCOM",  /* Composer */
        "TCOP",  /* Copyright */
        "TPUB"   /* Publisher */
    };
    
    char *values[] = {
        data->title,
        data->artist,
        data->album,
        data->year,
        data->genre,
        data->comment,
        data->track,
        data->album_artist,
        data->composer,
        data->copyright,
        data->publisher
    };

    /* Write each frame (only if value is non-NULL) */
    for (int i = 0; i < 11; i++) {
        if (values[i] != NULL) {
            /* -----------------------------------------------------------------
             * Write Frame Header (10 bytes)
             * ----------------------------------------------------------------- */
            
            /* Bytes 0-3: Frame ID (e.g., "TIT2") */
            fwrite(frame_ids[i], 1, 4, fp_temp);
            
            /* Bytes 4-7: Frame size (synchsafe integer)
             * Size = 1 (encoding byte) + strlen(text) */
            uint32_t frame_len = 1 + strlen(values[i] + 1);
            int_to_synchsafe(frame_len, size_bytes);
            fwrite(size_bytes, 1, 4, fp_temp);
            
            /* Bytes 8-9: Frame flags (0x00 0x00 = no flags) */
            uint8_t frame_flags[2] = {0, 0};
            fwrite(frame_flags, 1, 2, fp_temp);
            
            /* -----------------------------------------------------------------
             * Write Frame Content
             * 
             * Format: [encoding byte][text]
             * The encoding byte is already in values[i][0]
             * ----------------------------------------------------------------- */
            fwrite(values[i], 1, frame_len, fp_temp);
        }
    }

    /* =========================================================================
     * STEP 5: Write Album Art Frame (APIC)
     * 
     * Only write if image data exists and size is valid
     * ========================================================================= */
    if (data->album_art.image_data != NULL && data->album_art.image_size > 0) {
        
        /* ---------------------------------------------------------------------
         * Write APIC Frame Header (10 bytes)
         * --------------------------------------------------------------------- */
        
        /* Bytes 0-3: Frame ID "APIC" */
        fwrite("APIC", 1, 4, fp_temp);

        /* Calculate APIC Frame Size:
         * = 1 (Encoding byte)
         * + strlen(MIME type) + 1 (null terminator)
         * + 1 (Picture type byte)
         * + 1 (Description null terminator)
         * + Image data size */
        
        /* Use default MIME type if not specified */
        const char *mime = data->album_art.mime_type ? 
                          data->album_art.mime_type : "image/jpeg";
        
        uint32_t apic_frame_size = 1 +                      /* Encoding */
                                   strlen(mime) + 1 +       /* MIME + null */
                                   1 +                      /* Picture type */
                                   1 +                      /* Description null */
                                   data->album_art.image_size; /* Image data */

        /* Bytes 4-7: Frame size (synchsafe integer) */
        int_to_synchsafe(apic_frame_size, size_bytes);
        fwrite(size_bytes, 1, 4, fp_temp);

        /* Bytes 8-9: Frame flags (0x00 0x00 = no flags) */
        uint8_t frame_flags[2] = {0, 0};
        fwrite(frame_flags, 1, 2, fp_temp);

        /* ---------------------------------------------------------------------
         * Write APIC Frame Data
         * 
         * APIC Frame Structure:
         *   1. Text encoding (1 byte): 0x00 = ISO-8859-1
         *   2. MIME type (variable): null-terminated string
         *   3. Picture type (1 byte): 0x03 = Cover (front)
         *   4. Description (variable): empty (just null terminator)
         *   5. Picture data (variable): raw image bytes
         * --------------------------------------------------------------------- */
        
        /* 1. Text encoding */
        uint8_t encoding = 0x00;  /* ISO-8859-1 */
        fwrite(&encoding, 1, 1, fp_temp);
        
        /* 2. MIME type (with null terminator) */
        fwrite(mime, 1, strlen(mime) + 1, fp_temp);
        
        /* 3. Picture type */
        uint8_t pic_type = 0x03;  /* 0x03 = Cover (front) */
        fwrite(&pic_type, 1, 1, fp_temp);
        
        /* 4. Description (empty - just null terminator) */
        uint8_t desc_null = 0x00;
        fwrite(&desc_null, 1, 1, fp_temp);
        
        /* 5. Picture data (raw image binary) */
        fwrite(data->album_art.image_data, 1, data->album_art.image_size, fp_temp);
    }

    /* =========================================================================
     * STEP 6: Write Padding (1024 null bytes)
     * 
     * Padding allows future edits to change tag values without rewriting
     * the entire file. If a new value is shorter than the old, the extra
     * space becomes padding. If longer, padding can be consumed.
     * ========================================================================= */
    uint8_t null_byte = 0;
    for (int i = 0; i < 1024; i++) {
        fwrite(&null_byte, 1, 1, fp_temp);
    }

    /* =========================================================================
     * STEP 7: Copy Audio Data from Original File
     * 
     * Process:
     *   1. Read old tag size from original file's header
     *   2. Seek past the old tag to reach audio data
     *   3. Copy all remaining bytes (audio) to temp file
     * ========================================================================= */
    
    /* Read the old file's header to get old tag size */
    unsigned char old_header[10];
    if (fread(old_header, 1, 10, fp_src) == 10) {
        /* Extract old tag size (synchsafe integer at bytes 6-9) */
        uint32_t old_tag_size = synchsafe_to_int(&old_header[6]);
        
        /* Seek to start of audio data
         * Position = old_tag_size + 10 (header size) */
        fseek(fp_src, old_tag_size + 10, SEEK_SET);
        
        /* Copy audio data in 4KB chunks */
        char buffer[4096];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), fp_src)) > 0) {
            fwrite(buffer, 1, n, fp_temp);
        }
    }

    /* =========================================================================
     * STEP 8: Close files and replace original with updated version
     * ========================================================================= */
    fclose(fp_src);
    fclose(fp_temp);

    /* Replace original file with temp file */
    remove(filename);
    rename("temp.mp3", filename);
    
    return 0;
}

/* ============================================================================
 * HIGH-LEVEL EDITING FUNCTION
 * ============================================================================
 */

/**
 * @brief Edits a specific tag field in an MP3 file
 * 
 * @details
 * Workflow:
 *   1. Read current tags from the file
 *   2. Map the tag option to the correct field
 *   3. Allocate memory for new value with encoding prefix
 *   4. Update the field in the TagData structure
 *   5. Write all tags back to the file
 * 
 * The new value is stored with the format: [0x00][text][\0]
 *   - 0x00: ISO-8859-1 encoding indicator
 *   - text: The actual text content
 *   - \0: Null terminator
 * 
 * @param[in] filename Path to the MP3 file to edit
 * @param[in] tag      Tag option ("-t", "-a", "-A", "-y", "-c", "-g", etc.)
 * @param[in] value    New text value for the tag (without encoding prefix)
 * 
 * @return 0 on success, non-zero on failure
 */
int edit_tag(const char *filename, const char *tag, const char *value)
{
    /* =========================================================================
     * STEP 1: Read current tags from the file
     * ========================================================================= */
    TagData *data = read_id3_tags(filename);
    if (!data) {
        return 1;
    }

    /* =========================================================================
     * STEP 2: Map command-line option to TagData field
     * ========================================================================= */
    char **field_ptr = NULL;
    
    if      (strcmp(tag, "-t") == 0) field_ptr = &data->title;
    else if (strcmp(tag, "-a") == 0) field_ptr = &data->artist;
    else if (strcmp(tag, "-A") == 0) field_ptr = &data->album;
    else if (strcmp(tag, "-y") == 0) field_ptr = &data->year;
    else if (strcmp(tag, "-c") == 0) field_ptr = &data->comment;
    else if (strcmp(tag, "-g") == 0) field_ptr = &data->genre;
    else if (strcmp(tag, "-T") == 0) field_ptr = &data->track;
    else if (strcmp(tag, "-b") == 0) field_ptr = &data->album_artist;
    else if (strcmp(tag, "-m") == 0) field_ptr = &data->composer;
    else if (strcmp(tag, "-o") == 0) field_ptr = &data->copyright;
    else if (strcmp(tag, "-p") == 0) field_ptr = &data->publisher;
    else {
        /* Unknown tag option */
        display_error("Unknown Tag Option");
        free_tag_data(data);
        return 1;
    }

    /* =========================================================================
     * STEP 3: Allocate memory for new content
     * 
     * Size calculation:
     *   - strlen(value): Length of the text
     *   - + 1: For encoding byte (0x00)
     *   - + 1: For null terminator
     * ========================================================================= */
    int len = strlen(value);
    char *new_content = malloc(len + 2);
    
    if (new_content == NULL) {
        display_error("Memory allocation failed");
        free_tag_data(data);
        return 1;
    }

    /* =========================================================================
     * STEP 4: Format the new content: [0x00][text][\0]
     * ========================================================================= */
    new_content[0] = 0x00;               /* Set encoding to ISO-8859-1 */
    strcpy(new_content + 1, value);      /* Copy text after encoding byte */

    /* =========================================================================
     * STEP 5: Update the field in the TagData structure
     * ========================================================================= */
    if (*field_ptr != NULL) {
        free(*field_ptr);  /* Free old value if it exists */
    }
    *field_ptr = new_content;

    /* =========================================================================
     * STEP 6: Write updated tags back to file
     * ========================================================================= */
    if (write_id3_tags(filename, data) != 0) {
        free_tag_data(data);
        return 1;
    }

    printf("Tag Edited Successfully.\n");
    free_tag_data(data);
    return 0;
}

/* ============================================================================
 * ALBUM ART EXTRACTION FUNCTION
 * ============================================================================
 */

/**
 * @brief Extracts the embedded album art from an MP3 file to an image file
 * 
 * @details
 * This function parses the APIC frame structure to locate the actual
 * image data, then writes it to a separate file.
 * 
 * APIC Frame Parsing:
 *   The raw album_art.image_data buffer contains:
 *     [encoding][MIME\0][pic_type][desc\0][IMAGE DATA]
 *   
 *   We need to skip the metadata to find [IMAGE DATA]:
 *     1. Skip encoding byte (1 byte)
 *     2. Skip MIME type (variable, null-terminated)
 *     3. Skip picture type (1 byte)
 *     4. Skip description (variable, null-terminated)
 *     5. What remains is the actual image data
 * 
 * @param[in] mp3_filename        Path to the MP3 file
 * @param[in] output_image_name   Path for the extracted image file
 * 
 * @return 0 on success, non-zero on failure
 */
int extract_album_art(const char *mp3_filename, const char *output_image_name) {
    /* =========================================================================
     * STEP 1: Read tags from MP3 file
     * ========================================================================= */
    TagData *data = read_id3_tags(mp3_filename);
    if (!data) {
        return 1;
    }

    /* =========================================================================
     * STEP 2: Verify album art exists
     * ========================================================================= */
    if (data->album_art.image_data == NULL || data->album_art.image_size == 0) {
        display_error("No embedded album art found.");
        free_tag_data(data);
        return 1;
    }

    /* =========================================================================
     * STEP 3: Open output file for writing
     * ========================================================================= */
    FILE *fp_out = fopen(output_image_name, "wb");
    if (!fp_out) {
        display_error("Could not open output image file.");
        free_tag_data(data);
        return 1;
    }

    /* =========================================================================
     * STEP 4: Calculate offset to actual image data
     * 
     * Parse APIC frame structure to skip metadata
     * ========================================================================= */
    char *ptr = (char*)data->album_art.image_data;
    uint32_t full_size = data->album_art.image_size;
    uint32_t offset = 0;

    /* 1. Skip Text Encoding (1 Byte) */
    offset += 1;

    /* 2. Skip MIME Type (null-terminated string)
     * Example: "image/jpeg\0" */
    while (offset < full_size && ptr[offset] != '\0') {
        offset++;
    }
    offset++; /* Skip the null terminator */

    /* 3. Skip Picture Type (1 Byte)
     * 0x03 = Cover (front), etc. */
    offset += 1;

    /* 4. Skip Description (null-terminated string)
     * Usually empty (just "\0") */
    while (offset < full_size && ptr[offset] != '\0') {
        offset++;
    }
    offset++; /* Skip the null terminator */

    /* =========================================================================
     * STEP 5: Safety check - ensure we haven't gone past buffer end
     * ========================================================================= */
    if (offset >= full_size) {
        display_error("APIC frame is corrupted or empty.");
        fclose(fp_out);
        free_tag_data(data);
        return 1;
    }

    /* =========================================================================
     * STEP 6: Write ONLY the actual image binary data
     * ========================================================================= */
    uint32_t actual_image_size = full_size - offset;
    fwrite(ptr + offset, 1, actual_image_size, fp_out);
    
    /* =========================================================================
     * STEP 7: Clean up and return
     * ========================================================================= */
    fclose(fp_out);
    free_tag_data(data);
    printf("Album art extracted to: %s\n", output_image_name);
    return 0;
}

/* ============================================================================
 * TAG DELETION FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Deletes ALL ID3v2 tags from the file
 * 
 * @details
 * This function strips all metadata, leaving only audio data.
 * 
 * Process:
 *   1. Read header to get tag size
 *   2. Create temp file
 *   3. Seek past tag in source
 *   4. Copy audio data
 *   5. Replace original
 * 
 * @param[in] filename Path to the MP3 file
 * 
 * @return 0 on success, non-zero on failure
 */
int delete_all_tags(const char *filename) {
    /* =========================================================================
     * STEP 1: Open the original file
     * ========================================================================= */
    FILE *fp_src = fopen(filename, "rb");
    if (!fp_src) {
        display_error("File Could Not Open");
        return 1;
    }

    /* =========================================================================
     * STEP 2: Read header to determine tag size
     * ========================================================================= */
    unsigned char header[10];
    if (fread(header, 1, 10, fp_src) != 10) {
        fclose(fp_src);
        return 1;
    }

    /* =========================================================================
     * STEP 3: Verify ID3 tag exists
     * ========================================================================= */
    if (strncmp((char*)header, "ID3", 3) != 0) {
        printf("No ID3v2 tags found to delete.\n");
        fclose(fp_src);
        return 0;
    }

    /* =========================================================================
     * STEP 4: Calculate offset to audio data
     * ========================================================================= */
    uint32_t tag_size = synchsafe_to_int(&header[6]);
    uint32_t data_start = tag_size + 10;  /* Tag size + header size */

    /* =========================================================================
     * STEP 5: Create temporary file for stripped version
     * ========================================================================= */
    FILE *fp_temp = fopen("temp_stripped.mp3", "wb");
    if (!fp_temp) {
        fclose(fp_src);
        return 1;
    }

    /* =========================================================================
     * STEP 6: Seek past the ID3 tag to audio data
     * ========================================================================= */
    fseek(fp_src, data_start, SEEK_SET);

    /* =========================================================================
     * STEP 7: Copy audio data to temp file
     * ========================================================================= */
    char buffer[4096];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), fp_src)) > 0) {
        fwrite(buffer, 1, n, fp_temp);
    }

    /* =========================================================================
     * STEP 8: Clean up and replace original
     * ========================================================================= */
    fclose(fp_src);
    fclose(fp_temp);

    remove(filename);
    rename("temp_stripped.mp3", filename);
    printf("All ID3 tags deleted successfully.\n");
    return 0;
}

/**
 * @brief Deletes a specific tag field from the MP3 file
 * 
 * @details
 * Process:
 *   1. Read all tags
 *   2. Free the specified field
 *   3. Set field to NULL
 *   4. Rewrite file (NULL fields are skipped)
 * 
 * @param[in] filename Path to the MP3 file
 * @param[in] tag      Tag option to delete (e.g., "-t", "-a")
 * 
 * @return 0 on success, non-zero on failure
 */
int delete_tag(const char *filename, const char *tag) {
    /* =========================================================================
     * STEP 1: Read current tags
     * ========================================================================= */
    TagData *data = read_id3_tags(filename);
    if (!data) {
        return 1;
    }

    /* =========================================================================
     * STEP 2: Map option to field pointer
     * ========================================================================= */
    char **field_ptr = NULL;

    if      (strcmp(tag, "-t") == 0) field_ptr = &data->title;
    else if (strcmp(tag, "-a") == 0) field_ptr = &data->artist;
    else if (strcmp(tag, "-A") == 0) field_ptr = &data->album;
    else if (strcmp(tag, "-y") == 0) field_ptr = &data->year;
    else if (strcmp(tag, "-c") == 0) field_ptr = &data->comment;
    else if (strcmp(tag, "-g") == 0) field_ptr = &data->genre;
    else if (strcmp(tag, "-T") == 0) field_ptr = &data->track;
    else if (strcmp(tag, "-b") == 0) field_ptr = &data->album_artist;
    else if (strcmp(tag, "-m") == 0) field_ptr = &data->composer;
    else if (strcmp(tag, "-o") == 0) field_ptr = &data->copyright;
    else if (strcmp(tag, "-p") == 0) field_ptr = &data->publisher;
    else {
        display_error("Unknown Tag Option");
        free_tag_data(data);
        return 1;
    }

    /* =========================================================================
     * STEP 3: Delete the field (free memory and set to NULL)
     * ========================================================================= */
    if (field_ptr && *field_ptr) {
        free(*field_ptr);
        *field_ptr = NULL;
    } else {
        printf("Warning: Tag was already empty.\n");
    }

    /* =========================================================================
     * STEP 4: Rewrite the file (write_id3_tags skips NULL fields)
     * ========================================================================= */
    if (write_id3_tags(filename, data) != 0) {
        free_tag_data(data);
        return 1;
    }

    printf("Tag deleted successfully.\n");
    free_tag_data(data);
    return 0;
}