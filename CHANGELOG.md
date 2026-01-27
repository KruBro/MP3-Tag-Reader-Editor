# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Planned
- Full UTF-8 support for ID3v2.4
- Album art embedding functionality
- ID3v1 tag read/write support
- Batch processing mode
- JSON export format

---

## [1.0.0] - 2024-01-27

### Added
- **Core Features**
  - ID3v2.3 and ID3v2.4 tag reading
  - ID3 tag editing (title, artist, album, year, genre, comment)
  - Extended tag support (track, album artist, composer, copyright, publisher)
  - Album art extraction (APIC frame)
  - Individual tag deletion
  - Complete tag stripping (delete all metadata)
  
- **Text Encoding**
  - ISO-8859-1 (Latin-1) support
  - UTF-16 Little Endian basic support (ASCII subset)
  
- **Utility Functions**
  - Synchsafe integer encoding/decoding
  - Big-endian integer conversion
  - Safe memory management
  - Error handling and validation
  
- **Command-Line Interface**
  - View tags (`-v`, `--view`)
  - Edit tags (`-e`, `--edit`)
  - Delete tag (`-d`, `--delete`)
  - Delete all tags (`-D`, `--delete-all`)
  - Extract album art (`-x`, `--extract`)
  - Help screen (`-h`, `--help`)
  
- **Documentation**
  - Comprehensive README.md
  - Inline code documentation
  - Usage examples
  - Contributing guidelines
  
- **Build System**
  - Makefile for easy compilation
  - Clean build process
  - No external dependencies

### Technical Details
- Written in pure C (C99 standard)
- Zero external dependencies
- Binary-safe file handling
- Atomic file updates using temporary files
- Proper memory management (no leaks)

### Known Limitations
- UTF-16 decoding simplified (ASCII only)
- UTF-8 not yet supported
- ID3v2.2 not supported
- ID3v1 not supported
- Large files (>2GB) may have issues

---

## Version History

### Version Numbering

This project follows semantic versioning (MAJOR.MINOR.PATCH):

- **MAJOR**: Incompatible API changes
- **MINOR**: New features (backwards compatible)
- **PATCH**: Bug fixes (backwards compatible)

### Release Notes Format

Each release includes:
- **Added**: New features
- **Changed**: Changes to existing functionality
- **Deprecated**: Soon-to-be removed features
- **Removed**: Removed features
- **Fixed**: Bug fixes
- **Security**: Security fixes

---

## Future Roadmap

### Version 1.1.0 (Planned)
- [ ] Full UTF-8 text encoding support
- [ ] Improved UTF-16 decoder (full Unicode)
- [ ] Better error messages
- [ ] Performance optimizations

### Version 1.2.0 (Planned)
- [ ] Album art embedding
- [ ] Multiple image support
- [ ] Image format conversion

### Version 2.0.0 (Planned)
- [ ] ID3v1 support
- [ ] Batch processing
- [ ] Interactive TUI mode
- [ ] Configuration file support

---

## Migration Guides

### Upgrading to 1.0.0

Initial release - no migration needed.

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for information on how to contribute to this project.

---

## Links

- [Repository](https://github.com/KruBro/MP3-Tag-Reader-Editor)
- [Issues](https://github.com/KruBro/MP3-Tag-Reader-Editor/issues)
- [Releases](https://github.com/KruBro/MP3-Tag-Reader-Editor/releases)

---

**[1.0.0]**: https://github.com/KruBro/MP3-Tag-Reader-Editor/releases/tag/v1.0.0
**[Unreleased]**: https://github.com/KruBro/MP3-Tag-Reader-Editor/compare/v1.0.0...HEAD
