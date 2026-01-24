/**
 * @file id3_utils.c
 * @brief Implementation of utility functions for ID3 tag manipulation.
 * 
 * This file provides core utilities for:
 * - Memory management of TagData structures
 * - Byte order conversions (big-endian and synchsafe)
 * - Integer encoding/decoding for ID3 specifications
 */

#include <stdlib.h>
#include <string.h>
#include "id3_utils.h"

/**
 * @brief Creates and initializes a new TagData structure.
 * 
 * Uses calloc to allocate and zero-initialize the structure, ensuring
 * all pointer fields start as NULL and numeric fields as 0.
 * 
 * @return Pointer to the newly created TagData structure.
 */
TagData* create_tag_data() {
    TagData *data = (TagData *)calloc(1, sizeof(TagData));
    return data;
}

/**
 * @brief Frees all memory associated with a TagData structure.
 * 
 * Safely deallocates all dynamically allocated fields within the structure,
 * then frees the structure itself. NULL checks ensure this function is safe
 * to call even with partially initialized or NULL structures.
 * 
 * @param data Pointer to the TagData structure to free.
 */
void free_tag_data(TagData *data) {
    if (data) {
        /* Free all text metadata fields */
        free(data->version);
        free(data->title);
        free(data->artist);
        free(data->album);
        free(data->year);
        free(data->comment);
        free(data->genre);
        
        /* Free album artwork data if present */
        if ((data->album_art.mime_type) != NULL) {
            free(data->album_art.mime_type);
        }
        if ((data->album_art.image_data) != NULL) {
            free(data->album_art.image_data);
        }
        
        /* Free the structure itself */
        free(data);
    }
}

/**
 * @brief Converts a 4-byte big-endian value to host byte order.
 * 
 * Big-endian format stores the most significant byte first:
 * [MSB] [byte2] [byte3] [LSB]
 * 
 * This function reassembles the bytes into a 32-bit integer by:
 * - Shifting byte 0 left 24 bits (becomes MSB)
 * - Shifting byte 1 left 16 bits
 * - Shifting byte 2 left 8 bits
 * - Byte 3 remains as LSB (no shift)
 * 
 * @param bytes Pointer to 4-byte array in big-endian order.
 * @return 32-bit unsigned integer in host byte order.
 */
uint32_t big_endian_to_host(uint8_t *bytes) {
    uint32_t val = 0;
    
    /* Iterate through each byte and position it correctly */
    for (int i = 0; i < 4; i++) {
        /* Shift starts at 24 and decreases by 8 each iteration: 24, 16, 8, 0 */
        val |= (uint32_t)bytes[i] << (24 - (i * 8));
    }
    
    return val;
}

/**
 * @brief Converts a 4-byte synchsafe integer to a regular integer.
 * 
 * Synchsafe integers use only 7 bits per byte (bit 7 is always 0) to avoid
 * conflicts with MPEG audio synchronization patterns.
 * 
 * Format: 0xxxxxxx 0xxxxxxx 0xxxxxxx 0xxxxxxx
 * Total usable bits: 7 * 4 = 28 bits
 * 
 * Conversion process:
 * - Byte 0: bits 27-21 (shift left 21)
 * - Byte 1: bits 20-14 (shift left 14)
 * - Byte 2: bits 13-7  (shift left 7)
 * - Byte 3: bits 6-0   (shift left 0)
 * 
 * @param bytes Pointer to 4-byte synchsafe integer.
 * @return Decoded 32-bit unsigned integer.
 */
uint32_t synchsafe_to_int(uint8_t *bytes) {
    uint32_t val = 0;
    
    /* Iterate through each byte and extract 7 bits */
    for (int i = 0; i < 4; i++) {
        /* Shift starts at 21 and decreases by 7 each iteration: 21, 14, 7, 0 */
        val |= (uint32_t)bytes[i] << (21 - (i * 7));
    }
    
    return val;
}

/**
 * @brief Converts a regular integer to a 4-byte synchsafe integer.
 * 
 * This function encodes a 32-bit integer into synchsafe format by:
 * 1. Extracting 7-bit chunks from the input value
 * 2. Ensuring bit 7 of each byte is 0
 * 3. Storing the result in big-endian order
 * 
 * The encoding distributes the lower 28 bits of the input across 4 bytes,
 * with each byte containing 7 bits of data.
 * 
 * @param num The integer value to encode (only lower 28 bits are used).
 * @param out_bytes Output buffer for 4 synchsafe bytes (must be pre-allocated).
 */
void int_to_synchsafe(uint32_t num, uint8_t* out_bytes) {
    /* Mask to extract 7 bits (0x7F = 0b01111111) */
    unsigned int mask = 0x7F;
    
    /* Extract and store each 7-bit chunk */
    for (int i = 0; i < 4; i++) {
        /* Shift right by: 21, 14, 7, 0 and mask to get 7 bits */
        out_bytes[i] = (num >> (21 - (i * 7))) & mask;
    }
}