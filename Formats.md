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

* Magic Number - 4 bytes. " SCB"
* Unknown - 4 bytes.
* Image Width - 4 bytes.
* Image Height - 4 bytes.
* Unknown - 4 bytes.
* Block Count - 4 bytes.
* Unknown 4 bytes.
* Image blocks.

Image block format:
* Block Size - 4 bytes
* Block Data
