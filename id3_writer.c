/**
 * @file id3_writer.c
 * @brief Implementation of functions for writing and editing ID3 tags in MP3 files.
 */

#include <stdio.h>
#include <string.h>
#include "id3_writer.h"
#include "id3_reader.h"
#include "id3_utils.h"

/**
 * @brief Writes the ID3 tags to an MP3 file.
 * 
 * @param filename The name of the MP3 file.
 * @param data Pointer to the TagData structure containing the ID3 tags.
 * @return 0 on success, non-zero on failure.
 */
int write_id3_tags(const char *filename, const TagData *data) {
    // Implementation for writing ID3 tags
    return 0;
}

/** TODO: Add documentation as sample given above */
int edit_tag(const char *filename, const char *tag, const char *value) {
    TagData *data = read_id3_tags(filename);
    if (!data) {
        return 1;
    }
    // Implementation for writing ID3 tags to file

    return 0;
}
