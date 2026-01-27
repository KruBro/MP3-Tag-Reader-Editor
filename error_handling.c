/**
 * @file error_handling.c
 * @brief Implementation of error handling and ID3 tag validation functions.
 * 
 * This file implements utilities for error reporting and ID3 tag validation,
 * providing a consistent interface for error handling throughout the application.
 */

#include <stdio.h>
#include "error_handling.h"

/**
 * @brief Displays an error message to standard error.
 * 
 * @param message The error message to display.
 */
void display_error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
}

/**
 * @brief Checks if ID3 tags are present in the file.
 * 
 * This function performs comprehensive validation of the ID3 tag:
 * - Opens the file and reads the 10-byte header
 * - Verifies the "ID3" signature at the beginning
 * - Checks that the version is ID3v2.3 (version byte = 0x03)
 * 
 * @param filename Path to the MP3 file to validate.
 * @return 1 if valid ID3v2.3 tag found, 0 otherwise.
 */
int check_id3_tag_presence(const char *filename) {
    /* 1. Define a buffer to hold the 10-byte ID3 header */
    char header[10];
    FILE *fp = fopen(filename, "rb");

    /* Check if file opened successfully */
    if (fp == NULL) {
        display_error("File Could Not Open");
        return 0; // Failure
    }

    /* 2. Read exactly 10 bytes from the file header */
    if (fread(header, 1, 10, fp) != 10) {
        display_error("File is too small to be a valid MP3/ID3 File");
        fclose(fp);
        return 0; // Failure
    }

    /* Close file - we have all the data needed for validation */
    fclose(fp);

    /* 3. Validate the ID3 signature and version */
    
    /* Check if header starts with "ID3" (bytes 0, 1, 2) */
    if (header[0] == 'I' && header[1] == 'D' && header[2] == '3') {
        
        /* Verify the version is ID3v2.3 */
        /* ID3v2.3 format: header[3] = 0x03, header[4] = 0x00 */
        if (header[3] != 0x03 && header[3] != 0x04) {
             display_error("Version not supported (Only ID3v2.3 and v2.4 supported)");
             return 0;
        }
        
        /* Valid ID3v2.3 tag found */
        return 1; 
    } else {
        /* Signature doesn't match "ID3" */
        display_error("Signature mismatch. Not an ID3 file.");
        return 0; 
    }
}