/**
 * @file id3_reader.h
 * @brief Functions for reading and displaying ID3 tags from MP3 files.
 * 
 * This module provides the core functionality for:
 * - Reading ID3v2 tags from MP3 files
 * - Parsing individual frames (title, artist, album, etc.)
 * - Displaying metadata in a user-friendly format
 * - Decoding UTF-16 encoded text
 */

#ifndef ID3_READER_H
#define ID3_READER_H

#include "id3_utils.h"

/**
 * @brief Reads and parses all ID3 tags from an MP3 file.
 * 
 * This function performs a complete read of the ID3v2 tag structure:
 * 1. Opens the specified file
 * 2. Validates the ID3 header
 * 3. Iterates through all frames in the tag
 * 4. Extracts supported frames (TIT2, TPE1, TALB, TYER, TCON, COMM)
 * 5. Handles both ID3v2.3 and ID3v2.4 frame size encoding
 * 
 * @param filename Path to the MP3 file to read.
 * 
 * @return Pointer to a TagData structure containing all parsed metadata,
 *         or NULL if the file cannot be read or has no valid ID3 tags.
 * 
 * @note The caller is responsible for freeing the returned TagData structure
 *       using free_tag_data().
 * 
 * @note Supports ID3v2.3 and ID3v2.4 tags. Other versions will be rejected.
 * 
 * @example
 * TagData *tags = read_id3_tags("song.mp3");
 * if (tags) {
 *     // Use the tags
 *     free_tag_data(tags);
 * }
 */
TagData* read_id3_tags(const char *filename);

/**
 * @brief Displays all metadata from a TagData structure.
 * 
 * Formats and prints all available ID3 tag information to stdout, including:
 * - ID3 version
 * - Title, artist, album, year, genre, and comment
 * - Handles both ISO-8859-1 (0x00) and UTF-16 (0x01) text encodings
 * 
 * Text fields are automatically decoded based on their encoding byte:
 * - 0x00: ISO-8859-1 (printed directly)
 * - 0x01: UTF-16 with BOM (decoded using utf_16_decoder)
 * 
 * @param data Pointer to TagData structure containing the metadata to display.
 * 
 * @note If data is NULL, this function returns without doing anything.
 * 
 * @note UTF-16 text is simplified to ASCII by this implementation (only
 *       printable ASCII characters are displayed).
 * 
 * @example
 * TagData *tags = read_id3_tags("song.mp3");
 * display_metadata(tags);
 * // Output:
 * // Mp3 Tag Reader & Editor:
 * // ---------------------
 * // Version ID : 2.3
 * // Title      : My Song
 * // Artist     : The Band
 * // ...
 */
void display_metadata(const TagData *data);

/**
 * @brief Reads and displays ID3 tags from a file (convenience function).
 * 
 * This is a high-level convenience function that combines reading and
 * displaying tags in a single operation. It:
 * 1. Calls read_id3_tags() to parse the file
 * 2. Calls display_metadata() to show the results
 * 3. Automatically cleans up allocated memory
 * 
 * @param filename Path to the MP3 file to view.
 * 
 * @note If reading fails, an error message is displayed via display_error().
 * 
 * @example
 * view_tags("song.mp3");
 * // Reads and displays all tags in one call
 */
void view_tags(const char *filename);

/**
 * @brief Decodes and prints UTF-16 encoded text.
 * 
 * This function handles UTF-16 Little Endian (UTF-16LE) encoded strings
 * commonly found in ID3 tags. The encoding format is:
 * 
 * Byte 0: Encoding flag (0x01 for UTF-16)
 * Bytes 1-2: BOM (Byte Order Mark) - typically 0xFF 0xFE for UTF-16LE
 * Bytes 3+: UTF-16 character pairs
 * 
 * The function extracts printable ASCII characters from the UTF-16 stream
 * by reading every other byte (assuming most text is in the ASCII range).
 * 
 * @param content Pointer to UTF-16 encoded string buffer (must start with 0x01).
 * 
 * @note This is a simplified decoder that only handles basic ASCII characters
 *       within UTF-16. Full Unicode support would require proper UTF-16 decoding.
 * 
 * @note The function reads until it encounters a double null terminator (0x00 0x00).
 * 
 * @example
 * // content = {0x01, 0xFF, 0xFE, 'H', 0x00, 'i', 0x00, 0x00, 0x00}
 * utf_16_decoder(content);
 * // Output: "Hi\n"
 */
void utf_16_decoder(char *content);

#endif // ID3_READER_H