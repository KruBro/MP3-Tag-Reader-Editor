/**
 * ============================================================================
 * @file id3_utils.c
 * @brief Implementation of utility functions for ID3 tag manipulation
 * 
 * This file provides core utilities for:
 *   - Memory management of TagData structures
 *   - Byte order conversions (big-endian and synchsafe)
 *   - Integer encoding/decoding for ID3 specifications
 * 
 * @details
 * These utilities are essential for ID3 tag processing because:
 *   1. ID3 uses network byte order (big-endian) which differs from
 *      most modern CPUs (little-endian)
 *   2. ID3v2.4 uses synchsafe integers to avoid MPEG sync conflicts
 *   3. Proper memory management prevents leaks in tag structures
 * ============================================================================
 */

#include <stdlib.h>
#include <string.h>
#include "id3_utils.h"

/* ============================================================================
 * MEMORY MANAGEMENT FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Creates and initializes a new TagData structure
 * 
 * @details
 * Implementation notes:
 *   - Uses calloc() instead of malloc() to zero-initialize all fields
 *   - Zero-initialization ensures all pointers are NULL
 *   - NULL pointers are safe for conditional checks and free()
 * 
 * @return Pointer to the newly created TagData structure
 * 
 * @note calloc() guarantees:
 *       - All bytes set to 0
 *       - All pointers set to NULL
 *       - All integers set to 0
 */
TagData* create_tag_data() {
    /* Allocate and zero-initialize the structure
     * calloc(count, size) allocates count * size bytes and sets them to 0 */
    TagData *data = (TagData *)calloc(1, sizeof(TagData));
    
    return data;
}

/**
 * @brief Frees all memory associated with a TagData structure
 * 
 * @details
 * Deallocation order:
 *   1. Free all text metadata fields (11 fields)
 *   2. Free album artwork MIME type string
 *   3. Free album artwork image data buffer
 *   4. Free the TagData structure itself
 * 
 * Safety features:
 *   - NULL check on input pointer
 *   - NULL checks before freeing each field
 *   - Prevents double-free errors
 * 
 * @param[in] data Pointer to the TagData structure to free
 * 
 * @note The free() function safely handles NULL pointers (does nothing),
 *       but we explicitly check for clarity and documentation
 */
void free_tag_data(TagData *data) {
    /* Guard against NULL pointer */
    if (data == NULL) {
        return;
    }
    
    /* -------------------------------------------------------------------------
     * Free standard text metadata fields
     * Note: free() is safe to call on NULL pointers, but we check explicitly
     *       for documentation purposes
     * ------------------------------------------------------------------------- */
    free(data->version);
    free(data->title);
    free(data->artist);
    free(data->album);
    free(data->year);
    free(data->comment);
    free(data->genre);
    free(data->track);
    
    /* -------------------------------------------------------------------------
     * Free extended text metadata fields
     * These fields might be NULL if not set, so we check explicitly
     * ------------------------------------------------------------------------- */
    if (data->album_artist) {
        free(data->album_artist);
    }
    if (data->composer) {
        free(data->composer);
    }
    if (data->copyright) {
        free(data->copyright);
    }
    if (data->publisher) {
        free(data->publisher);
    }
    
    /* -------------------------------------------------------------------------
     * Free album artwork data if present
     * These are binary data fields that may not always be allocated
     * ------------------------------------------------------------------------- */
    if (data->album_art.mime_type != NULL) {
        free(data->album_art.mime_type);
    }
    if (data->album_art.image_data != NULL) {
        free(data->album_art.image_data);
    }
    
    /* -------------------------------------------------------------------------
     * Free the structure itself
     * This must be done last, after all member fields are freed
     * ------------------------------------------------------------------------- */
    free(data);
}

/* ============================================================================
 * BYTE ORDER CONVERSION FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Converts a 4-byte big-endian value to host byte order
 * 
 * @details
 * Big-endian format stores the most significant byte first:
 *   [MSB] [byte 2] [byte 3] [LSB]
 * 
 * This function reassembles the bytes into a 32-bit integer:
 *   byte[0] << 24  |  (Move to highest position - bits 31-24)
 *   byte[1] << 16  |  (Move to position bits 23-16)
 *   byte[2] << 8   |  (Move to position bits 15-8)
 *   byte[3] << 0      (Already in position - bits 7-0)
 * 
 * Why loop instead of direct assignment?
 *   - More maintainable and readable
 *   - Easier to verify correctness
 *   - Compiler optimizes it anyway
 * 
 * @param[in] bytes Pointer to 4-byte array in big-endian order
 * 
 * @return 32-bit unsigned integer in host byte order
 * 
 * @par Example:
 * Input bytes:  [0x00] [0x00] [0x01] [0x00]
 * Calculation:  (0x00 << 24) | (0x00 << 16) | (0x01 << 8) | (0x00 << 0)
 * Result:       0x00000100 = 256 decimal
 */
uint32_t big_endian_to_host(uint8_t *bytes) {
    uint32_t val = 0;
    
    /* Iterate through each byte and position it correctly
     * i = 0: shift = 24 (MSB position)
     * i = 1: shift = 16
     * i = 2: shift = 8
     * i = 3: shift = 0  (LSB position) */
    for (int i = 0; i < 4; i++) {
        /* Calculate shift amount: starts at 24, decreases by 8 each iteration */
        int shift_amount = 24 - (i * 8);
        
        /* Cast byte to uint32_t (promote), shift to position, then OR into result */
        val |= (uint32_t)bytes[i] << shift_amount;
    }
    
    return val;
}

/**
 * @brief Converts a 4-byte synchsafe integer to a regular integer
 * 
 * @details
 * Synchsafe integers use only 7 bits per byte (bit 7 is always 0) to avoid
 * conflicts with MPEG audio synchronization patterns (0xFF bytes).
 * 
 * Format: 0xxxxxxx 0xxxxxxx 0xxxxxxx 0xxxxxxx
 *         └──7 bits──┘
 * Total usable bits: 7 * 4 = 28 bits
 * 
 * Conversion process (extracting 7 bits from each byte):
 *   byte[0]: bits 27-21  →  shift left 21 positions
 *   byte[1]: bits 20-14  →  shift left 14 positions
 *   byte[2]: bits 13-7   →  shift left 7 positions
 *   byte[3]: bits 6-0    →  shift left 0 positions (already in place)
 * 
 * Example:
 *   Input:  [0x00] [0x00] [0x02] [0x01]
 *           └─0─┘  └─0─┘  └─2─┘  └─1─┘
 *   Calculation:
 *     (0 << 21) | (0 << 14) | (2 << 7) | (1 << 0)
 *     = 0 | 0 | 256 | 1
 *     = 257
 * 
 * @param[in] bytes Pointer to 4-byte synchsafe integer
 * 
 * @return Decoded 32-bit unsigned integer
 * 
 * @note The input bytes must have bit 7 clear (0x00-0x7F range).
 *       If bit 7 is set, the result will be incorrect but no error is raised.
 */
uint32_t synchsafe_to_int(uint8_t *bytes) {
    uint32_t val = 0;
    
    /* Iterate through each byte and extract 7 bits
     * i = 0: shift = 21 (bits 27-21)
     * i = 1: shift = 14 (bits 20-14)
     * i = 2: shift = 7  (bits 13-7)
     * i = 3: shift = 0  (bits 6-0) */
    for (int i = 0; i < 4; i++) {
        /* Calculate shift amount: starts at 21, decreases by 7 each iteration */
        int shift_amount = 21 - (i * 7);
        
        /* Each byte contributes 7 bits to the final value
         * No masking needed here - we assume input is valid synchsafe
         * (bit 7 is already 0) */
        val |= (uint32_t)bytes[i] << shift_amount;
    }
    
    return val;
}

/**
 * @brief Converts a regular integer to a 4-byte synchsafe integer
 * 
 * @details
 * This function encodes a 32-bit integer into synchsafe format by:
 *   1. Extracting 7-bit chunks from the input value
 *   2. Ensuring bit 7 of each byte is 0 (via mask 0x7F)
 *   3. Storing the result in big-endian order
 * 
 * The encoding distributes the lower 28 bits of the input across 4 bytes,
 * with each byte containing 7 bits of data.
 * 
 * Bit extraction process:
 *   out_bytes[0] = (num >> 21) & 0x7F  (Extract bits 27-21)
 *   out_bytes[1] = (num >> 14) & 0x7F  (Extract bits 20-14)
 *   out_bytes[2] = (num >> 7)  & 0x7F  (Extract bits 13-7)
 *   out_bytes[3] = (num >> 0)  & 0x7F  (Extract bits 6-0)
 * 
 * Example:
 *   Input:  257 (0x00000101 in binary)
 *   Binary: 00000000 00000000 00000001 00000001
 *   
 *   Extraction:
 *     bits 27-21: 0000000 → 0x00
 *     bits 20-14: 0000000 → 0x00
 *     bits 13-7:  0000010 → 0x02
 *     bits 6-0:   0000001 → 0x01
 *   
 *   Output: [0x00] [0x00] [0x02] [0x01]
 * 
 * @param[in]  num       The integer value to encode (only lower 28 bits used)
 * @param[out] out_bytes Output buffer for 4 synchsafe bytes (must be pre-allocated)
 * 
 * @warning Values larger than 2^28 - 1 (268,435,455) will have their upper
 *          bits truncated. No overflow detection is performed.
 * 
 * @note The mask 0x7F (0b01111111) ensures bit 7 is always 0, making the
 *       output synchsafe-compliant
 */
void int_to_synchsafe(uint32_t num, uint8_t* out_bytes) {
    /* Mask to extract 7 bits (0x7F = 0b01111111)
     * This ensures bit 7 is always 0 in the output */
    const unsigned int mask = 0x7F;
    
    /* Extract and store each 7-bit chunk
     * i = 0: shift = 21, extract bits 27-21
     * i = 1: shift = 14, extract bits 20-14
     * i = 2: shift = 7,  extract bits 13-7
     * i = 3: shift = 0,  extract bits 6-0 */
    for (int i = 0; i < 4; i++) {
        /* Calculate shift amount: starts at 21, decreases by 7 each iteration */
        int shift_amount = 21 - (i * 7);
        
        /* Shift the bits into position, then mask to get only 7 bits */
        out_bytes[i] = (num >> shift_amount) & mask;
    }
}