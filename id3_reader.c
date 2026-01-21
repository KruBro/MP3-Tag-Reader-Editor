/**
 * @file id3_reader.c
 * @brief Implementation of functions for reading ID3 tags from MP3 files.
 */

#include <stdio.h>
#include <stdlib.h>
#include "id3_reader.h"
#include "error_handling.h"

/**
TODO: Add documention as sample given
 */
TagData* read_id3_tags(const char *filename) {
    // Implementation for reading ID3 tags
    return NULL;
}

/**
TODO: Add documention as sample given
 */
void display_metadata(const TagData *data) {
    // Implementation for displaying metadata
}

/**
TODO: Add documention as sample given
 */
void view_tags(const char *filename) {
    TagData *data = read_id3_tags(filename);
    if (!data) {
        display_error("Failed to read ID3 tags.");
        return;
    }
    display_metadata(data);
    free_tag_data(data);
}
