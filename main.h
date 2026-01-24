/**
 * @file main.h
 * @brief Header file for main application entry point.
 * 
 * This file declares the interface for the main application module,
 * including the help/usage display function.
 */

#ifndef MAIN_H
#define MAIN_H

/**
 * @brief Displays the help message for the MP3 Tag Reader application.
 * 
 * Prints usage information to stdout, including:
 * - Command syntax
 * - Available options and their meanings
 * - Examples of proper usage
 * 
 * This function is called when:
 * - The user provides the -h flag
 * - Invalid arguments are provided
 * - No arguments are provided
 * 
 * Usage output format:
 * ```
 * Usage: mp3tagreader [options] filename
 * Options:
 *   -h               Display help
 *   -v               View tags
 *   -e -t/-T/-a/-A/-y/-c/-g <value>  Edit tags
 * ```
 * 
 * @note This function does not take any parameters and does not return a value.
 * 
 * @example
 * display_help();
 * // Prints usage information to stdout
 */
void display_help();

#endif // MAIN_H