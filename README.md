# 🎵 MP3 ID3 Tag Reader & Editor

[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Standard](https://img.shields.io/badge/standard-C99-brightgreen.svg)](https://en.wikipedia.org/wiki/C99)
[![License](https://img.shields.io/badge/license-MIT-orange.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](https://www.linux.org/)

> A lightweight, dependency-free MP3 metadata parser and editor implemented in pure C, operating directly on the binary structure of ID3v2 tags.

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [Technical Highlights](#-technical-highlights)
- [Project Structure](#-project-structure)
- [Installation](#-installation)
- [Usage](#-usage)
- [ID3 Tag Support](#-id3-tag-support)
- [Architecture](#-architecture)
- [Examples](#-examples)
- [Limitations](#-limitations)
- [Contributing](#-contributing)
- [License](#-license)
- [References](#-references)

---

## 🎯 Overview

This project is a **binary-level MP3 metadata manipulation tool** built from scratch in C, designed to read, parse, and modify **ID3v2 tags** without relying on external libraries. It demonstrates low-level systems programming concepts including binary file I/O, endianness handling, and memory-safe parsing.

### Why This Project?

- **Educational Focus**: Learn how MP3 metadata actually works at the byte level
- **Zero Dependencies**: Pure C implementation with no external libraries
- **Production-Ready**: Handles real-world MP3 files safely and efficiently
- **Systems Programming**: Demonstrates file I/O, memory management, and binary parsing

---

## ✨ Features

### Core Functionality

- ✅ **Read ID3 Tags**: Extract metadata from MP3 files
- ✅ **Edit Tags**: Modify individual metadata fields
- ✅ **Delete Tags**: Remove specific fields or strip all metadata
- ✅ **Album Art**: Extract embedded cover images
- ✅ **Version Support**: Full ID3v2.3 and ID3v2.4 compatibility
- ✅ **Safe Writing**: Atomic file updates using temporary files
- ✅ **Text Encoding**: Handle ISO-8859-1 and UTF-16 encodings

### Technical Features

- 🔐 **Memory Safe**: Proper allocation/deallocation tracking
- 🛡️ **Error Handling**: Comprehensive validation and error reporting
- 🔄 **Endianness Aware**: Correct big-endian and synchsafe integer handling
- 📦 **Modular Design**: Clean separation of concerns across modules
- 📝 **Well Documented**: Extensive inline documentation and comments

---

## 🔬 Technical Highlights

### Binary Parsing Concepts

```c
// Synchsafe Integer Decoding (ID3v2.4)
// Bits:  0xxxxxxx 0xxxxxxx 0xxxxxxx 0xxxxxxx
// Used to avoid MPEG sync conflicts (0xFF bytes)
uint32_t synchsafe_to_int(uint8_t *bytes) {
    return (bytes[0] << 21) | (bytes[1] << 14) | 
           (bytes[2] << 7)  | (bytes[3] << 0);
}

// Big-Endian to Host Conversion (ID3v2.3)
uint32_t big_endian_to_host(uint8_t *bytes) {
    return (bytes[0] << 24) | (bytes[1] << 16) | 
           (bytes[2] << 8)  | (bytes[3] << 0);
}
```

### ID3v2 Structure

```
┌────────────────────────────────┐
│  ID3v2 Header (10 bytes)       │
├────────────────────────────────┤
│  Frame 1 Header (10 bytes)     │
│  Frame 1 Data (variable)       │
├────────────────────────────────┤
│  Frame 2 Header (10 bytes)     │
│  Frame 2 Data (variable)       │
├────────────────────────────────┤
│  ...                           │
├────────────────────────────────┤
│  Padding (optional)            │
└────────────────────────────────┘
```

---

## 📁 Project Structure

```
mp3-id3-tag-editor/
│
├── 📄 main.c                    # Main entry point and CLI parser
├── 📄 main.h                    # Main header declarations
│
├── 📄 id3_reader.c              # ID3 tag reading and parsing
├── 📄 id3_reader.h              # Reader function declarations
│
├── 📄 id3_writer.c              # ID3 tag writing and editing
├── 📄 id3_writer.h              # Writer function declarations
│
├── 📄 id3_utils.c               # Utility functions (byte conversion, memory)
├── 📄 id3_utils.h               # Utility data structures and declarations
│
├── 📄 error_handling.c          # Error reporting and validation
├── 📄 error_handling.h          # Error handling declarations
│
├── 📄 Makefile                  # Build configuration
└── 📄 README.md                 # This file
```

### Module Responsibilities

| Module | Purpose |
|--------|---------|
| **main** | Command-line interface and argument parsing |
| **id3_reader** | Binary parsing of ID3 tags and frames |
| **id3_writer** | Writing and modifying ID3 tags |
| **id3_utils** | Data structures, memory management, byte conversions |
| **error_handling** | Consistent error reporting and validation |

---

## 🛠️ Installation

### Prerequisites

- **GCC** (GNU Compiler Collection) or compatible C compiler
- **Make** build tool
- **Linux/Unix** environment (or WSL on Windows)

### Build from Source

```bash
# Clone the repository
git clone https://github.com/KruBro/MP3-Tag-Reader-Editor.git
cd mp3-id3-tag-editor

# Build using Make
make

# Or compile manually
gcc -Wall -Wextra -std=c99 -o mp3_tag_editor \
    main.c id3_reader.c id3_writer.c id3_utils.c error_handling.c
```

### Verify Installation

```bash
# Display help
./mp3_tag_editor -h

# Test with an MP3 file
./mp3_tag_editor -v your_song.mp3
```

---

## 📖 Usage

### Command Syntax

```bash
./mp3_tag_editor [OPTION] [ARGUMENTS]
```

### Available Operations

#### 1. View Tags

Display all metadata from an MP3 file:

```bash
./mp3_tag_editor -v song.mp3
./mp3_tag_editor --view song.mp3
```

#### 2. Edit Tags

Modify individual metadata fields:

```bash
# Edit title
./mp3_tag_editor -e -t "New Song Title" song.mp3

# Edit artist
./mp3_tag_editor -e -a "Artist Name" song.mp3

# Edit album
./mp3_tag_editor -e -A "Album Name" song.mp3

# Edit year
./mp3_tag_editor -e -y "2024" song.mp3

# Edit genre
./mp3_tag_editor -e -g "Rock" song.mp3

# Edit comment
./mp3_tag_editor -e -c "My favorite song" song.mp3
```

#### 3. Delete Tags

Remove specific fields or all metadata:

```bash
# Delete a specific tag
./mp3_tag_editor -d song.mp3 -t    # Delete title
./mp3_tag_editor -d song.mp3 -c    # Delete comment

# Strip ALL metadata
./mp3_tag_editor -D song.mp3
./mp3_tag_editor --delete-all song.mp3
```

#### 4. Extract Album Art

Save embedded cover image to a file:

```bash
./mp3_tag_editor -x song.mp3 cover.jpg
./mp3_tag_editor --extract song.mp3 albumart.png
```

#### 5. Display Help

```bash
./mp3_tag_editor -h
./mp3_tag_editor --help
```

### Tag Identifiers

| Flag | ID3 Frame | Description |
|------|-----------|-------------|
| `-t` | TIT2 | Song title |
| `-a` | TPE1 | Artist/performer |
| `-A` | TALB | Album name |
| `-y` | TYER | Year |
| `-T` | TRCK | Track number |
| `-g` | TCON | Genre |
| `-c` | COMM | Comment |
| `-b` | TPE2 | Album artist |
| `-m` | TCOM | Composer |
| `-o` | TCOP | Copyright |
| `-p` | TPUB | Publisher |

---

## 🏷️ ID3 Tag Support

### Supported Versions

| Version | Read | Write | Notes |
|---------|------|-------|-------|
| ID3v2.3 | ✅ | ✅ | Full support |
| ID3v2.4 | ✅ | ✅ | Full support |
| ID3v2.2 | ❌ | ❌ | Different frame structure |
| ID3v1   | ❌ | ❌ | Legacy format |

### Supported Frames

#### Text Frames
- **TIT2** - Title/Song name
- **TPE1** - Lead artist/performer
- **TALB** - Album/Movie/Show title
- **TYER** - Year (v2.3) / TDRC (v2.4)
- **TRCK** - Track number/position
- **TCON** - Content type (Genre)
- **COMM** - Comments
- **TPE2** - Band/Orchestra/Accompaniment
- **TCOM** - Composer
- **TCOP** - Copyright message
- **TPUB** - Publisher

#### Binary Frames
- **APIC** - Attached picture (album art)

### Text Encodings

| Encoding | Byte | Support | Notes |
|----------|------|---------|-------|
| ISO-8859-1 | 0x00 | ✅ Full | Standard Latin-1 |
| UTF-16 LE | 0x01 | ✅ ASCII subset | Simplified decoder |
| UTF-16 BE | 0x02 | ⚠️ Limited | Basic support |
| UTF-8 | 0x03 | ❌ | Not implemented |

---

## 🏗️ Architecture

### Data Flow Diagram

```
┌─────────────┐
│   User CLI  │
└──────┬──────┘
       │
       ├─── View ────► read_id3_tags() ────► display_metadata()
       │
       ├─── Edit ────► read_id3_tags() ────► edit_tag() ────► write_id3_tags()
       │
       ├─── Delete ──► read_id3_tags() ────► delete_tag() ───► write_id3_tags()
       │
       └─── Extract ─► read_id3_tags() ────► extract_album_art()
```

### Key Algorithms

#### 1. Tag Reading Algorithm

```
1. Open MP3 file in binary read mode
2. Read 10-byte ID3 header
3. Validate "ID3" signature
4. Extract version (v2.3 or v2.4)
5. Parse tag size (synchsafe integer)
6. Iterate through frames:
   a. Read 10-byte frame header
   b. Extract frame ID and size
   c. Allocate memory for frame data
   d. Read frame content
   e. Handle text encoding (UTF-16 → ASCII if needed)
   f. Store in TagData structure
7. Return populated TagData
```

#### 2. Tag Writing Algorithm

```
1. Calculate new tag size (including padding)
2. Open source file (read) and temp file (write)
3. Write ID3v2.4 header to temp file
4. Write all non-NULL frames to temp file
5. Write padding (1024 bytes)
6. Read old tag size from source
7. Copy audio data from source to temp
8. Close both files
9. Replace original with temp file
```

### Memory Management

```c
// Safe allocation pattern
TagData *data = create_tag_data();  // calloc() - zero-initialized

// ... use data ...

free_tag_data(data);  // Frees all nested allocations
data = NULL;          // Prevent dangling pointer
```

---

## 💡 Examples

### Example 1: View All Tags

```bash
$ ./mp3_tag_editor -v example.mp3
```

**Output:**
```
Mp3 Tag Reader & Editor:
---------------------
Version ID : 2.3
Title      : Shape of You
Artist     : Ed Sheeran
Album      : Divide
Year       : 2017
Track      : 4
Genre      : Pop
Comment    : Downloaded from iTunes
Composer   : Ed Sheeran, Steve Mac
Image Type : image/jpeg
Image Size : 52341 bytes
```

### Example 2: Edit Multiple Fields

```bash
# Change title
./mp3_tag_editor -e -t "New Title" song.mp3

# Change artist
./mp3_tag_editor -e -a "New Artist" song.mp3

# Change year
./mp3_tag_editor -e -y "2024" song.mp3
```

### Example 3: Extract and View Album Art

```bash
# Extract cover art
./mp3_tag_editor -x song.mp3 cover.jpg

# View the image
xdg-open cover.jpg  # Linux
open cover.jpg      # macOS
```

### Example 4: Clean Metadata

```bash
# Remove comment field
./mp3_tag_editor -d song.mp3 -c

# Strip all metadata (privacy)
./mp3_tag_editor -D song.mp3
```

### Example 5: Batch Processing

```bash
# Process all MP3 files in directory
for file in *.mp3; do
    ./mp3_tag_editor -e -y "2024" "$file"
done
```

---

## ⚠️ Limitations

### Current Limitations

1. **Text Encoding**
   - UTF-16 decoding is simplified (ASCII subset only)
   - Full Unicode characters may not display correctly
   - UTF-8 encoding (ID3v2.4) not yet implemented

2. **Frame Support**
   - Only common text frames implemented
   - Advanced frames (PRIV, GEOB, SYLT) are ignored
   - Picture description in APIC frames is basic

3. **ID3 Versions**
   - ID3v2.2 not supported (different structure)
   - ID3v1 tags are not read or written
   - Mixed version tags may cause issues

4. **File Safety**
   - Corrupted MP3 files may fail validation
   - Very large files (>2GB) may have issues
   - Requires free disk space for temporary file

5. **Platform**
   - Primarily tested on Linux
   - Windows support requires WSL or MinGW
   - File paths are Unix-style

### Known Issues

- **Issue #1**: Very long metadata values (>1000 chars) may cause display issues
- **Issue #2**: Some APIC frames with complex descriptions may fail extraction
- **Issue #3**: Files with unsynchronization flag set may not parse correctly

---

## 🚀 Future Enhancements

### Planned Features

- [ ] **Full UTF-8/UTF-16 Support**: Complete Unicode implementation
- [ ] **Album Art Embedding**: Add images to MP3 files
- [ ] **ID3v1 Support**: Read and write legacy tags
- [ ] **Batch Operations**: Process multiple files at once
- [ ] **Interactive Mode**: TUI for easier tag editing
- [ ] **JSON Export**: Export metadata to JSON format
- [ ] **Auto-tagging**: Fetch metadata from online databases
- [ ] **Corrupt File Recovery**: Better handling of damaged files

### Performance Improvements

- [ ] Memory-mapped file I/O for large files
- [ ] Incremental tag updates (avoid full rewrite)
- [ ] Multi-threaded batch processing

---

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

### Ways to Contribute

1. **Report Bugs**: Open an issue with details
2. **Suggest Features**: Propose enhancements
3. **Submit PRs**: Fix bugs or add features
4. **Improve Docs**: Enhance documentation
5. **Test**: Try with various MP3 files

### Development Setup

```bash
# Fork and clone the repo
git clone https://github.com/KruBro/MP3-Tag-Reader-Editor.git
cd mp3-id3-tag-editor

# Create a feature branch
git checkout -b feature/your-feature-name

# Make your changes and test
make clean
make
./mp3_tag_editor -v test.mp3

# Commit with clear messages
git commit -m "Add: Feature description"

# Push and create PR
git push origin feature/your-feature-name
```

### Code Style

- Follow K&R C style
- Use meaningful variable names
- Comment complex algorithms
- Keep functions under 100 lines
- Test thoroughly before submitting

---

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2024 [Your Name]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

---

## 📚 References

### ID3 Specifications

- [ID3v2.3 Specification](https://id3.org/id3v2.3.0) - Official ID3v2.3 standard
- [ID3v2.4 Specification](https://id3.org/id3v2.4.0-structure) - Official ID3v2.4 standard
- [ID3.org](https://id3.org/) - Main ID3 website and resources

### MPEG & Audio Standards

- [MPEG Audio Layer III](https://en.wikipedia.org/wiki/MP3) - MP3 format overview
- [ISO/IEC 11172-3](https://www.iso.org/standard/22412.html) - MPEG-1 Audio standard
- [Synchsafe Integers](https://en.wikipedia.org/wiki/Synchsafe) - Encoding explanation

### Learning Resources

- [MP3 File Structure](https://www.codeproject.com/Articles/8295/MPEG-Audio-Frame-Header) - Detailed MP3 format breakdown
- [ID3 Tag Primer](https://mutagen-specs.readthedocs.io/en/latest/id3/id3v2.4.0-structure.html) - Comprehensive guide
- [Binary File I/O in C](https://www.cprogramming.com/tutorial/cfileio.html) - C file handling tutorial

---

## 👨‍💻 Author

**Shahad K**

- GitHub: [@KruBro](https://github.com/KruBro)
- Email: shahadkurungadan7@gmail.com
- LinkedIn: [Shahad K](https://www.linkedin.com/in/shahad-kurungadan-057596227/)

---

## 🙏 Acknowledgments

- **ID3.org** - For maintaining the ID3 tag specifications
- **The C Programming Language** (K&R) - Foundation of systems programming
- **Open Source Community** - For inspiration and learning resources

---

## 📊 Project Stats

![GitHub repo size](https://img.shields.io/github/repo-size/KruBro/MP3-Tag-Reader-Editor)
![GitHub last commit](https://img.shields.io/github/last-commit/KruBro/MP3-Tag-Reader-Editor)
![GitHub issues](https://img.shields.io/github/issues/KruBro/MP3-Tag-Reader-Editor)
![GitHub stars](https://img.shields.io/github/stars/KruBro/MP3-Tag-Reader-Editor)

---

<div align="center">

**⭐ Star this repo if you find it useful! ⭐**

Made with ❤️ and C

[Report Bug](https://github.com/KruBro/MP3-Tag-Reader-Editor/issues) · [Request Feature](https://github.com/KruBro/MP3-Tag-Reader-Editor/issues) · [Documentation](https://github.com/KruBro/MP3-Tag-Reader-Editor/wiki)

</div>
