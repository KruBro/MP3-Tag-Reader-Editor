#ifndef ID3_WRITER_H
#define ID3_WRITER_H

#include "id3_utils.h"

/**
 * @brief Writes the ID3 tags to an MP3 file.
 * 
 * @param filename The name of the MP3 file.
 * @param data Pointer to the TagData structure containing the ID3 tags.
 * @return 0 on success, non-zero on failure.
 */
int write_id3_tags(const char *filename, const TagData *data);

/**
TODO: Add documention as sample given above
 */
int edit_tag(const char *filename, const char *tag, const char *value);

#endif // ID3_WRITER_H
