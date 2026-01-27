/**
 * ============================================================================
 * @file id3_reader.h
 * @brief Functions for reading and displaying ID3 tags from MP3 files
 * 
 * This module provides the core functionality for:
 *   - Reading ID3v2 tags from MP3 files
 *   - Parsing individual frames (title, artist, album, etc.)
 *   - Displaying metadata in a user-friendly format
 *   - Decoding UTF-16 encoded text
 * 
 * @details
 * Supported ID3 versions: ID3v2.3 and ID3v2.4
 * Supported text encodings:
 *   - ISO-8859-1 (Latin-1) - encoding byte 0x00
 *   - UTF-16 with BOM      - encoding byte 0x01
 * 
 * @note This implementation focuses on text frames and album artwork.
 *       Other frame types (e.g., PRIV, GEOB) are not processed but are
 *       safely skipped during parsing.
 * ============================================================================
 */

#ifndef ID3_READER_H
#define ID3_READER_H

#include "id3_utils.h"

/* ============================================================================
 * PRIMARY READING FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Reads and parses all ID3 tags from an MP3 file
 * 
 * @details
 * This function performs a complete read of the ID3v2 tag structure:
 * 
 * Process Flow:
 *   1. Open the file in binary read mode
 *   2. Read and validate the 10-byte ID3v2 header
 *   3. Verify the ID3 signature ("ID3")
 *   4. Check version compatibility (only v2.3 and v2.4 supported)
 *   5. Parse the tag size (synchsafe integer)
 *   6. Iterate through all frames in the tag:
 *      a. Read 10-byte frame header
 *      b. Extract frame ID and size
 *      c. Read frame content
 *      d. Handle text encoding (ISO-8859-1 or UTF-16)
 *      e. Store in appropriate TagData field
 *   7. Handle padding bytes
 *   8. Return populated TagData structure
 * 
 * Supported Frames:
 *   ┌──────────┬─────────────────┬────────────────────┐
 *   │ Frame ID │ Description     │ TagData Field      │
 *   ├──────────┼─────────────────┼────────────────────┤
 *   │ TIT2     │ Title           │ title              │
 *   │ TPE1     │ Artist          │ artist             │
 *   │ TALB     │ Album           │ album              │
 *   │ TYER     │ Year            │ year               │
 *   │ TRCK     │ Track           │ track              │
 *   │ TCON     │ Genre           │ genre              │
 *   │ COMM     │ Comment         │ comment            │
 *   │ TPE2     │ Album Artist    │ album_artist       │
 *   │ TCOM     │ Composer        │ composer           │
 *   │ TCOP     │ Copyright       │ copyright          │
 *   │ TPUB     │ Publisher       │ publisher          │
 *   │ APIC     │ Album Art       │ album_art          │
 *   └──────────┴─────────────────┴────────────────────┘
 * 
 * Frame Size Calculation:
 *   - ID3v2.3: Uses standard big-endian integers
 *   - ID3v2.4: Uses synchsafe integers
 * 
 * Text Encoding Handling:
 *   - 0x00: ISO-8859-1 (stored as-is)
 *   - 0x01: UTF-16 with BOM (converted to ASCII for simplicity)
 * 
 * @param[in] filename Path to the MP3 file to read
 * 
 * @return Pointer to a TagData structure containing all parsed metadata,
 *         or NULL if:
 *         - File cannot be opened
 *         - File has no valid ID3 tags
 *         - ID3 version is not supported
 *         - Memory allocation fails
 * 
 * @note The caller is responsible for freeing the returned TagData structure
 *       using free_tag_data() to prevent memory leaks
 * 
 * @note Version support:
 *       - ID3v2.3 (version byte 0x03) ✓ Supported
 *       - ID3v2.4 (version byte 0x04) ✓ Supported
 *       - ID3v2.2 and earlier        ✗ Not supported
 * 
 * @warning This function will return NULL for ID3v2.2 and earlier versions,
 *          as they use a different frame header structure (6 bytes vs 10 bytes)
 * 
 * @par Example Usage:
 * @code
 * // Read tags from a file
 * TagData *tags = read_id3_tags("mysong.mp3");
 * 
 * if (tags == NULL) {
 *     fprintf(stderr, "Failed to read ID3 tags\n");
 *     return -1;
 * }
 * 
 * // Access the data
 * if (tags->title) {
 *     printf("Title: %s\n", tags->title + 1);  // Skip encoding byte
 * }
 * 
 * // Clean up
 * free_tag_data(tags);
 * @endcode
 */
TagData* read_id3_tags(const char *filename);

/* ============================================================================
 * DISPLAY FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Displays all metadata from a TagData structure in formatted output
 * 
 * @details
 * This function creates a user-friendly display of all ID3 tag information.
 * 
 * Output format:
 *   Mp3 Tag Reader & Editor:
 *   ---------------------
 *   Version ID : 2.3
 *   Title      : Song Name
 *   Artist     : Artist Name
 *   Album      : Album Name
 *   Year       : 2024
 *   Track      : 5
 *   Genre      : Rock
 *   Comment    : My favorite song
 *   Alb.Artist : Band Name
 *   Composer   : Composer Name
 *   Copyright  : © 2024
 *   Publisher  : Label Name
 *   Image Type : image/jpeg
 *   Image Size : 45678 bytes
 * 
 * Text Encoding Handling:
 *   - Checks the first byte of each field (encoding indicator)
 *   - If 0x00: Prints as ISO-8859-1 (skip first byte, print rest)
 *   - If 0x01: Calls utf_16_decoder() for UTF-16 text
 * 
 * Field Alignment:
 *   - Labels are left-aligned with 10-character width
 *   - Ensures consistent, readable output
 * 
 * @param[in] data Pointer to TagData structure containing the metadata to display
 * 
 * @note If data is NULL, this function returns immediately without output
 * 
 * @note Fields that are NULL (not present in the file) are not displayed
 * 
 * @note UTF-16 text is simplified to ASCII by the utf_16_decoder() function.
 *       Only printable ASCII characters are displayed. Full Unicode support
 *       would require a more sophisticated decoder.
 * 
 * @par Example Output:
 * @code
 * Mp3 Tag Reader & Editor:
 * ---------------------
 * Version ID : 2.4
 * Title      : Bohemian Rhapsody
 * Artist     : Queen
 * Album      : A Night at the Opera
 * Year       : 1975
 * Track      : 11
 * Genre      : Rock
 * Image Type : image/jpeg
 * Image Size : 52341 bytes
 * @endcode
 */
void display_metadata(const TagData *data);

/**
 * @brief Reads and displays ID3 tags from a file in one operation
 * 
 * @details
 * This is a high-level convenience function that combines reading and
 * displaying tags in a single operation. It:
 *   1. Calls read_id3_tags() to parse the file
 *   2. Calls display_metadata() to show the results
 *   3. Automatically cleans up allocated memory
 * 
 * This is the simplest way to view tags and is ideal for command-line
 * tools and simple applications where you just want to display the
 * metadata without further processing.
 * 
 * Error Handling:
 *   - If reading fails, displays an error message via display_error()
 *   - Returns immediately without attempting to display
 * 
 * @param[in] filename Path to the MP3 file to view
 * 
 * @note This function handles all memory management internally.
 *       No cleanup is required by the caller.
 * 
 * @note If reading fails (file not found, no ID3 tags, unsupported version),
 *       an error message is displayed and the function returns.
 * 
 * @par Example Usage:
 * @code
 * // Simple one-line tag viewing
 * view_tags("mysong.mp3");
 * 
 * // That's it! No memory management needed.
 * @endcode
 * 
 * @par Equivalent Code:
 * @code
 * // This function does the same as:
 * TagData *data = read_id3_tags(filename);
 * if (data) {
 *     display_metadata(data);
 *     free_tag_data(data);
 * } else {
 *     display_error("Failed to read ID3 tags.");
 * }
 * @endcode
 */
void view_tags(const char *filename);

/* ============================================================================
 * TEXT ENCODING FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Decodes and prints UTF-16 Little Endian encoded text to stdout
 * 
 * @details
 * ID3 tags can use UTF-16LE encoding for international characters.
 * This function handles the decoding and display of such text.
 * 
 * UTF-16LE Structure in ID3 Tags:
 *   Byte 0:    0x01 (encoding flag indicating UTF-16)
 *   Bytes 1-2: BOM (Byte Order Mark) - typically 0xFF 0xFE for Little Endian
 *   Bytes 3+:  Character data (2 bytes per character)
 * 
 * Decoding Process:
 *   1. Skip byte 0 (encoding flag)
 *   2. Skip bytes 1-2 (BOM)
 *   3. Start reading at byte 3
 *   4. Read every other byte (UTF-16LE stores ASCII as [char, 0x00])
 *   5. Print printable ASCII characters
 *   6. Skip non-printable characters
 *   7. Stop at double null (0x00 0x00)
 * 
 * Limitations:
 *   - This is a simplified decoder for ASCII subset of UTF-16
 *   - Only extracts characters in the ASCII range (0x01-0x7F)
 *   - Full Unicode characters (outside ASCII) are not displayed
 *   - Surrogate pairs are not handled
 * 
 * Why this works for most English text:
 *   - Most ID3 tags use English text (ASCII range)
 *   - UTF-16LE stores ASCII as: [char] [0x00]
 *   - By reading every other byte, we extract the ASCII
 * 
 * @param[in] content Pointer to UTF-16 encoded string buffer
 *                    (must start with 0x01 encoding flag)
 * 
 * @note This function prints directly to stdout and appends a newline
 * 
 * @note The function reads until it encounters a double null terminator
 *       (0x00 0x00), which marks the end of the UTF-16 string
 * 
 * @warning This is not a full UTF-16 decoder. It only handles the ASCII
 *          subset of UTF-16. For full Unicode support, use a proper UTF-16
 *          to UTF-8 conversion library (e.g., iconv, ICU).
 * 
 * @par Example:
 * @code
 * // UTF-16LE encoded "Hi" with BOM:
 * // [0x01] [0xFF] [0xFE] ['H'] [0x00] ['i'] [0x00] [0x00] [0x00]
 * unsigned char utf16_text[] = {0x01, 0xFF, 0xFE, 'H', 0x00, 'i', 0x00, 0x00, 0x00};
 * 
 * utf_16_decoder((char*)utf16_text);
 * // Output: "Hi\n"
 * @endcode
 */
void utf_16_decoder(char *content);

/**
 * @brief Converts UTF-16 (LE) buffer to ISO-8859-1 (ASCII) in place
 * 
 * @details
 * This function fixes the "strlen stops at null byte" bug that occurs when
 * processing UTF-16 encoded text with standard C string functions.
 * 
 * The Problem:
 *   - UTF-16 stores ASCII as: [char] [0x00] [char] [0x00]...
 *   - strlen() stops at the first 0x00 byte
 *   - This causes incorrect length calculations and truncated text
 * 
 * The Solution:
 *   - Extract every other byte (the actual characters)
 *   - Store them in a new buffer without the 0x00 padding
 *   - Update encoding flag to 0x00 (ISO-8859-1)
 *   - Replace the original buffer with the cleaned buffer
 * 
 * Conversion Process:
 *   1. Allocate new buffer for clean ASCII
 *   2. Set byte 0 to 0x00 (ISO-8859-1 encoding)
 *   3. Skip original bytes 0-2 (encoding flag and BOM)
 *   4. Starting at byte 3, copy every other byte
 *   5. Ignore null padding bytes (0x00)
 *   6. Null-terminate the new string
 *   7. Free the original buffer
 *   8. Update the pointer to point to the new buffer
 * 
 * @param[in,out] buffer Pointer to pointer to the UTF-16 buffer
 *                       (buffer is freed and replaced with ASCII version)
 * @param[in]     size   Size of the original buffer in bytes
 * 
 * @note This function modifies the buffer pointer (*buffer).
 *       The original buffer is freed, and the pointer is updated
 *       to point to the newly allocated ASCII buffer.
 * 
 * @note The new buffer is always smaller than or equal to the original
 *       buffer size, as we're removing padding bytes.
 * 
 * @warning The caller must not use the original buffer pointer after
 *          calling this function, as it has been freed.
 * 
 * @par Example:
 * @code
 * // UTF-16LE: [0x01] [0xFF] [0xFE] ['H'] [0x00] ['i'] [0x00]
 * char *utf16_buf = malloc(7);
 * memcpy(utf16_buf, "\x01\xFF\xFEH\x00i\x00", 7);
 * 
 * convert_utf16_to_ascii(&utf16_buf, 7);
 * 
 * // Now utf16_buf points to: [0x00] ['H'] ['i'] [0x00]
 * // (ISO-8859-1 encoding with ASCII text)
 * printf("%s\n", utf16_buf + 1);  // Output: "Hi"
 * 
 * free(utf16_buf);  // Clean up the new buffer
 * @endcode
 */
void convert_utf16_to_ascii(char **buffer, int size);

#endif // ID3_READER_H