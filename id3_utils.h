#ifndef ID3_UTILS_H
#define ID3_UTILS_H
#include <stdint.h>

/**
 * @brief Structure to hold ID3 tag data.
 */

 typedef struct
 {
    char file_identifier[3];
    uint16_t version;
    uint8_t flags;
    uint32_t size;
 } ID3Header;

 typedef struct
 {
    char frame_id[4];
    uint32_t size;
    uint16_t flags;
 } FrameHeader;
 
typedef struct {
    char *version; /**< Version of the ID3 tag */
    char *title;   /**< Title of the song */
    char *artist;  /**< Artist of the song */
    char *album;   /**< Album name */
    char *year;    /**< Year of release */
    char *comment; /**< Comment */
    char *genre;   /**< Genre */
    // Add other fields as needed
    uint16_t tag_version;
    uint32_t padding_size;
    struct{
        char *mime_type;
        uint32_t image_size;
        unsigned char *image_data;
    } album_art;
    
} TagData;

/**
 * @brief Creates a new TagData structure.
 * 
 * @return Pointer to the newly created TagData structure.
 */
TagData* create_tag_data();

/**
TODO: Add documention as sample given above
 */
void free_tag_data(TagData *data);

uint32_t big_endian_to_host(uint8_t *bytes);
uint32_t synchsafe_to_int(uint8_t *bytes);
void int_to_synchsafe(uint32_t num, uint8_t* out_bytes);

#endif // ID3_UTILS_H
