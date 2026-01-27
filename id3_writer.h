/**
 * ============================================================================
 * @file id3_writer.h
 * @brief Functions for writing and editing ID3 tags in MP3 files
 * 
 * This module provides functionality for:
 *   - Creating new ID3v2.4 tags from scratch
 *   - Updating existing tags with new metadata
 *   - Editing individual tag fields
 *   - Deleting specific tags or all tags
 *   - Extracting embedded album artwork
 *   - Preserving audio data while modifying tags
 * 
 * @details
 * Tag Modification Strategy:
 *   All write operations use a "read-modify-write" approach:
 *   1. Read existing tags (if any)
 *   2. Modify the desired fields
 *   3. Write complete new tag to temporary file
 *   4. Copy audio data from original file
 *   5. Replace original file with temporary file
 * 
 * Safety Features:
 *   - Uses temporary files to prevent data loss
 *   - Preserves all audio data
 *   - Only replaces original on successful completion
 * 
 * @note All write operations create ID3v2.4 format tags
 * @note Text frames use ISO-8859-1 encoding by default
 * ============================================================================
 */

#ifndef ID3_WRITER_H
#define ID3_WRITER_H

#include "id3_utils.h"

/* ============================================================================
 * PRIMARY WRITING FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Writes ID3 tags to an MP3 file, replacing any existing tags
 * 
 * @details
 * This function performs a complete rewrite of the ID3 tag section.
 * 
 * Process Flow:
 *   ┌─────────────────────────────────────┐
 *   │ 1. Calculate new tag size           │
 *   ├─────────────────────────────────────┤
 *   │ 2. Open source file (read)          │
 *   │    Open temp file (write)           │
 *   ├─────────────────────────────────────┤
 *   │ 3. Write ID3v2.4 header             │
 *   ├─────────────────────────────────────┤
 *   │ 4. Write text frames:               │
 *   │    - TIT2 (Title)                   │
 *   │    - TPE1 (Artist)                  │
 *   │    - TALB (Album)                   │
 *   │    - TYER (Year)                    │
 *   │    - TRCK (Track)                   │
 *   │    - TCON (Genre)                   │
 *   │    - COMM (Comment)                 │
 *   │    - TPE2 (Album Artist)            │
 *   │    - TCOM (Composer)                │
 *   │    - TCOP (Copyright)               │
 *   │    - TPUB (Publisher)               │
 *   ├─────────────────────────────────────┤
 *   │ 5. Write APIC frame (if present)    │
 *   ├─────────────────────────────────────┤
 *   │ 6. Write padding (1024 bytes)       │
 *   ├─────────────────────────────────────┤
 *   │ 7. Read old tag size from original  │
 *   │    Seek past old tag                │
 *   │    Copy audio data to temp file     │
 *   ├─────────────────────────────────────┤
 *   │ 8. Replace original with temp       │
 *   └─────────────────────────────────────┘
 * 
 * ID3v2.4 Tag Structure Written:
 *   ┌────────────────────┬──────────┬─────────────────────┐
 *   │ Component          │ Size     │ Description         │
 *   ├────────────────────┼──────────┼─────────────────────┤
 *   │ ID3 Header         │ 10 bytes │ "ID3" + version etc │
 *   │ Text Frames        │ Variable │ TIT2, TPE1, etc     │
 *   │ APIC Frame         │ Variable │ Album art (if any)  │
 *   │ Padding            │ 1024 b   │ Room for edits      │
 *   └────────────────────┴──────────┴─────────────────────┘
 * 
 * Text Frame Format:
 *   Each text frame consists of:
 *     - Frame header (10 bytes):
 *       * Frame ID (4 bytes, e.g., "TIT2")
 *       * Size (4 bytes, synchsafe)
 *       * Flags (2 bytes)
 *     - Encoding byte (1 byte): 0x00 for ISO-8859-1
 *     - Text content (variable length)
 * 
 * @param[in] filename The name of the MP3 file to modify
 * @param[in] data     Pointer to TagData structure containing the tags to write
 * 
 * @return 0 on success, non-zero on failure
 * 
 * @warning This function overwrites the original file's ID3 tags.
 *          Any frames not explicitly set in 'data' will be lost.
 *          Ensure you have a backup if needed.
 * 
 * @note The function preserves all audio data but replaces the entire ID3 tag.
 *       Padding (1024 bytes) is added to allow future edits without full rewrite.
 * 
 * @note All text frames are written with ISO-8859-1 encoding (0x00).
 *       UTF-16 text will be converted to ISO-8859-1.
 * 
 * @par Example Usage:
 * @code
 * // Create new tag data
 * TagData *data = create_tag_data();
 * 
 * // Set fields (note the 0x00 encoding prefix)
 * data->title = strdup("\x00My Song Title");
 * data->artist = strdup("\x00Artist Name");
 * data->album = strdup("\x00Album Name");
 * data->year = strdup("\x002024");
 * 
 * // Write to file
 * if (write_id3_tags("song.mp3", data) == 0) {
 *     printf("Tags written successfully\n");
 * } else {
 *     printf("Failed to write tags\n");
 * }
 * 
 * // Clean up
 * free_tag_data(data);
 * @endcode
 */
int write_id3_tags(const char *filename, const TagData *data);

/* ============================================================================
 * HIGH-LEVEL EDITING FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Edits a specific tag field in an MP3 file
 * 
 * @details
 * This is a high-level convenience function that simplifies tag editing.
 * 
 * Workflow:
 *   1. Read current tags from the file using read_id3_tags()
 *   2. Map the tag option to the correct TagData field
 *   3. Allocate memory for the new value with encoding prefix
 *   4. Update the specific field in the TagData structure
 *   5. Write all tags back to the file using write_id3_tags()
 *   6. Clean up allocated memory
 * 
 * Supported Tag Options:
 *   ┌────────┬─────────────────┬──────────────┬────────────────┐
 *   │ Option │ Frame ID        │ Field        │ Description    │
 *   ├────────┼─────────────────┼──────────────┼────────────────┤
 *   │ -t     │ TIT2            │ title        │ Song title     │
 *   │ -a     │ TPE1            │ artist       │ Artist name    │
 *   │ -A     │ TALB            │ album        │ Album name     │
 *   │ -y     │ TYER            │ year         │ Year           │
 *   │ -T     │ TRCK            │ track        │ Track number   │
 *   │ -g     │ TCON            │ genre        │ Genre          │
 *   │ -c     │ COMM            │ comment      │ Comment        │
 *   │ -b     │ TPE2            │ album_artist │ Album artist   │
 *   │ -m     │ TCOM            │ composer     │ Composer       │
 *   │ -o     │ TCOP            │ copyright    │ Copyright      │
 *   │ -p     │ TPUB            │ publisher    │ Publisher      │
 *   └────────┴─────────────────┴──────────────┴────────────────┘
 * 
 * Value Format:
 *   - The user provides plain text (e.g., "My Song")
 *   - Function automatically adds encoding prefix: [0x00][text][\0]
 *   - 0x00 indicates ISO-8859-1 encoding
 * 
 * @param[in] filename Path to the MP3 file to edit
 * @param[in] tag      Tag option specifying which field to edit (e.g., "-t", "-a")
 * @param[in] value    New value for the tag field (plain text, no encoding byte)
 * 
 * @return 0 on success, non-zero on failure
 * 
 * @note All other existing tags are preserved - only the specified field changes
 * 
 * @note If the tag option is unrecognized, the function displays an error
 *       message and returns 1
 * 
 * @note The function displays "Tag Edited Successfully." on success
 * 
 * @par Example Usage:
 * @code
 * // Change the title of a song
 * if (edit_tag("mysong.mp3", "-t", "New Title") == 0) {
 *     printf("Title updated\n");
 * }
 * 
 * // Change the artist
 * edit_tag("mysong.mp3", "-a", "New Artist Name");
 * 
 * // Change the year
 * edit_tag("mysong.mp3", "-y", "2024");
 * @endcode
 */
int edit_tag(const char *filename, const char *tag, const char *value);

/* ============================================================================
 * TAG DELETION FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Deletes a specific tag field from the MP3 file
 * 
 * @details
 * This function removes a single metadata field while preserving all others.
 * 
 * Process:
 *   1. Read all current tags
 *   2. Find the specified field in TagData
 *   3. Free the field's memory
 *   4. Set the field pointer to NULL
 *   5. Rewrite the file (NULL fields are skipped)
 * 
 * How it works:
 *   - The write_id3_tags() function checks each field
 *   - If a field is NULL, it's not written to the file
 *   - This effectively removes it from the MP3
 * 
 * @param[in] filename    Path to the MP3 file
 * @param[in] tag_option  Tag option to delete (e.g., "-t", "-a", "-c")
 * 
 * @return 0 on success, non-zero on failure
 * 
 * @note If the tag was already empty, a warning is displayed but operation succeeds
 * 
 * @note All other tags remain unchanged
 * 
 * @par Example Usage:
 * @code
 * // Delete the comment field
 * delete_tag("mysong.mp3", "-c");
 * 
 * // Delete the year field
 * delete_tag("mysong.mp3", "-y");
 * @endcode
 */
int delete_tag(const char *filename, const char *tag_option);

/**
 * @brief Deletes ALL ID3v2 tags from the file
 * 
 * @details
 * This function completely strips all metadata from the MP3 file,
 * leaving only the audio data.
 * 
 * Process:
 *   1. Open the original file
 *   2. Read and validate the ID3 header
 *   3. Calculate the tag size
 *   4. Create a temporary file
 *   5. Seek to the end of the tag (start of audio)
 *   6. Copy all audio data to the temporary file
 *   7. Replace the original file with the stripped version
 * 
 * Result:
 *   - Original file is replaced with audio-only version
 *   - File size is reduced by the size of the ID3 tag
 *   - No metadata remains in the file
 * 
 * Use Cases:
 *   - Privacy: Remove all identifying information
 *   - File size: Reduce file size by removing large album art
 *   - Clean slate: Start fresh with new metadata
 * 
 * @param[in] filename Path to the MP3 file
 * 
 * @return 0 on success, non-zero on failure
 * 
 * @warning This operation is irreversible. All metadata will be permanently lost.
 *          Make sure you have a backup if you might need the tags later.
 * 
 * @note If the file has no ID3v2 tags, a message is displayed and the file
 *       is left unchanged
 * 
 * @note The function displays "All ID3 tags deleted successfully." on success
 * 
 * @par Example Usage:
 * @code
 * // Strip all metadata from a file
 * if (delete_all_tags("mysong.mp3") == 0) {
 *     printf("File stripped of all metadata\n");
 * }
 * @endcode
 */
int delete_all_tags(const char *filename);

/* ============================================================================
 * ALBUM ART EXTRACTION FUNCTION
 * ============================================================================
 */

/**
 * @brief Extracts the embedded album art from an MP3 file to an image file
 * 
 * @details
 * This function reads the APIC (Attached Picture) frame from the MP3
 * and saves the raw image data to a separate file.
 * 
 * Process:
 *   1. Read ID3 tags using read_id3_tags()
 *   2. Check if album art exists (album_art.image_data != NULL)
 *   3. Parse the APIC frame structure to find the image data offset:
 *      - Skip text encoding byte (1 byte)
 *      - Skip MIME type string (variable, null-terminated)
 *      - Skip picture type byte (1 byte)
 *      - Skip description string (variable, null-terminated)
 *   4. Calculate actual image data offset
 *   5. Write raw image bytes to output file
 *   6. Clean up and return
 * 
 * APIC Frame Structure:
 *   ┌──────────────────┬──────────┬────────────────────────────┐
 *   │ Field            │ Size     │ Description                │
 *   ├──────────────────┼──────────┼────────────────────────────┤
 *   │ Text Encoding    │ 1 byte   │ 0x00 = ISO-8859-1          │
 *   │ MIME Type        │ Variable │ e.g., "image/jpeg\0"       │
 *   │ Picture Type     │ 1 byte   │ 0x03 = Cover (front)       │
 *   │ Description      │ Variable │ Usually empty: "\0"        │
 *   │ Picture Data     │ Variable │ Raw image bytes ← Extract! │
 *   └──────────────────┴──────────┴────────────────────────────┘
 * 
 * Supported Image Formats:
 *   - JPEG (.jpg, .jpeg) - Most common
 *   - PNG (.png)         - Common for high quality
 *   - GIF (.gif)         - Rare
 *   - BMP (.bmp)         - Very rare
 * 
 * @param[in] mp3_filename        Path to the MP3 file containing album art
 * @param[in] output_image_name   Path where the image should be saved
 *                                (extension should match MIME type)
 * 
 * @return 0 on success, non-zero on failure
 * 
 * @note The function does NOT automatically add a file extension.
 *       The caller should specify the complete output filename including
 *       the appropriate extension based on the MIME type.
 * 
 * @note If no album art is found, an error message is displayed
 * 
 * @note The function displays "Album art extracted to: <filename>" on success
 * 
 * @warning If the APIC frame is corrupted or has an unexpected structure,
 *          the extraction will fail with an error message
 * 
 * @par Example Usage:
 * @code
 * // Extract album art to a JPEG file
 * if (extract_album_art("mysong.mp3", "cover.jpg") == 0) {
 *     printf("Album art saved successfully\n");
 * }
 * 
 * // Extract to PNG (if the embedded image is PNG)
 * extract_album_art("mysong.mp3", "albumart.png");
 * @endcode
 * 
 * @par Determining Image Format:
 * @code
 * // First read tags to check MIME type
 * TagData *data = read_id3_tags("mysong.mp3");
 * if (data && data->album_art.mime_type) {
 *     printf("Image format: %s\n", data->album_art.mime_type);
 *     
 *     // Then extract with appropriate extension
 *     if (strcmp(data->album_art.mime_type, "image/jpeg") == 0) {
 *         extract_album_art("mysong.mp3", "cover.jpg");
 *     } else if (strcmp(data->album_art.mime_type, "image/png") == 0) {
 *         extract_album_art("mysong.mp3", "cover.png");
 *     }
 * }
 * free_tag_data(data);
 * @endcode
 */
int extract_album_art(const char *mp3_filename, const char *output_image_name);

#endif // ID3_WRITER_H