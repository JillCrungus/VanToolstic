from sprint_pak import *
import os
import sys


def IsInt(s):
    try: 
        int(s)
        return True
    except ValueError:
        return False

try:
    inputFileName = sys.argv[1]
except IndexError:
    print("Jill's PAK UnWhacker")
    print("Part of the VanToolstic toolset")
    print("")
    print("Usage:")
    print("pakunwhacker.py <PAK File> list - Lists files inside a Sprint PAK")
    print("pakunwhacker.py <PAK File> info - Display information about a Sprint PAK")
    print("pakunwhacker.py <PAK File> extract <folder> - Extracts all files from a Sprint PAK into a given folder")
    print("pakunwhacker.py <PAK File> extract_file <file/file number> - Extracts a specific file from a Sprint PAK")
    exit()


inputFile = SprintPak.from_file(inputFileName)


if len(sys.argv) == 3:
    if sys.argv[2] == "list":
        print(inputFileName + " file listing")
        for i in range(inputFile.header.file_count):
            print( "File " + str(i) )
            print("File name: " + inputFile.file_list[i].filename)
            print("File size: " + str(inputFile.file_list[i].filesize) + " bytes")

    if sys.argv[2] == "info":
        print("Sprint PAK File")
        print("Files in PAK: " + str(inputFile.header.file_count))
        print("Initial file offset: " + str(inputFile.header.initial_file_offset))

if len(sys.argv) == 4:
    if sys.argv[2] == "extract":
        print("Extracting files from " + inputFileName)
        if os.path.exists(sys.argv[3]) != True:
            os.mkdir(sys.argv[3])
        targetDir = sys.argv[3]
        for i in range(inputFile.header.file_count):
            print( "File " + str(i) )
            print("File name: " + inputFile.file_list[i].filename)
            print("File size: " + str(inputFile.file_list[i].filesize) + " bytes")
            filetoWrite = targetDir + "\\" + inputFile.file_list[i].filename
            print("Writing to " + filetoWrite)
            f = open(filetoWrite, 'wb')
            f.write(inputFile.file_list[i].file_data)
            f.close()

    if sys.argv[2] == "extract_file":
        if IsInt(sys.argv[3]) == True:
            print("Extracting File Number " + sys.argv[3] + " from " + inputFileName)
        else:
            print("Extracting " + sys.argv[3] + " from " + inputFileName)
        fileFound = False
        for i in range(inputFile.header.file_count):
            if IsInt(sys.argv[3]) == True:
                if i == int(sys.argv[3]):
                    fileFound = True
                    print( "File " + str(i) )
                    print("File name: " + inputFile.file_list[i].filename)
                    print("File size: " + str(inputFile.file_list[i].filesize) + " bytes")
                    print("Writing to " + inputFile.file_list[i].filename)
                    f = open(inputFile.file_list[i].filename, 'wb')
                    f.write(inputFile.file_list[i].file_data)
                    f.close()
            else:
                if inputFile.file_list[i].filename.lower() == sys.argv[3].lower():
                    fileFound = True
                    print( "File " + str(i) )
                    print("File name: " + inputFile.file_list[i].filename)
                    print("File size: " + str(inputFile.file_list[i].filesize) + " bytes")
                    print("Writing to " + inputFile.file_list[i].filename)
                    f = open(inputFile.file_list[i].filename, 'wb')
                    f.write(inputFile.file_list[i].file_data)
                    f.close()

        if fileFound != True:
            if IsInt(sys.argv[3]) == True:
                print("Error!! File number " + sys.argv[3] + " not found in " + inputFileName)
            else:
                print("Error!! File " + sys.argv[3] + " not found in " + inputFileName)
        else:
            print("Extraction successful!")
