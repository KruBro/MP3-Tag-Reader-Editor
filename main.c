/**
 * @file main.c
 * @brief Main entry point for the MP3 Tag Reader application.
 * 
 * This program provides command-line functionality for:
 * - Viewing ID3 tags in MP3 files
 * - Editing individual tag fields
 * - Displaying help information
 * 
 * The application supports ID3v2.3 and ID3v2.4 tags and can modify
 * standard text frames including title, artist, album, year, comment, and genre.
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "id3_reader.h"
#include "id3_writer.h"
#include "error_handling.h"

/**
 * @brief Displays the help message for the MP3 Tag Reader application.
 * 
 * Shows all available command-line options and their usage patterns.
 * 
 * Supported operations:
 * - View tags: mp3tagreader -v filename.mp3
 * - Edit tags: mp3tagreader -e -[option] "value" filename.mp3
 * - Help: mp3tagreader -h
 * 
 * Available edit options:
 * - -t : Edit title
 * - -a : Edit artist
 * - -A : Edit album
 * - -y : Edit year
 * - -c : Edit comment
 * - -g : Edit genre
 */
void display_help() {
    printf("Usage: mp3tagreader [options] filename\n");
    printf("Options:\n");
    printf(" \t-h\t\t\tDisplay help\n");
    printf(" \t-v\t\t\tView tags\n");
    printf("\t-e -t <value>\t\tTo Edit Title Name\n\t-e -a <value>\t\tTo Edit Artist Name\n\t-e -A <value>\t\tTo Edit Album Name \n",
        "\t-e -y <value>\t\tTo Edit Year of Release\n\t-e -c <value>\t\tTo Edit/Add Comment\n\t-e -g <value>\t\tTo Edit Genre \n");
}

/**
 * @brief Main function to handle command-line arguments and execute appropriate actions.
 * 
 * This function parses command-line arguments and routes execution to the
 * appropriate handler based on the provided options.
 * 
 * Command patterns:
 * 
 * 1. Help display:
 *    mp3tagreader -h
 * 
 * 2. View tags:
 *    mp3tagreader -v filename.mp3
 * 
 * 3. Edit tag:
 *    mp3tagreader -e -[tag] "new value" filename.mp3
 *    Example: mp3tagreader -e -a "The Beatles" song.mp3
 * 
 * Argument validation:
 * - Minimum 2 arguments required (program name + option)
 * - View mode requires exactly 3 arguments
 * - Edit mode requires exactly 5 arguments
 * 
 * @param argc Argument count (number of command-line arguments).
 * @param argv Argument vector (array of argument strings).
 * 
 * @return 0 on success, 1 on failure or invalid arguments.
 * 
 * @example
 * // View tags
 * main(3, {"mp3tagreader", "-v", "song.mp3"})
 * 
 * // Edit artist
 * main(5, {"mp3tagreader", "-e", "-a", "New Artist", "song.mp3"})
 */
int main(int argc, char *argv[]) {
    /* Validate minimum argument count */
    if (argc < 2) {
        display_help();
        return 1;
    }

    /* ===== Help Option ===== */
    if (strcmp(argv[1], "-h") == 0) {
        display_help();
        return 0;
    }
    
    /* ===== View Tags Option ===== */
    else if (strcmp(argv[1], "-v") == 0 && argc == 3) {
        /* Arguments: [program] [-v] [filename] */
        view_tags(argv[2]);
    }
    
    /* ===== Edit Tag Option ===== */
    else if (strcmp(argv[1], "-e") == 0 && argc == 5) {
        /* Arguments: [program] [-e] [tag_option] [value] [filename] */
        
        char *tag = argv[2];       /* Tag option (e.g., "-a" for artist) */
        char *value = argv[3];     /* New value (e.g., "The Beatles") */
        char *filename = argv[4];  /* File to modify (e.g., "song.mp3") */

        /* Attempt to edit the specified tag */
        if (edit_tag(filename, tag, value) != 0) {
            display_error("Failed to edit tag.");
            return 1;
        }
        
        printf("Tag edited successfully.\n");
    }
    
    /* ===== Invalid Arguments ===== */
    else {
        display_help();
        return 1;
    }

    return 0;
}