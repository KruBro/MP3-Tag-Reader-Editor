# Contributing to MP3 ID3 Tag Reader & Editor

First off, thank you for considering contributing to this project! It's people like you that make this tool better for everyone.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Setup](#development-setup)
- [Coding Standards](#coding-standards)
- [Commit Guidelines](#commit-guidelines)
- [Pull Request Process](#pull-request-process)
- [Testing Guidelines](#testing-guidelines)
- [Documentation](#documentation)

---

## 📜 Code of Conduct

This project adheres to a code of conduct that we expect all contributors to follow. Please be respectful and constructive in your interactions.

### Our Standards

- **Be Respectful**: Treat everyone with respect and kindness
- **Be Collaborative**: Work together and help others
- **Be Professional**: Keep discussions focused on the project
- **Be Patient**: Remember that everyone was a beginner once

---

## 🤝 How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check existing issues to avoid duplicates.

**When reporting a bug, include:**

- **Clear title**: Summarize the issue in one line
- **Description**: Detailed explanation of the problem
- **Steps to reproduce**: Step-by-step instructions
- **Expected behavior**: What you expected to happen
- **Actual behavior**: What actually happened
- **Environment**: OS, compiler version, etc.
- **Test file**: Sample MP3 file if relevant (ensure no copyright issues)

**Example Bug Report:**

```markdown
### Bug: UTF-16 text truncated at null byte

**Description**: When reading UTF-16 encoded tags, the text is cut off at the first null byte.

**Steps to Reproduce**:
1. Create MP3 with UTF-16 encoded title: "Test Song"
2. Run `./mp3_tag_editor -v test.mp3`
3. Observe truncated output

**Expected**: "Test Song"
**Actual**: "T"

**Environment**: Ubuntu 22.04, GCC 11.3.0

**Additional Context**: Seems related to strlen() stopping at null bytes in UTF-16 encoding.
```

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues.

**When suggesting an enhancement, include:**

- **Clear title**: What feature you want
- **Motivation**: Why this feature is useful
- **Detailed description**: How it should work
- **Examples**: Usage examples
- **Alternatives**: Other solutions you've considered

**Example Enhancement Suggestion:**

```markdown
### Enhancement: Add JSON export functionality

**Motivation**: Would be useful to export metadata to JSON for integration with other tools.

**Description**: Add a `-j` or `--json` flag that outputs all tags in JSON format instead of the current text format.

**Example Usage**:
```bash
./mp3_tag_editor -j song.mp3 > metadata.json
```

**Output Format**:
```json
{
  "version": "2.3",
  "title": "Song Name",
  "artist": "Artist Name",
  "album": "Album Name",
  "year": "2024"
}
```
```

### Contributing Code

1. **Fork the repository**
2. **Create a feature branch** (`git checkout -b feature/AmazingFeature`)
3. **Make your changes**
4. **Test thoroughly**
5. **Commit your changes** (see commit guidelines)
6. **Push to your fork** (`git push origin feature/AmazingFeature`)
7. **Open a Pull Request**

---

## 🛠️ Development Setup

### Prerequisites

```bash
# Install required tools (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential gcc make git

# Verify installation
gcc --version
make --version
```

### Clone and Build

```bash
# Clone your fork
git clone https://github.com/KruBro/MP3-Tag-Reader-Editor.git
cd mp3-id3-tag-editor

# Add upstream remote
git remote add upstream https://github.com/KruBro/MP3-Tag-Reader-Editor.git

# Build the project
make clean
make

# Run tests
./mp3_tag_editor -h
```

### Staying Synchronized

```bash
# Fetch upstream changes
git fetch upstream

# Merge upstream changes into your main branch
git checkout main
git merge upstream/main

# Push to your fork
git push origin main
```

---

## 💻 Coding Standards

### C Style Guidelines

We follow the **K&R (Kernighan and Ritchie)** C style with some modifications.

#### Indentation

- **4 spaces** (no tabs)
- Indent each nested block

```c
// Good
if (condition) {
    do_something();
    if (nested_condition) {
        do_nested_thing();
    }
}

// Bad
if (condition) {
do_something();  // Not indented
    if (nested_condition) {
do_nested_thing();  // Inconsistent
    }
}
```

#### Braces

- Opening brace on same line for functions
- Opening brace on same line for control structures

```c
// Good
void my_function(void) {
    // function body
}

if (condition) {
    // code
} else {
    // code
}

// Bad
void my_function(void)
{  // Brace on new line
    // function body
}
```

#### Naming Conventions

- **Functions**: `snake_case` (e.g., `read_id3_tags`)
- **Variables**: `snake_case` (e.g., `frame_size`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_BUFFER_SIZE`)
- **Structs**: `PascalCase` (e.g., `TagData`)
- **Typedefs**: `PascalCase` (e.g., `typedef struct TagData`)

```c
// Good
#define MAX_FRAME_SIZE 1024
uint32_t frame_size = 0;
TagData *tag_data = NULL;

void parse_frame_header(uint8_t *header) {
    // function body
}

// Bad
#define maxFrameSize 1024  // Should be UPPER_SNAKE_CASE
uint32_t FrameSize = 0;    // Should be snake_case
void ParseFrameHeader(uint8_t *header) {  // Should be snake_case
    // function body
}
```

#### Comments

- Use `/* */` for multi-line comments
- Use `//` for single-line comments
- Document all public functions
- Explain complex algorithms

```c
/* Good: Clear function documentation */
/**
 * @brief Converts synchsafe integer to regular integer
 * 
 * @param bytes Pointer to 4-byte synchsafe integer
 * @return Decoded 32-bit unsigned integer
 */
uint32_t synchsafe_to_int(uint8_t *bytes) {
    // Extract 7 bits from each byte
    return (bytes[0] << 21) | (bytes[1] << 14) | 
           (bytes[2] << 7)  | (bytes[3] << 0);
}

// Bad: No documentation
uint32_t synchsafe_to_int(uint8_t *bytes) {
    return (bytes[0] << 21) | (bytes[1] << 14) | 
           (bytes[2] << 7)  | (bytes[3] << 0);
}
```

#### Function Length

- Keep functions under **100 lines**
- If longer, consider breaking into smaller functions
- Each function should do **one thing well**

#### Error Handling

- Check return values
- Handle NULL pointers
- Use consistent error reporting

```c
// Good
FILE *fp = fopen(filename, "rb");
if (fp == NULL) {
    display_error("Failed to open file");
    return NULL;
}

// Bad
FILE *fp = fopen(filename, "rb");
// No error checking!
```

### Memory Management

- **Always** free allocated memory
- Set pointers to NULL after freeing
- Use `calloc()` for zero-initialization
- Check allocation success

```c
// Good
char *buffer = malloc(size);
if (buffer == NULL) {
    return -1;  // Handle allocation failure
}

// ... use buffer ...

free(buffer);
buffer = NULL;  // Prevent dangling pointer

// Bad
char *buffer = malloc(size);
// ... use buffer ...
free(buffer);
// buffer still points to freed memory!
```

---

## 📝 Commit Guidelines

### Commit Message Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

#### Types

- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Code style changes (formatting)
- **refactor**: Code refactoring
- **test**: Adding or modifying tests
- **chore**: Build process or auxiliary tool changes

#### Examples

```bash
# Feature
git commit -m "feat(reader): Add support for TDRC frame in ID3v2.4"

# Bug fix
git commit -m "fix(writer): Correct synchsafe integer encoding for frame sizes"

# Documentation
git commit -m "docs(readme): Update installation instructions for macOS"

# Refactoring
git commit -m "refactor(utils): Simplify byte order conversion functions"
```

#### Detailed Commit Message

```
feat(reader): Add UTF-8 text encoding support

- Implement UTF-8 decoder for ID3v2.4 tags
- Add encoding byte 0x03 handling
- Update display_metadata() to use new decoder
- Add unit tests for UTF-8 decoding

Closes #15
```

---

## 🔄 Pull Request Process

### Before Submitting

1. **Test your changes** thoroughly
2. **Update documentation** if needed
3. **Follow coding standards**
4. **Write clear commit messages**
5. **Ensure code compiles** without warnings

### PR Checklist

- [ ] Code compiles without warnings (`gcc -Wall -Wextra`)
- [ ] All tests pass
- [ ] Code follows project style guidelines
- [ ] Documentation is updated
- [ ] Commit messages are clear and follow guidelines
- [ ] No unnecessary files included (.o, executables, etc.)
- [ ] Changes are focused (one feature/fix per PR)

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Code refactoring

## Testing
Describe how you tested your changes

## Related Issues
Fixes #(issue number)

## Screenshots (if applicable)
Add screenshots for UI changes

## Checklist
- [ ] Code compiles without warnings
- [ ] Tests pass
- [ ] Documentation updated
- [ ] Follows coding standards
```

### Review Process

1. Maintainer reviews your PR
2. Address any requested changes
3. Once approved, PR will be merged
4. Your contribution will be credited

---

## 🧪 Testing Guidelines

### Manual Testing

```bash
# Build the project
make clean
make

# Test basic functionality
./mp3_tag_editor -v test.mp3

# Test editing
./mp3_tag_editor -e -t "Test Title" test.mp3
./mp3_tag_editor -v test.mp3

# Test deletion
./mp3_tag_editor -d test.mp3 -c
./mp3_tag_editor -v test.mp3

# Test album art extraction
./mp3_tag_editor -x test.mp3 cover.jpg
file cover.jpg
```

### Test Cases to Verify

- [ ] Reading tags from ID3v2.3 files
- [ ] Reading tags from ID3v2.4 files
- [ ] Editing each tag type (title, artist, etc.)
- [ ] Deleting specific tags
- [ ] Stripping all tags
- [ ] Extracting album art
- [ ] Handling files without tags
- [ ] Handling corrupted files gracefully
- [ ] UTF-16 encoded text
- [ ] Very long tag values

### Creating Test Files

```bash
# Use ffmpeg to create test MP3s
ffmpeg -f lavfi -i sine=frequency=440:duration=5 -c:a libmp3lame test_sine.mp3

# Add tags using id3v2 tool (for testing)
id3v2 -t "Test Title" -a "Test Artist" test_sine.mp3
```

### Memory Leak Testing

```bash
# Install valgrind
sudo apt-get install valgrind

# Run with valgrind
valgrind --leak-check=full --show-leak-kinds=all ./mp3_tag_editor -v test.mp3

# Should show: "All heap blocks were freed -- no leaks are possible"
```

---

## 📚 Documentation

### Code Documentation

- Document all public functions
- Use Doxygen-style comments
- Explain complex algorithms
- Include examples where helpful

### README Updates

When adding features, update:
- Feature list
- Usage examples
- Command reference
- Known limitations

### Wiki Contributions

Consider adding to the project wiki:
- Detailed tutorials
- Architecture explanations
- Troubleshooting guides
- FAQ entries

---

## 🎯 Good First Issues

Looking for a place to start? Check out issues labeled:
- `good first issue`
- `beginner friendly`
- `documentation`
- `help wanted`

---

## 💬 Questions?

- **General questions**: Open a discussion
- **Bug reports**: Open an issue
- **Feature ideas**: Open an issue with `enhancement` label
- **Security issues**: Email directly (see README)

---

## 🙏 Thank You!

Your contributions make this project better. Whether it's a bug report, feature suggestion, or code contribution, we appreciate your effort!

---

**Happy Coding! 🚀**
