# 🚀 Quick Start Guide

Get up and running with the MP3 ID3 Tag Editor in 5 minutes!

---

## 📥 Installation

### Option 1: Quick Build (Linux/macOS)

```bash
# Clone the repository
git clone https://github.com/KruBro/MP3-Tag-Reader-Editor.git
cd MP3-Tag-Reader-Editor

# Build
make

# Test it works
./mp3_tag_editor -h
```

### Option 2: Manual Compilation

```bash
# Compile all source files
gcc -Wall -Wextra -std=c99 -o mp3_tag_editor \
    main.c id3_reader.c id3_writer.c id3_utils.c error_handling.c

# Run
./mp3_tag_editor -h
```

---

## 🎯 Common Tasks

### 1. View All Tags

```bash
./mp3_tag_editor -v your_song.mp3
```

**Example Output:**
```
Mp3 Tag Reader & Editor:
---------------------
Version ID : 2.3
Title      : Bohemian Rhapsody
Artist     : Queen
Album      : A Night at the Opera
Year       : 1975
Genre      : Rock
```

---

### 2. Edit Title

```bash
./mp3_tag_editor -e -t "My New Title" song.mp3
```

---

### 3. Edit Artist

```bash
./mp3_tag_editor -e -a "New Artist Name" song.mp3
```

---

### 4. Edit Multiple Tags

```bash
# Edit title
./mp3_tag_editor -e -t "New Title" song.mp3

# Edit artist
./mp3_tag_editor -e -a "New Artist" song.mp3

# Edit year
./mp3_tag_editor -e -y "2024" song.mp3

# Verify changes
./mp3_tag_editor -v song.mp3
```

---

### 5. Extract Album Art

```bash
./mp3_tag_editor -x song.mp3 cover.jpg

# View the image
xdg-open cover.jpg  # Linux
open cover.jpg      # macOS
```

---

### 6. Delete a Tag

```bash
# Delete the comment field
./mp3_tag_editor -d song.mp3 -c

# Verify it's gone
./mp3_tag_editor -v song.mp3
```

---

### 7. Strip All Metadata

```bash
# Remove ALL ID3 tags (use with caution!)
./mp3_tag_editor -D song.mp3

# Verify tags are gone
./mp3_tag_editor -v song.mp3
```

---

## 📝 All Available Tag Options

| Flag | Tag Field | Example |
|------|-----------|---------|
| `-t` | Title | `-t "Song Title"` |
| `-a` | Artist | `-a "Artist Name"` |
| `-A` | Album | `-A "Album Name"` |
| `-y` | Year | `-y "2024"` |
| `-T` | Track | `-T "5"` |
| `-g` | Genre | `-g "Rock"` |
| `-c` | Comment | `-c "My comment"` |
| `-b` | Album Artist | `-b "Band Name"` |
| `-m` | Composer | `-m "Composer Name"` |
| `-o` | Copyright | `-o "© 2024"` |
| `-p` | Publisher | `-p "Label Name"` |

---

## 🔧 Troubleshooting

### Problem: "File Could Not Open"

**Solution:** Check file exists and you have read permissions
```bash
ls -l your_song.mp3
chmod 644 your_song.mp3
```

### Problem: "Version not supported"

**Solution:** File has ID3v2.2 or ID3v1 tags (not supported)
```bash
# Check ID3 version with xxd
xxd -l 10 your_song.mp3
```

### Problem: Command not found

**Solution:** Make sure the executable is in your current directory
```bash
# Add ./ before the command
./mp3_tag_editor -v song.mp3

# Or add to PATH
export PATH=$PATH:$(pwd)
mp3_tag_editor -v song.mp3
```

### Problem: Compilation errors

**Solution:** Ensure you have GCC installed
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# macOS
xcode-select --install

# Check GCC version
gcc --version
```

---

## 📚 Next Steps

- Read the [full README](README.md) for detailed information
- Check [CONTRIBUTING.md](CONTRIBUTING.md) to contribute
- Browse [examples](#) for advanced usage
- Report bugs or request features via [Issues](https://github.com/KruBro/MP3-Tag-Reader-Editor/issues)

---

## 💡 Pro Tips

### Batch Processing

Process multiple MP3 files at once:

```bash
# Change year for all MP3s in current directory
for file in *.mp3; do
    ./mp3_tag_editor -e -y "2024" "$file"
done

# Extract all album art
for file in *.mp3; do
    basename="${file%.mp3}"
    ./mp3_tag_editor -x "$file" "${basename}.jpg"
done
```

### Backup Before Editing

Always create backups of important files:

```bash
# Backup a file
cp song.mp3 song.mp3.backup

# Edit the original
./mp3_tag_editor -e -t "New Title" song.mp3

# Restore if needed
cp song.mp3.backup song.mp3
```

### Check Before Stripping

View tags before removing them all:

```bash
# View current tags
./mp3_tag_editor -v song.mp3

# Only strip if you're sure!
./mp3_tag_editor -D song.mp3
```

---

## ⚡ Quick Reference Card

```
VIEWING:
  -v, --view <file>              View all tags

EDITING:
  -e, --edit <tag> <val> <file>  Edit a tag
  
  Tags: -t (title), -a (artist), -A (album),
        -y (year), -T (track), -g (genre),
        -c (comment), -b (album artist),
        -m (composer), -o (copyright),
        -p (publisher)

DELETING:
  -d, --delete <file> <tag>      Delete one tag
  -D, --delete-all <file>        Delete all tags

EXTRACTING:
  -x, --extract <file> <output>  Extract album art

HELP:
  -h, --help                     Show help
```

---

**Need more help?** Check the [full documentation](README.md) or [open an issue](https://github.com/KruBro/MP3-Tag-Reader-Editor/issues)!

Happy tagging! 🎵
