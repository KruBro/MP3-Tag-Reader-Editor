/**
 * @file id3_utils.h
 * @brief Utility structures and functions for ID3 tag manipulation.
 * 
 * This module defines the core data structures used to represent ID3 tags
 * and provides utility functions for byte-order conversion and memory management.
 */

#ifndef ID3_UTILS_H
#define ID3_UTILS_H

#include <stdint.h>

/**
 * @struct ID3Header
 * @brief Represents the 10-byte ID3v2 tag header.
 * 
 * Structure of ID3v2 header:
 * - Bytes 0-2: "ID3" identifier
 * - Bytes 3-4: Version (major.minor)
 * - Byte 5: Flags
 * - Bytes 6-9: Tag size (synchsafe integer)
 */
typedef struct {
    char file_identifier[3];  /**< Always "ID3" for valid ID3v2 tags */
    uint16_t version;         /**< Version number (e.g., 0x0300 for v2.3) */
    uint8_t flags;            /**< Header flags (unsynchronization, etc.) */
    uint32_t size;            /**< Size of tag excluding header (synchsafe) */
} ID3Header;

/**
 * @struct FrameHeader
 * @brief Represents a 10-byte ID3v2 frame header.
 * 
 * Each frame in an ID3 tag has a header containing:
 * - Frame ID (4 characters, e.g., "TIT2" for title)
 * - Size (4 bytes)
 * - Flags (2 bytes)
 */
typedef struct {
    char frame_id[4];    /**< Four-character frame identifier */
    uint32_t size;       /**< Size of frame data (excluding header) */
    uint16_t flags;      /**< Frame flags (compression, encryption, etc.) */
} FrameHeader;

/**
 * @struct TagData
 * @brief Complete ID3 tag metadata container.
 * 
 * This structure holds all parsed ID3 tag information including:
 * - Text frames (title, artist, album, etc.)
 * - Version information
 * - Album artwork (if present)
 * 
 * @note Memory for string fields is dynamically allocated and must be
 *       freed using free_tag_data().
 */
typedef struct {
    /* Text metadata fields */
    char *version;       /**< Version string of the ID3 tag */
    char *title;         /**< Song title (TIT2 frame) */
    char *artist;        /**< Artist name (TPE1 frame) */
    char *album;         /**< Album name (TALB frame) */
    char *year;          /**< Year of release (TYER frame) */
    char *comment;       /**< Comment (COMM frame) */
    char *genre;         /**< Genre (TCON frame) */
    char *track;         /**< Track (TRCK frame) */
    
    char *album_artist;  // TPE2 (New)
    char *composer;      // TCOM (New)
    char *copyright;     // TCOP (New)
    char *publisher;     // TPUB (New)

    /* Tag metadata */
    uint16_t tag_version;    /**< Numeric version (3 for v2.3, 4 for v2.4) */
    uint32_t padding_size;   /**< Size of padding at end of tag */
    
    /* Album artwork data */
    struct {
        char *mime_type;            /**< MIME type (e.g., "image/jpeg") */
        uint32_t image_size;        /**< Size of image data in bytes */
        unsigned char *image_data;  /**< Raw image data */
    } album_art;
} TagData;

/**
 * @brief Creates and initializes a new TagData structure.
 * 
 * Allocates memory for a TagData structure and initializes all fields to zero.
 * This ensures that all pointers start as NULL, preventing accidental use of
 * uninitialized memory.
 * 
 * @return Pointer to newly allocated TagData structure, or NULL if allocation fails.
 * 
 * @note The returned structure must be freed with free_tag_data() when no longer needed.
 * 
 * @example
 * TagData *data = create_tag_data();
 * if (data) {
 *     // Use the structure
 *     free_tag_data(data);
 * }
 */
TagData* create_tag_data();

/**
 * @brief Frees all memory associated with a TagData structure.
 * 
 * This function safely deallocates:
 * - All text field strings
 * - Album art MIME type and image data
 * - The TagData structure itself
 * 
 * @param data Pointer to the TagData structure to free. Can be NULL (no-op).
 * 
 * @note This function checks for NULL before freeing each field, making it
 *       safe to use even with partially initialized structures.
 * 
 * @example
 * TagData *data = create_tag_data();
 * // ... use data ...
 * free_tag_data(data); // Safely frees all allocated memory
 */
void free_tag_data(TagData *data);

/**
 * @brief Converts a 4-byte big-endian value to host byte order.
 * 
 * ID3v2.3 uses big-endian (most significant byte first) for multi-byte integers.
 * This function converts such values to the host machine's native byte order.
 * 
 * @param bytes Pointer to array of 4 bytes in big-endian order.
 * @return 32-bit unsigned integer in host byte order.
 * 
 * @note Used for frame sizes in ID3v2.3 tags.
 * 
 * @example
 * uint8_t size_bytes[4] = {0x00, 0x00, 0x01, 0x00}; // 256 in big-endian
 * uint32_t size = big_endian_to_host(size_bytes);   // Returns 256
 */
uint32_t big_endian_to_host(uint8_t *bytes);

/**
 * @brief Converts a 4-byte synchsafe integer to a regular integer.
 * 
 * Synchsafe integers are used in ID3v2.4 to avoid confusion with MPEG audio
 * sync bytes (0xFF). Each byte only uses 7 bits (bit 7 is always 0).
 * 
 * Format: 0xxxxxxx 0xxxxxxx 0xxxxxxx 0xxxxxxx (28 usable bits)
 * 
 * @param bytes Pointer to array of 4 bytes in synchsafe format.
 * @return 32-bit unsigned integer value.
 * 
 * @note Used for tag and frame sizes in ID3v2.4, and tag size in ID3v2.3.
 * 
 * @example
 * uint8_t synch[4] = {0x00, 0x00, 0x02, 0x01}; // Synchsafe 257
 * uint32_t value = synchsafe_to_int(synch);     // Returns 257
 */
uint32_t synchsafe_to_int(uint8_t *bytes);

/**
 * @brief Converts a regular integer to a 4-byte synchsafe integer.
 * 
 * This is the inverse of synchsafe_to_int(). Converts a normal 32-bit integer
 * into the synchsafe format required by ID3v2.4.
 * 
 * @param num The integer value to convert (uses only lower 28 bits).
 * @param out_bytes Pointer to array where 4 synchsafe bytes will be written.
 * 
 * @note The output buffer must be at least 4 bytes in size.
 * 
 * @example
 * uint8_t output[4];
 * int_to_synchsafe(257, output);
 * // output now contains: {0x00, 0x00, 0x02, 0x01}
 */
void int_to_synchsafe(uint32_t num, uint8_t* out_bytes);

#endif // ID3_UTILS_H