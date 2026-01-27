/**
 * ============================================================================
 * @file main.c
 * @brief Main entry point for the MP3 Tag Reader & Editor application
 * 
 * This application provides a command-line interface for:
 *   - Viewing ID3 tags in MP3 files
 *   - Editing individual tag fields
 *   - Deleting specific tags or all tags
 *   - Extracting embedded album artwork
 * 
 * @details
 * Command-Line Interface:
 *   The program uses a flexible argument parsing system that supports
 *   multiple operations with various tag identifiers.
 * 
 * Supported Operations:
 *   - View:    Display all ID3 tags
 *   - Edit:    Modify a specific tag field
 *   - Delete:  Remove a specific tag field
 *   - Strip:   Remove all ID3 tags
 *   - Extract: Save album art to an image file
 * 
 * @author  [Your Name]
 * @version 1.0
 * @date    2024
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "id3_reader.h"
#include "id3_writer.h"
#include "error_handling.h"

/* ============================================================================
 * HELP AND USAGE DISPLAY
 * ============================================================================
 */

/**
 * @brief Displays comprehensive help information for the application
 * 
 * @details
 * This function prints a formatted help screen that includes:
 *   - Usage syntax
 *   - All available command-line options
 *   - Tag identifiers for edit/delete operations
 *   - Practical usage examples
 * 
 * Help Screen Structure:
 *   ┌──────────────────────────────────────┐
 *   │ 1. Header banner                     │
 *   ├──────────────────────────────────────┤
 *   │ 2. Usage syntax                      │
 *   ├──────────────────────────────────────┤
 *   │ 3. Standard options                  │
 *   │    - View tags                       │
 *   │    - Help screen                     │
 *   ├──────────────────────────────────────┤
 *   │ 4. Modification options              │
 *   │    - Edit, delete, extract           │
 *   ├──────────────────────────────────────┤
 *   │ 5. Tag identifiers table             │
 *   ├──────────────────────────────────────┤
 *   │ 6. Usage examples                    │
 *   └──────────────────────────────────────┘
 * 
 * @note This function is called when:
 *       - User runs program with -h or --help flag
 *       - User provides invalid arguments
 *       - User runs program without arguments
 */
void display_help() {
    /* =========================================================================
     * Print formatted header banner
     * ========================================================================= */
    printf("\n==================================================================\n");
    printf("                  MP3 Tag Reader & Editor - Help                  \n");
    printf("==================================================================\n");
    
    /* =========================================================================
     * General usage syntax
     * ========================================================================= */
    printf("Usage: ./mp3_tag_editor [OPTION] [FILE] [...]\n\n");
    
    /* =========================================================================
     * Standard viewing and help options
     * ========================================================================= */
    printf("Standard Options:\n");
    printf("  -v, --view <file>             View all tags.\n");
    printf("  -h, --help                    Display this help screen.\n\n");
    
    /* =========================================================================
     * Modification options (edit, delete, extract)
     * ========================================================================= */
    printf("Modification Options:\n");
    printf("  -e, --edit <tag> <val> <file> Edit a specific tag.\n");
    printf("  -d, --delete <file> <tag>     Delete a specific tag.\n");
    printf("  -D, --delete-all <file>       Delete ALL ID3 tags (Strip).\n");
    printf("  -x, --extract <file> <name>   Extract album art to an image file.\n\n");
    
    /* =========================================================================
     * Tag identifiers reference table
     * 
     * These identifiers are used with -e (edit) and -d (delete) options
     * ========================================================================= */
    printf("Tag Identifiers (for -e and -d):\n");
    printf("  -t  Title      -a  Artist\n");
    printf("  -A  Album      -y  Year\n");
    printf("  -T  Track      -G  Genre\n");
    printf("  -c  Comment    -b  Album Artist\n");
    printf("  -m  Composer   -o  Copyright\n");
    printf("  -p  Publisher\n\n");
    
    /* =========================================================================
     * Practical usage examples
     * ========================================================================= */
    printf("Examples:\n");
    printf("  ./mp3_tag_editor -v song.mp3\n");
    printf("  ./mp3_tag_editor -e -t \"New Title\" song.mp3\n");
    printf("  ./mp3_tag_editor -d song.mp3 -c\n");
    printf("  ./mp3_tag_editor -x song.mp3 cover.jpg\n");
    printf("==================================================================\n\n");
}

/* ============================================================================
 * MAIN FUNCTION - COMMAND-LINE ARGUMENT PARSING
 * ============================================================================
 */

/**
 * @brief Main entry point of the application
 * 
 * @details
 * This function implements a flexible command-line parser that handles
 * multiple operations with validation.
 * 
 * Argument Parsing Strategy:
 *   1. Check minimum argument count
 *   2. Identify the operation from argv[1]
 *   3. Validate argument count for that operation
 *   4. Extract parameters from appropriate argv indices
 *   5. Execute the operation
 *   6. Handle errors and return appropriate status code
 * 
 * Supported Command Patterns:
 *   ┌────────────────────────────────────────┬──────────────┐
 *   │ Command Pattern                        │ argc Count   │
 *   ├────────────────────────────────────────┼──────────────┤
 *   │ ./program -h                           │ 2            │
 *   │ ./program -v <file>                    │ 3            │
 *   │ ./program -e <tag> <value> <file>      │ 5            │
 *   │ ./program -d <file> <tag>              │ 4            │
 *   │ ./program -D <file>                    │ 3            │
 *   │ ./program -x <file> <output>           │ 4            │
 *   └────────────────────────────────────────┴──────────────┘
 * 
 * @param[in] argc Argument count (number of command-line arguments)
 * @param[in] argv Argument vector (array of command-line argument strings)
 * 
 * @return 0 on success, 1 on failure or invalid arguments
 * 
 * @note argv[0] is always the program name
 * @note argv[1] is the operation flag (-v, -e, -d, etc.)
 * @note Remaining arguments depend on the operation
 */
int main(int argc, char *argv[]) {
    /* =========================================================================
     * VALIDATION: Check minimum argument count
     * 
     * At minimum, we need:
     *   argv[0]: program name
     *   argv[1]: operation flag
     * ========================================================================= */
    if (argc < 2) {
        display_help();
        return 1;
    }

    /* =========================================================================
     * OPERATION 1: Display Help
     * 
     * Command: ./mp3_tag_editor -h
     * Command: ./mp3_tag_editor --help
     * ========================================================================= */
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        display_help();
        return 0;
    }
    
    /* =========================================================================
     * OPERATION 2: View Tags
     * 
     * Command: ./mp3_tag_editor -v song.mp3
     * 
     * Arguments:
     *   argv[0]: program name
     *   argv[1]: "-v" or "--view"
     *   argv[2]: filename
     * 
     * Total argc: 3
     * ========================================================================= */
    else if ((strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--view") == 0) && argc == 3) {
        /* Extract filename from argv[2] */
        const char *filename = argv[2];
        
        /* Call view_tags to read and display all metadata */
        view_tags(filename);
    }
    
    /* =========================================================================
     * OPERATION 3: Edit Tag
     * 
     * Command: ./mp3_tag_editor -e -t "New Title" song.mp3
     * 
     * Arguments:
     *   argv[0]: program name
     *   argv[1]: "-e" or "--edit"
     *   argv[2]: tag identifier (e.g., "-t", "-a", "-A")
     *   argv[3]: new value (e.g., "New Title")
     *   argv[4]: filename
     * 
     * Total argc: 5
     * 
     * Note: Using order: program -e <tag> <value> <file>
     * ========================================================================= */
    else if ((strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--edit") == 0) && argc == 5) {
        /* Extract parameters */
        char *tag      = argv[2];  /* Tag identifier (e.g., "-t") */
        char *value    = argv[3];  /* New value (e.g., "My Song") */
        char *filename = argv[4];  /* MP3 filename */

        /* Attempt to edit the tag */
        if (edit_tag(filename, tag, value) != 0) {
            display_error("Failed to edit tag.");
            return 1;
        }
    }

    /* =========================================================================
     * OPERATION 4: Delete Specific Tag
     * 
     * Command: ./mp3_tag_editor -d song.mp3 -t
     * 
     * Arguments:
     *   argv[0]: program name
     *   argv[1]: "-d" or "--delete"
     *   argv[2]: filename
     *   argv[3]: tag identifier to delete (e.g., "-t", "-c")
     * 
     * Total argc: 4
     * 
     * Usage: ./mp3_tag_editor -d song.mp3 -t
     *        (deletes the title tag from song.mp3)
     * ========================================================================= */
    else if ((strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--delete") == 0) && argc == 4) {
        /* Extract parameters */
        char *filename = argv[2];  /* MP3 filename */
        char *tag      = argv[3];  /* Tag to delete (e.g., "-t") */

        /* Attempt to delete the specified tag */
        if (delete_tag(filename, tag) != 0) {
            display_error("Failed to delete tag.");
            return 1;
        }
    }

    /* =========================================================================
     * OPERATION 5: Delete ALL Tags (Strip)
     * 
     * Command: ./mp3_tag_editor -D song.mp3
     * 
     * Arguments:
     *   argv[0]: program name
     *   argv[1]: "-D" or "--delete-all"
     *   argv[2]: filename
     * 
     * Total argc: 3
     * 
     * Usage: ./mp3_tag_editor -D song.mp3
     *        (removes all ID3 metadata from song.mp3)
     * 
     * Warning: This operation is irreversible!
     * ========================================================================= */
    else if ((strcmp(argv[1], "-D") == 0 || strcmp(argv[1], "--delete-all") == 0) && argc == 3) {
        /* Extract filename */
        char *filename = argv[2];

        /* Attempt to delete all tags */
        if (delete_all_tags(filename) != 0) {
            display_error("Failed to delete all tags.");
            return 1;
        }
    }

    /* =========================================================================
     * OPERATION 6: Extract Album Art
     * 
     * Command: ./mp3_tag_editor -x song.mp3 cover.jpg
     * 
     * Arguments:
     *   argv[0]: program name
     *   argv[1]: "-x" or "--extract"
     *   argv[2]: MP3 filename (source)
     *   argv[3]: output image filename (destination)
     * 
     * Total argc: 4
     * 
     * Usage: ./mp3_tag_editor -x song.mp3 cover.jpg
     *        (extracts album art from song.mp3 to cover.jpg)
     * 
     * Note: The output filename should have the appropriate extension
     *       (.jpg, .png, etc.) matching the embedded image format
     * ========================================================================= */
    else if ((strcmp(argv[1], "-x") == 0 || strcmp(argv[1], "--extract") == 0) && argc == 4) {
        /* Extract parameters */
        char *filename    = argv[2];  /* MP3 filename */
        char *output_name = argv[3];  /* Output image filename */

        /* Attempt to extract album art */
        if (extract_album_art(filename, output_name) != 0) {
            display_error("Failed to extract album art.");
            return 1;
        }
    }
    
    /* =========================================================================
     * INVALID ARGUMENTS: Unknown command or incorrect argument count
     * 
     * This else clause catches:
     *   - Unknown operation flags
     *   - Valid operations with wrong number of arguments
     *   - Malformed commands
     * ========================================================================= */
    else {
        printf("Error: Invalid arguments or command format.\n");
        display_help();
        return 1;
    }

    /* =========================================================================
     * SUCCESS: Operation completed successfully
     * ========================================================================= */
    return 0;
}