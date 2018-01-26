# This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

import array
import struct
import zlib
from enum import Enum
from pkg_resources import parse_version

from kaitaistruct import __version__ as ks_version, KaitaiStruct, KaitaiStream, BytesIO


if parse_version(ks_version) < parse_version('0.7'):
    raise Exception("Incompatible Kaitai Struct Python API: 0.7 or later is required, but you have %s" % (ks_version))

class SprintPak(KaitaiStruct):
    def __init__(self, _io, _parent=None, _root=None):
        self._io = _io
        self._parent = _parent
        self._root = _root if _root else self
        self.header = self._root.Header(self._io, self, self._root)
        self.file_list = [None] * (self.header.file_count)
        for i in range(self.header.file_count):
            self.file_list[i] = self._root.File(self._io, self, self._root)


    class Header(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self.magic = self._io.ensure_fixed_contents(struct.pack('4b', 83, 80, 80, 75))
            self.initial_file_offset = self._io.read_u4le()
            self.file_count = self._io.read_u4le()


    class File(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self.filename = (self._io.read_bytes_term(0, False, True, True)).decode(u"UTF-8")
            self.filesize = self._io.read_u4le()
            self.file_relative_offset = self._io.read_u4le()

        @property
        def file_data(self):
            if hasattr(self, '_m_file_data'):
                return self._m_file_data if hasattr(self, '_m_file_data') else None

            _pos = self._io.pos()
            self._io.seek((self._parent.header.initial_file_offset + self.file_relative_offset))
            self._m_file_data = self._io.read_bytes(self.filesize)
            self._io.seek(_pos)
            return self._m_file_data if hasattr(self, '_m_file_data') else None



