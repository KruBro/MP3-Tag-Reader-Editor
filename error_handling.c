#include <stdio.h>
#include "error_handling.h"

/**
 * @brief Displays an error message to standard error.
 * @param message The error message to display.
 */
void display_error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
}

/**
 * @brief Checks if ID3 tags are present in the file.
 * (Placeholder implementation as per header declaration)
 */
int check_id3_tag_presence(const char *filename) {
    // 1. Define a raw buffer
    char header[10];
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        display_error("File Could Not Open");
        return 0; // Failure
    }

    // 2. Read exactly 10 bytes
    if (fread(header, 1, 10, fp) != 10) {
        display_error("File is too small to be a valid MP3/ID3 File");
        fclose(fp);
        return 0; // Failure
    }

    fclose(fp); // Close now, we have what we need for the check

    // 3. Validate Logic: Positive Check
    // We check indices 0, 1, AND 2. All must match.
    if (header[0] == 'I' && header[1] == 'D' && header[2] == '3') {
        // OPTIONAL: Check version here too. 
        // ID3v2.3 is stored as: header[3] = 0x03, header[4] = 0x00
        if (header[3] != 0x03) {
             display_error("Version not supported (We only handle ID3v2.3)");
             return 0;
        }
        
        // It is valid!
        return 1; 
    } else {
        display_error("Signature mismatch. Not an ID3 file.");
        return 0; 
    }
}