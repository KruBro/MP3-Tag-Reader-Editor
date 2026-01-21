/**
 * @file main.c
 * @brief Main entry point for the MP3 Tag Reader application.
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "id3_reader.h"
#include "id3_writer.h"
#include "error_handling.h"

/**
 * @brief Displays the help message for the MP3 Tag Reader application.
 */
void display_help() {
    printf("Usage: mp3tagreader [options] filename\n");
    printf("Options:\n");
    printf("  -h               Display help\n");
    printf("  -v               View tags\n");
    printf("  -e -t/-T/-a/-A/-y/-c/-g <value>  Edit tags\n");
}

/**
 * @brief Main function to handle command-line arguments and execute appropriate actions.
 * 
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, non-zero on failure.
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        display_help();
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0) {
        display_help();
        return 0;
    } else if (strcmp(argv[1], "-v") == 0 && argc == 3) {
        view_tags(argv[2]);
    } else if (strcmp(argv[1], "-e") == 0 && argc == 5) {
        char *tag = argv[2];       // e.g. "-a"
        char *value = argv[3];     // e.g. "The Beatles"
        char *filename = argv[4];  // e.g. "song.mp3"

        if (edit_tag(filename, tag, value) != 0) {
            display_error("Failed to edit tag.");
            return 1;
        }
        printf("Tag edited successfully.\n");
    } else {
        display_help();
        return 1;
    }

    return 0;
}
