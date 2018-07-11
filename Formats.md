# Formats
Sprint formats are Little-Endian

Common formats:
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

Stream file. A stream of bytes as the game would read them.
"Our solution was what we called a stream file. A stream file was simply a stream of bytes as loaded by the engine. We knew that when you loaded one level in our engine that it would load the same data in the same order every time. If for some reason, it stopped reading from one file and started reading from another, it would do this every time at the same point as long as the data remained consistent between loads. So we created a piece of code in our file system that could be flipped on to dump every byte loaded back into a new stream file. On the next load, you could draw the data from the single stream file instead of the dozens or hundreds of original files needed to define the level. We could then do the same thing with our characters and global object data by creating separate stream files for each of them. Now when we go to load a level, we simply read a contiguous block of 30 MB of data as fast as the DVD can stream it. Wrap this code in a multi-threaded IO reader and this dropped our load times from 30 to 40 seconds down to 5 to 10 seconds, which in turn satisfied the Microsoft certification maximum load time." - https://www.gamasutra.com/view/feature/131337/postmortem_prestos_whacked.php?page=2

## BIK

Bink image format.

## SAF

Sprint Audio File. WAVE files, IMA ADPCM encoding.

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
