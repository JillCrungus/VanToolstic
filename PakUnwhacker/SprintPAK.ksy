meta:
  id: sprint_pak
  title: Sprint Engine PAK File
  application: Sprint Engine Games
  file-extension: PAK
  endian: le
seq:
  - id: header
    type: header
  - id: file_list
    type: file
    repeat: expr
    repeat-expr: header.file_count

types:

  header:
    seq:
      - id: magic
        contents: [0x53, 0x50, 0x50, 0x4b]
      - id: initial_file_offset
        type: u4
        doc: The actual files start at this address
      - id: file_count
        type: u4
        doc: How many files are in this PAK
      
        
  
  file:
    seq:
      - id: filename
        terminator: 0x00
        type: str
        encoding: UTF-8
        doc: The name of this file
      - id: filesize
        type: u4
        doc: This file's size in bytes
      - id: file_relative_offset
        type: u4
        doc: This file's offset relative to the initial offset
        
    instances:
        file_data:
          pos: _parent.header.initial_file_offset+file_relative_offset
          size: filesize
          doc: The data for this file
        
