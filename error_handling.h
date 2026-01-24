#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

/**
 * @file error_handling.h
 * @brief Error handling and validation utilities for MP3/ID3 file operations.
 * 
 * This module provides functions for displaying error messages and validating
 * ID3 tag presence in MP3 files.
 */

/**
 * @brief Displays an error message to standard error output.
 * 
 * Formats and prints the given error message to stderr with an "Error: " prefix.
 * This function is used throughout the application for consistent error reporting.
 * 
 * @param message The error message to display (null-terminated string).
 * 
 * @note The message is printed to stderr rather than stdout to allow proper
 *       separation of error messages from normal program output.
 * 
 * @example
 * display_error("File not found");
 * // Output to stderr: "Error: File not found\n"
 */
void display_error(const char *message);

/**
 * @brief Checks if a valid ID3v2.3 tag is present in the specified file.
 * 
 * This function validates that:
 * 1. The file can be opened and read
 * 2. The file is at least 10 bytes (minimum size for ID3 header)
 * 3. The file begins with the "ID3" signature
 * 4. The tag version is ID3v2.3 (other versions are not supported)
 * 
 * @param filename Path to the MP3 file to check (null-terminated string).
 * 
 * @return 1 if a valid ID3v2.3 tag is present, 0 otherwise.
 * 
 * @note This function only supports ID3v2.3. Files with other versions
 *       (including ID3v2.4) will be rejected.
 * 
 * @warning The function will display appropriate error messages via 
 *          display_error() if validation fails.
 * 
 * @example
 * if (check_id3_tag_presence("song.mp3")) {
 *     // Proceed with reading tags
 * } else {
 *     // Handle invalid or missing ID3 tag
 * }
 */
int check_id3_tag_presence(const char *filename);

#endif // ERROR_HANDLING_H