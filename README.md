# MP3 ID3 Tag Reader & Editor (Binary-Level Implementation in C)

## 📌 Project Overview
This project is a **low-level MP3 metadata parser and editor** implemented entirely in **C**, designed to operate directly on the **binary structure of MP3 files**.  
It reads, interprets, and modifies **ID3 tags** without using any external libraries, focusing on **file I/O, memory management, endianness, and binary-safe parsing**.

The project is intended as a **systems programming exercise**, not a media application.

---

## 🎯 Objectives
- Understand the internal structure of MP3 files
- Parse ID3 metadata at the byte level
- Support ID3v2.3 and ID3v2.4 tag formats
- Handle synchsafe integers correctly
- Implement safe tag editing without corrupting audio data
- Strengthen binary file handling skills in C

---

## 🧠 Key Concepts Covered
- Binary file I/O (`rb`, `wb`)
- ID3 header and frame parsing
- Synchsafe integer decoding
- Big-endian vs little-endian handling
- Dynamic memory allocation
- Safe file rewriting using temporary files
- UTF-16 and ISO-8859-1 text handling

---

## 🗂️ Project Structure
.
├── error_handling.c
├── error_handling.h
├── id3_reader.c
├── id3_reader.h
├── id3_writer.c
├── id3_writer.h
├── id3_utils.c
├── id3_utils.h
├── main.c
├── README.md


---

## 🏗️ Module Description

### 1. `error_handling`
Handles consistent error reporting and file validation.

### 2. `id3_reader`
- Reads ID3 headers and frames
- Supports ID3v2.3 and ID3v2.4
- Extracts metadata such as:
  - Title
  - Artist
  - Album
  - Year
  - Genre
  - Comment

### 3. `id3_writer`
- Writes ID3v2.4 tags
- Safely edits metadata fields
- Preserves original audio data
- Uses temporary file strategy for atomic updates

### 4. `id3_utils`
- Synchsafe integer conversion
- Big-endian decoding
- Memory management utilities

---

## ⚙️ Compilation
Use GCC on a Linux-based system:

```bash
gcc main.c id3_reader.c id3_writer.c id3_utils.c error_handling.c -o mp3_tag_tool

▶️ Usage
View MP3 Metadata

./mp3_tag_tool -v song.mp3

Edit Metadata

./mp3_tag_tool -t "New Title" song.mp3
./mp3_tag_tool -a "New Artist" song.mp3
./mp3_tag_tool -A "New Album" song.mp3
./mp3_tag_tool -y "2025" song.mp3
./mp3_tag_tool -g "Rock" song.mp3
./mp3_tag_tool -c "Sample Comment" song.mp3

📤 Expected Output
Viewing Tags

Mp3 Tag Reader & Editor:
-----------------------
Version ID : 2.3
Title      : Shape of You
Artist     : Ed Sheeran
Album      : Divide
Year       : 2017
Genre      : Pop
Comment    : Original File

Editing a Tag

Tag Edited Successfully.

🧪 Validation Strategy

Verified output using hex editors

Tested with multiple MP3 files from different sources

Ensured audio playback integrity after editing

Cross-checked metadata using VLC Media Player

⚠️ Limitations

Full UTF-16 decoding is simplified

Album art (APIC frame) parsing is limited

ID3v1 tags are not modified

Corrupted MP3 files may fail validation

🚀 Future Enhancements

Full Unicode (UTF-16 / UTF-8) support

Album art extraction and embedding

ID3v1 read/write support

Robust corrupted-file recovery

CLI auto-detection of tag versions

📚 References

ID3v2.3 Specification

ID3v2.4 Specification

MPEG Audio Layer III Documentation

ISO/IEC 11172-3 Standard

👨‍💻 Author

Developed as a systems-level learning project focusing on binary engineering and low-level file parsing in C.