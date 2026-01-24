/**
 * @file id3_writer.h
 * @brief Functions for writing and editing ID3 tags in MP3 files.
 * 
 * This module provides functionality for:
 * - Creating new ID3v2.4 tags
 * - Updating existing tags with new metadata
 * - Editing individual tag fields
 * - Preserving audio data while modifying tags
 */

#ifndef ID3_WRITER_H
#define ID3_WRITER_H

#include "id3_utils.h"

/**
 * @brief Writes ID3 tags to an MP3 file, replacing any existing tags.
 * 
 * This function performs a complete rewrite of the ID3 tag section:
 * 
 * 1. Calculates the required size for new tags
 * 2. Creates a temporary file with the new ID3v2.4 tag
 * 3. Writes all provided metadata as frames
 * 4. Adds padding for future edits
 * 5. Copies the audio data from the original file
 * 6. Replaces the original file with the updated version
 * 
 * The new tag is written in ID3v2.4 format with synchsafe integers.
 * All text frames use ISO-8859-1 encoding (encoding byte = 0x00).
 * 
 * Tag structure written:
 * - ID3v2.4 header (10 bytes)
 * - Text frames (TIT2, TPE1, TALB, TYER, TCON, COMM)
 * - Padding (1024 bytes)
 * - Original audio data
 * 
 * @param filename The name of the MP3 file to modify.
 * @param data Pointer to TagData structure containing the tags to write.
 * 
 * @return 0 on success, non-zero on failure.
 * 
 * @warning This function overwrites the original file. Ensure you have a backup
 *          if needed, or that you're willing to lose the original tags.
 * 
 * @note The function preserves all audio data but replaces the entire ID3 tag,
 *       so any frames not explicitly set in 'data' will be lost.
 * 
 * @example
 * TagData *data = create_tag_data();
 * data->title = strdup("\x00My Song");  // 0x00 = ISO-8859-1 encoding
 * data->artist = strdup("\x00The Band");
 * write_id3_tags("song.mp3", data);
 * free_tag_data(data);
 */
int write_id3_tags(const char *filename, const TagData *data);

/**
 * @brief Edits a specific tag field in an MP3 file.
 * 
 * This is a high-level convenience function that:
 * 1. Reads the current tags from the file
 * 2. Updates the specified field with the new value
 * 3. Writes the modified tags back to the file
 * 
 * Supported tag options:
 * - "-t" : Title (TIT2 frame)
 * - "-a" : Artist (TPE1 frame)
 * - "-A" : Album (TALB frame)
 * - "-y" : Year (TYER frame)
 * - "-c" : Comment (COMM frame)
 * - "-g" : Genre (TCON frame)
 * 
 * The new value is automatically formatted with the correct encoding byte
 * (0x00 for ISO-8859-1) before being stored.
 * 
 * @param filename Path to the MP3 file to edit.
 * @param tag Tag option specifying which field to edit (e.g., "-t", "-a").
 * @param value New value for the tag field (plain text, no encoding byte).
 * 
 * @return 0 on success, non-zero on failure.
 * 
 * @note All other existing tags are preserved - only the specified field changes.
 * 
 * @note The function displays "Tag Edited Successfully." on success.
 * 
 * @example
 * // Change the artist to "New Artist"
 * edit_tag("song.mp3", "-a", "New Artist");
 * 
 * // Change the title to "New Title"
 * edit_tag("song.mp3", "-t", "New Title");
 */
int edit_tag(const char *filename, const char *tag, const char *value);

#endif // ID3_WRITER_H