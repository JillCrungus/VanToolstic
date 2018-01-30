# Formats
Sprint formats are Little-Endian

# Sprint Formats

Formats used in the Sprint Engine

Common examples:
* STR
* PAK
* BIK
* SAF

## PAK

PAK is the first archive format used in the Sprint Engine. In Whacked! it contains some audio files, some movie files and some SCB files.

* Magic Number - 4 bytes. Spells out "SPPK" for "Sprint PAK"
* Initial file offset - 4 bytes, marks where in the PAK the actual files start.
* File Count - 4 bytes, how many files are in the PAK.
* File info listing - A list of filenames and info pertaining to them. Structure: Filename, filesize, relative offset to initial offset.
* Data - The actual data of the files.
    
## STR

An absolute mess. More research is needed.

## BIK

Bink image format.

## SAF

Sprint Audio File. WAVE files.

## SCB
Credit: [JuanJP600](https://github.com/juanjp600/)

Sprint Colour Bitmap.

Header:
* Magic number - 4 bytes, should be the string ".SCB"
* Version number - 4 bytes, little endian. Seems to always be 1.
* Image width, in pixels - 4 bytes, little endian.
* Image height - 4 bytes, little endian.
* Compression method - 4 bytes, little endian. 0 indicates DXT1, 1 and 2 indicate DXT3.
* Block count - 4 bytes, little endian.
* Total image size, in bytes - 4 bytes, little endian.

Image block format:
* Block size, in bytes - 4 bytes, little endian.
* Block data - Data compressed by the method described in the header.
