//
//  plist.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-08-23.
//  Copyright (c) 2014 Daniel Cliche. All rights reserved.
//

//
//   PlistCpp Property List (plist) serialization and parsing library.
//
//   https://github.com/animetrics/PlistCpp
//
//   Copyright (c) 2011 Animetrics Inc. (marc@animetrics.com)
//
//   Permission is hereby granted, free of charge, to any person obtaining a copy
//   of this software and associated documentation files (the "Software"), to deal
//   in the Software without restriction, including without limitation the rights
//   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//   copies of the Software, and to permit persons to whom the Software is
//   furnished to do so, subject to the following conditions:
//
//   The above copyright notice and this permission notice shall be included in
//   all copies or substantial portions of the Software.
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//   THE SOFTWARE.

#include "plist.h"

#include <string.h>

#include <algorithm>
#include <list>
#include <sstream>
#include <cstdint>

#include "base64.h"

namespace Plist {

struct PlistHelperData {
   public:
    // binary helper data
    std::vector<int32_t> _offsetTable;
    std::list<std::vector<unsigned char>> _objectTables;
    size_t _totalSize;
    std::vector<unsigned char> _objectTable;
    int32_t _offsetByteSize;
    int64_t _offsetTableOffset;

    int32_t _objRefSize;
    int32_t _refCount;
};

void writePlistBinary(PlistHelperData& d, const Any& message);

// helper functions

// msvc <= 2005 doesn't have std::vector::data() method

template <typename T>
T* vecData(std::vector<T>& vec) {
    return (vec.size() > 0) ? &vec[0] : 0;
    // if(vec.size() > 0)
    //		return &vec[0];
    // else
    //		throw Error("vecData trying to get pointer to empty std::vector");
}

template <typename T>
const T* vecData(const std::vector<T>& vec) {
    return (vec.size() > 0) ? &vec[0] : 0;
    // if(vec.size() > 0)
    //		return &vec[0];
    // else
    //		throw Error("vecData trying to get pointer to empty std::vector");
}

// binary helper functions

template <typename IntegerType>
IntegerType bytesToInt(const unsigned char* bytes, bool littleEndian);
double bytesToDouble(const unsigned char* bytes, bool littleEndian);
std::vector<unsigned char> doubleToBytes(double val, bool littleEndian);
template <typename IntegerType>
std::vector<unsigned char> intToBytes(IntegerType val, bool littleEndian);
std::vector<unsigned char> getRange(const unsigned char* origBytes, int64_t index, int64_t size);
std::vector<unsigned char> getRange(const std::vector<unsigned char>& origBytes, int64_t index, int64_t size);
std::vector<char> getRange(const char* origBytes, int64_t index, int64_t size);

// binary parsing

Any parseBinary(const PlistHelperData& d, int objRef);
dictionary_type parseBinaryDictionary(const PlistHelperData& d, int objRef);
array_type parseBinaryArray(const PlistHelperData& d, int objRef);
std::vector<int32_t> getRefsForContainers(const PlistHelperData& d, int objRef);
int64_t parseBinaryInt(const PlistHelperData& d, int headerPosition, int& intByteCount);
double parseBinaryReal(const PlistHelperData& d, int headerPosition);
Date parseBinaryDate(const PlistHelperData& d, int headerPosition);
bool parseBinaryBool(const PlistHelperData& d, int headerPosition);
std::string parseBinaryString(const PlistHelperData& d, int objRef);
std::string parseBinaryUnicode(const PlistHelperData& d, int headerPosition);
data_type parseBinaryByteArray(const PlistHelperData& d, int headerPosition);
std::vector<unsigned char> regulateNullBytes(const std::vector<unsigned char>& origBytes, unsigned int minBytes);
void parseTrailer(PlistHelperData& d, const std::vector<unsigned char>& trailer);
void parseOffsetTable(PlistHelperData& d, const std::vector<unsigned char>& offsetTableBytes);
int32_t getCount(const PlistHelperData& d, int bytePosition, unsigned char headerByte, int& startOffset);

// binary writing

int countAny(const Any& object);
int countDictionary(const dictionary_type& dictionary);
int countArray(const array_type& array);
std::vector<unsigned char> writeBinaryDictionary(PlistHelperData& d, const dictionary_type& dictionary);
std::vector<unsigned char> writeBinaryArray(PlistHelperData& d, const array_type& array);
std::vector<unsigned char> writeBinaryByteArray(PlistHelperData& d, const data_type& byteArray);
std::vector<unsigned char> writeBinaryInteger(PlistHelperData& d, int64_t value, bool write);
std::vector<unsigned char> writeBinaryBool(PlistHelperData& d, bool value);
std::vector<unsigned char> writeBinaryDate(PlistHelperData& d, const Date& date);
std::vector<unsigned char> writeBinaryDouble(PlistHelperData& d, double value);
std::vector<unsigned char> writeBinary(PlistHelperData& d, const Any& obj);
std::vector<unsigned char> writeBinaryString(PlistHelperData& d, const std::string& value, bool head);

inline bool hostLittleEndian() {
    union {
        uint32_t x;
        uint8_t c[4];
    } u;
    u.x = 0xab0000cd;
    return u.c[0] == 0xcd;
}

}  // namespace Plist

namespace Plist {

// ---------------------------------------------------------------------------------------------------------------------
void writePlistBinary(PlistHelperData& d, const Any& message) {
    using namespace std;
    // int totalRefs =  countDictionary(message);
    int totalRefs = countAny(message) - 1;
    d._refCount = totalRefs;

    d._objRefSize =
        static_cast<int32_t>(regulateNullBytes(intToBytes<int32_t>(d._refCount, hostLittleEndian()), 1).size());

    d._totalSize = 0L;

    // writeBinaryDictionary(d, message);
    writeBinary(d, message);
    writeBinaryString(d, "bplist00", false);

    d._offsetTableOffset = (int64_t)d._totalSize;
    d._offsetTable.push_back(static_cast<int32_t>(d._totalSize - 8));

    vector<unsigned char> offsetBytes;

    std::reverse(d._offsetTable.begin(), d._offsetTable.end());

    for (unsigned int i = 0; i < d._offsetTable.size(); ++i)
        d._offsetTable[i] = static_cast<int32_t>(d._totalSize) - d._offsetTable[i];

    d._offsetByteSize =
        static_cast<int32_t>(regulateNullBytes(intToBytes<int>(d._offsetTable.back(), hostLittleEndian()), 1).size());

    for (unsigned int i = 0; i < d._offsetTable.size(); ++i) {
        vector<unsigned char> buffer =
            regulateNullBytes(intToBytes<int>(d._offsetTable[i], hostLittleEndian()), d._offsetByteSize);
        // reverse(buffer.begin(), buffer.end());

        offsetBytes.insert(offsetBytes.end(), buffer.rbegin(), buffer.rend());
    }

    std::vector<unsigned char> value;

    value.insert(value.end(), offsetBytes.begin(), offsetBytes.end());

    vector<unsigned char> dummy(6, 0);
    value.insert(value.end(), dummy.begin(), dummy.end());
    value.push_back((unsigned char)(d._offsetByteSize));
    value.push_back((unsigned char)(d._objRefSize));
    vector<unsigned char> temp = intToBytes<int64_t>((int64_t)totalRefs + 1, hostLittleEndian());
    value.insert(value.end(), temp.rbegin(), temp.rend());
    temp = intToBytes<int64_t>(0, hostLittleEndian());
    value.insert(value.end(), temp.begin(), temp.end());
    temp = intToBytes<int64_t>(d._offsetTableOffset, hostLittleEndian());
    value.insert(value.end(), temp.rbegin(), temp.rend());

    d._objectTables.push_back(value);
    d._totalSize += value.size();
}

// ---------------------------------------------------------------------------------------------------------------------
void writePlistBinary(std::vector<char>& plist, const Any& message) {
    PlistHelperData d;
    writePlistBinary(d, message);

    plist.clear();
    for (auto& table : d._objectTables) plist.insert(plist.end(), table.begin(), table.end());
}

// ---------------------------------------------------------------------------------------------------------------------
void writePlistBinary(std::ostream& stream, const Any& message) {
    PlistHelperData d;
    writePlistBinary(d, message);

    for (auto& table : d._objectTables) stream.write((const char*)vecData(table), table.size());
}

// ---------------------------------------------------------------------------------------------------------------------
void writePlistBinary(const char* filename, const Any& message) {
    std::ofstream stream(filename, std::ios::binary);
    writePlistBinary(stream, message);
    stream.close();
}

// ---------------------------------------------------------------------------------------------------------------------
int countAny(const Any& object) {
    using namespace std;
    static Any dict = dictionary_type();
    static Any array = array_type();

    int count = 0;
    if (object.is<dictionary_type>()) {
        count += countDictionary(object.as<dictionary_type>()) + 1;
    } else if (object.is<array_type>())
        count += countArray(object.as<array_type>()) + 1;
    else
        ++count;

    return count;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> writeBinary(PlistHelperData& d, const Any& obj) {
    using namespace std;

    std::vector<unsigned char> value;
    if (obj.is<int32_t>())
        value = writeBinaryInteger(d, obj.as<const int32_t&>(), true);
    else if (obj.is<int64_t>())
        value = writeBinaryInteger(d, obj.as<const int64_t&>(), true);
    else if (obj.is<long>())
        value = writeBinaryInteger(d, obj.as<const long&>(), true);
    else if (obj.is<short>())
        value = writeBinaryInteger(d, obj.as<const short&>(), true);
    else if (obj.is<dictionary_type>())
        value = writeBinaryDictionary(d, obj.as<const dictionary_type&>());
    else if (obj.is<string>())
        value = writeBinaryString(d, obj.as<const string&>(), true);
    else if (obj.is<array_type>())
        value = writeBinaryArray(d, obj.as<const array_type&>());
    else if (obj.is<data_type>())
        value = writeBinaryByteArray(d, obj.as<const data_type&>());
    else if (obj.is<double>())
        value = writeBinaryDouble(d, obj.as<const double&>());
    else if (obj.is<float>())
        value = writeBinaryDouble(d, obj.as<const float&>());
    else if (obj.is<Date>())
        value = writeBinaryDate(d, obj.as<const Date&>());
    else if (obj.is<bool>())
        value = writeBinaryBool(d, obj.as<const bool&>());
    else
        throw Error(string("Plist Error: Can't serialize type"));

    return value;
}

// ---------------------------------------------------------------------------------------------------------------------
static uint32_t nextpow2(uint32_t x) {
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static uint32_t ilog2(uint32_t x) {
    uint32_t r = 0;
    while (x >>= 1) ++r;
    return r;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> writeBinaryByteArray(PlistHelperData& d, const data_type& byteArray) {
    using namespace std;
    vector<unsigned char> header;
    if (byteArray.size() < 15)
        header.push_back(0x40 | ((unsigned char)byteArray.size()));
    else {
        header.push_back(0x40 | 0xf);
        vector<unsigned char> theSize = writeBinaryInteger(d, byteArray.size(), false);
        header.insert(header.end(), theSize.begin(), theSize.end());
    }

    vector<unsigned char> buffer(header);
    buffer.insert(buffer.end(), (unsigned char*)vecData(byteArray),
                  (unsigned char*)vecData(byteArray) + byteArray.size());
    d._objectTables.push_front(buffer);
    d._totalSize += buffer.size();

    return buffer;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> writeBinaryArray(PlistHelperData& d, const array_type& array) {
    using namespace std;

    vector<int32_t> refs;
    for (array_type::const_reverse_iterator it = array.rbegin(); it != array.rend(); ++it) {
        writeBinary(d, *it);
        d._offsetTable.push_back(static_cast<int32_t>(d._totalSize));
        refs.push_back(d._refCount);
        d._refCount--;
    }

    vector<unsigned char> header;
    if (array.size() < 15) {
        header.push_back(0xA0 | ((unsigned char)array.size()));
    } else {
        header.push_back(0xA0 | 0xf);
        vector<unsigned char> theSize = writeBinaryInteger(d, array.size(), false);
        header.insert(header.end(), theSize.begin(), theSize.end());
    }

    // try to do this more efficiently.  Not good to insert at the begining of buffer.

    vector<unsigned char> buffer;
    for (vector<int32_t>::const_iterator it = refs.begin(); it != refs.end(); ++it) {
        vector<unsigned char> refBuffer =
            regulateNullBytes(intToBytes<int32_t>(*it, hostLittleEndian()), d._objRefSize);
        //		reverse(refBuffer.begin(), refBuffer.end());
        buffer.insert(buffer.begin(), refBuffer.rbegin(), refBuffer.rend());
    }

    buffer.insert(buffer.begin(), header.begin(), header.end());

    d._objectTables.push_front(buffer);
    d._totalSize += buffer.size();

    return buffer;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> writeBinaryDictionary(PlistHelperData& d, const dictionary_type& dictionary) {
    using namespace std;

    vector<int32_t> refs;
    for (dictionary_type::const_reverse_iterator it = dictionary.rbegin(); it != dictionary.rend(); ++it) {
        writeBinary(d, it->second);
        d._offsetTable.push_back(static_cast<int32_t>(d._totalSize));
        refs.push_back(d._refCount);
        d._refCount--;
    }

    for (dictionary_type::const_reverse_iterator it = dictionary.rbegin(); it != dictionary.rend(); ++it) {
        writeBinary(d, it->first);
        d._offsetTable.push_back(static_cast<int32_t>(d._totalSize));
        refs.push_back(d._refCount);
        d._refCount--;
    }

    vector<unsigned char> header;
    if (dictionary.size() < 15) {
        header.push_back(0xD0 | ((unsigned char)dictionary.size()));
    } else {
        header.push_back(0xD0 | 0xf);
        vector<unsigned char> theSize = writeBinaryInteger(d, dictionary.size(), false);
        header.insert(header.end(), theSize.begin(), theSize.end());
    }

    // try to do this more efficiently.  Not good to insert at the begining of buffer.

    vector<unsigned char> buffer;
    for (vector<int32_t>::const_iterator it = refs.begin(); it != refs.end(); ++it) {
        vector<unsigned char> refBuffer =
            regulateNullBytes(intToBytes<int32_t>(*it, hostLittleEndian()), d._objRefSize);
        //		reverse(refBuffer.begin(), refBuffer.end());
        buffer.insert(buffer.begin(), refBuffer.rbegin(), refBuffer.rend());
    }

    buffer.insert(buffer.begin(), header.begin(), header.end());

    d._objectTables.push_front(buffer);
    d._totalSize += buffer.size();

    return buffer;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> writeBinaryDouble(PlistHelperData& d, double value) {
    using namespace std;
    vector<unsigned char> buffer = regulateNullBytes(doubleToBytes(value, hostLittleEndian()), 4);
    buffer.resize(nextpow2(static_cast<uint32_t>(buffer.size())), 0);

    unsigned char header = 0x20 | ilog2(static_cast<uint32_t>(buffer.size()));
    buffer.push_back(header);
    reverse(buffer.begin(), buffer.end());

    d._objectTables.push_front(buffer);
    d._totalSize += buffer.size();

    return buffer;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> writeBinaryBool(PlistHelperData& d, bool value) {
    std::vector<unsigned char> buffer;
    if (value)
        buffer.push_back(0x09);
    else
        buffer.push_back(0x08);

    d._objectTables.push_front(buffer);
    d._totalSize += buffer.size();

    return buffer;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> writeBinaryDate(PlistHelperData& d, const Date& date) {
    std::vector<unsigned char> buffer;

    // need to serialize as Apple epoch.

    double macTime = date.timeAsAppleEpoch();

    buffer = doubleToBytes(macTime, false);
    buffer.insert(buffer.begin(), 0x33);

    d._objectTables.push_front(buffer);
    d._totalSize += buffer.size();

    return buffer;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> writeBinaryInteger(PlistHelperData& d, int64_t value, bool write) {
    using namespace std;

    // The integer is initially forced to be 64 bit because it must be serialized
    // as 8 bytes if it is negative.   If it is not negative, the
    // regulateNullBytes step will reduce the representation down to the min
    // power base 2 bytes needed to store it.

    vector<unsigned char> buffer = intToBytes<int64_t>(value, hostLittleEndian());
    buffer = regulateNullBytes(intToBytes<int64_t>(value, hostLittleEndian()), 1);
    buffer.resize(nextpow2(static_cast<uint32_t>(buffer.size())), 0);

    unsigned char header = 0x10 | ilog2(static_cast<uint32_t>(buffer.size()));
    buffer.push_back(header);
    reverse(buffer.begin(), buffer.end());

    if (write) {
        d._objectTables.push_front(buffer);
        d._totalSize += buffer.size();
    }

    return buffer;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> writeBinaryString(PlistHelperData& d, const std::string& value, bool head) {
    using namespace std;
    vector<unsigned char> buffer;
    buffer.reserve(value.size());

    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) buffer.push_back((unsigned char)*it);

    if (head) {
        vector<unsigned char> header;
        if (value.length() < 15)
            header.push_back(0x50 | ((unsigned char)value.length()));
        else {
            header.push_back(0x50 | 0xf);
            vector<unsigned char> theSize = writeBinaryInteger(d, buffer.size(), false);
            header.insert(header.end(), theSize.begin(), theSize.end());
        }
        buffer.insert(buffer.begin(), header.begin(), header.end());
    }

    d._objectTables.push_front(buffer);
    d._totalSize += buffer.size();

    return buffer;
}

// ---------------------------------------------------------------------------------------------------------------------
int countDictionary(const dictionary_type& dictionary) {
    using namespace std;

    int count = 0;
    for (dictionary_type::const_iterator it = dictionary.begin(); it != dictionary.end(); ++it) {
        ++count;
        count += countAny(it->second);
    }

    return count;
}

// ---------------------------------------------------------------------------------------------------------------------
int countArray(const array_type& array) {
    using namespace std;
    int count = 0;
    for (array_type::const_iterator it = array.begin(); it != array.end(); ++it) count += countAny(*it);

    return count;
}

// ---------------------------------------------------------------------------------------------------------------------
void readPlist(std::istream& stream, Any& message) {
    int start = static_cast<int>(stream.tellg());
    stream.seekg(0, std::ifstream::end);
    int size = ((int)stream.tellg()) - start;
    if (size > 0) {
        stream.seekg(0, std::ifstream::beg);
        std::vector<char> buffer(size);
        stream.read((char*)&buffer[0], size);

        readPlist(&buffer[0], size, message);
    } else {
        throw Error("Can't read zero length data");
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void readPlist(const char* byteArrayTemp, int64_t size, Any& message) {
    using namespace std;
    const unsigned char* byteArray = (const unsigned char*)byteArrayTemp;
    if (!byteArray || (size == 0)) throw Error("Plist: Empty plist data");

    // infer plist type from header.  If it has the bplist00 header as first 8
    // bytes, then it's a binary plist.  Otherwise, assume it's XML

    std::string magicHeader((const char*)byteArray, 8);
    if (magicHeader == "bplist00") {
        PlistHelperData d;
        parseTrailer(d, getRange(byteArray, size - 32, 32));

        d._objectTable = getRange(byteArray, 0, d._offsetTableOffset);
        std::vector<unsigned char> offsetTableBytes =
            getRange(byteArray, d._offsetTableOffset, size - d._offsetTableOffset - 32);

        parseOffsetTable(d, offsetTableBytes);

        message = parseBinary(d, 0);
    } else {
        throw Error("Plist: Format not supported");
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void parseOffsetTable(PlistHelperData& d, const std::vector<unsigned char>& offsetTableBytes) {
    for (unsigned int i = 0; i < offsetTableBytes.size(); i += d._offsetByteSize) {
        std::vector<unsigned char> temp = getRange(offsetTableBytes, i, d._offsetByteSize);
        std::reverse(temp.begin(), temp.end());
        d._offsetTable.push_back(bytesToInt<int32_t>(vecData(regulateNullBytes(temp, 4)), hostLittleEndian()));
    }
}

void parseTrailer(PlistHelperData& d, const std::vector<unsigned char>& trailer) {
    d._offsetByteSize = bytesToInt<int32_t>(vecData(regulateNullBytes(getRange(trailer, 6, 1), 4)), hostLittleEndian());
    d._objRefSize = bytesToInt<int32_t>(vecData(regulateNullBytes(getRange(trailer, 7, 1), 4)), hostLittleEndian());

    std::vector<unsigned char> refCountBytes = getRange(trailer, 12, 4);
    //	std::reverse(refCountBytes.begin(), refCountBytes.end());
    d._refCount = bytesToInt<int32_t>(vecData(refCountBytes), false);

    std::vector<unsigned char> offsetTableOffsetBytes = getRange(trailer, 24, 8);
    //	std::reverse(offsetTableOffsetBytes.begin(), offsetTableOffsetBytes.end());
    d._offsetTableOffset = bytesToInt<int64_t>(vecData(offsetTableOffsetBytes), false);
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> regulateNullBytes(const std::vector<unsigned char>& origBytes, unsigned int minBytes) {
    std::vector<unsigned char> bytes(origBytes);
    while ((bytes.back() == 0) && (bytes.size() > minBytes)) bytes.pop_back();

    while (bytes.size() < minBytes) bytes.push_back(0);

    return bytes;
}

// ---------------------------------------------------------------------------------------------------------------------
Any parseBinary(const PlistHelperData& d, int objRef) {
    unsigned char header = d._objectTable[d._offsetTable[objRef]];
    switch (header & 0xF0) {
        case 0x00: {
            return parseBinaryBool(d, d._offsetTable[objRef]);
        }
        case 0x10: {
            int intByteCount;
            return parseBinaryInt(d, d._offsetTable[objRef], intByteCount);
        }
        case 0x20: {
            return parseBinaryReal(d, d._offsetTable[objRef]);
        }
        case 0x30: {
            return parseBinaryDate(d, d._offsetTable[objRef]);
        }
        case 0x40:
        case 0x80: {
            return parseBinaryByteArray(d, d._offsetTable[objRef]);
        }
        case 0x50: {
            return parseBinaryString(d, d._offsetTable[objRef]);
        }
        case 0x60: {
            return parseBinaryUnicode(d, d._offsetTable[objRef]);
        }
        case 0xD0: {
            return parseBinaryDictionary(d, objRef);
        }
        case 0xA0: {
            return parseBinaryArray(d, objRef);
        }
    }
    throw Error("This type is not supported");
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<int32_t> getRefsForContainers(const PlistHelperData& d, int objRef) {
    using namespace std;
    int32_t refCount = 0;
    int refStartPosition;
    refCount = getCount(d, d._offsetTable[objRef], d._objectTable[d._offsetTable[objRef]], refStartPosition);
    refStartPosition += d._offsetTable[objRef];

    vector<int32_t> refs;
    int mult = 1;
    if ((((unsigned char)d._objectTable[d._offsetTable[objRef]]) & 0xF0) == 0xD0) mult = 2;
    for (int i = refStartPosition; i < refStartPosition + refCount * mult * d._objRefSize; i += d._objRefSize) {
        std::vector<unsigned char> refBuffer = getRange(d._objectTable, i, d._objRefSize);
        reverse(refBuffer.begin(), refBuffer.end());
        refs.push_back(bytesToInt<int32_t>(vecData(regulateNullBytes(refBuffer, 4)), hostLittleEndian()));
    }

    return refs;
}

// ---------------------------------------------------------------------------------------------------------------------
array_type parseBinaryArray(const PlistHelperData& d, int objRef) {
    using namespace std;
    vector<int32_t> refs = getRefsForContainers(d, objRef);
    int32_t refCount = static_cast<int32_t>(refs.size());

    array_type array;
    for (int i = 0; i < refCount; ++i) array.push_back(parseBinary(d, refs[i]));

    return array;
}

// ---------------------------------------------------------------------------------------------------------------------
dictionary_type parseBinaryDictionary(const PlistHelperData& d, int objRef) {
    using namespace std;
    vector<int32_t> refs = getRefsForContainers(d, objRef);
    int32_t refCount = static_cast<int32_t>(refs.size() / 2);

    dictionary_type dict;
    for (int i = 0; i < refCount; i++) {
        Any keyAny = parseBinary(d, refs[i]);

        // try
        //{
        std::string& key = keyAny.as<std::string&>();
        dict[key] = parseBinary(d, refs[i + refCount]);
        //}
        // catch(boost::bad_any_cast& )
        //{
        //	throw Error("Error parsing dictionary.  Key can't be parsed as a string");
        //}
    }

    return dict;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string parseBinaryString(const PlistHelperData& d, int headerPosition) {
    unsigned char headerByte = d._objectTable[headerPosition];
    int charStartPosition;
    int32_t charCount = getCount(d, headerPosition, headerByte, charStartPosition);
    charStartPosition += headerPosition;

    std::vector<unsigned char> characterBytes = getRange(d._objectTable, charStartPosition, charCount);
    std::string buffer = std::string((char*)vecData(characterBytes), characterBytes.size());
    return buffer;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string parseBinaryUnicode(const PlistHelperData& d, int headerPosition) {
    /*
        unsigned char headerByte = d._objectTable[headerPosition];
        int charStartPosition;
        int32_t charCount = getCount(d, headerPosition, headerByte, charStartPosition);
        charStartPosition += headerPosition;

        std::vector<unsigned char> characterBytes = getRange(d._objectTable, charStartPosition, charCount * 2);
        if (hostLittleEndian()) {
                if (! characterBytes.empty()) {
                        for (std::size_t i = 0, n = characterBytes.size(); i < n - 1; i += 2)
                                std::swap(characterBytes[i], characterBytes[i + 1]);
                }
        }

        int16_t *u16chars = (int16_t*) vecData(characterBytes);
        std::size_t u16len = characterBytes.size() / 2;
        std::string result = boost::locale::conv::utf_to_utf<char, int16_t>(u16chars, u16chars + u16len,
       boost::locale::conv::stop); return result;
    */
    return std::string();
}

// ---------------------------------------------------------------------------------------------------------------------
int64_t parseBinaryInt(const PlistHelperData& d, int headerPosition, int& intByteCount) {
    unsigned char header = d._objectTable[headerPosition];
    intByteCount = 1 << (header & 0xf);
    std::vector<unsigned char> buffer = getRange(d._objectTable, headerPosition + 1, intByteCount);
    reverse(buffer.begin(), buffer.end());

    return bytesToInt<int64_t>(vecData(regulateNullBytes(buffer, 8)), hostLittleEndian());
}

// ---------------------------------------------------------------------------------------------------------------------
double parseBinaryReal(const PlistHelperData& d, int headerPosition) {
    unsigned char header = d._objectTable[headerPosition];
    int byteCount = 1 << (header & 0xf);
    std::vector<unsigned char> buffer = getRange(d._objectTable, headerPosition + 1, byteCount);
    reverse(buffer.begin(), buffer.end());

    return bytesToDouble(vecData(regulateNullBytes(buffer, 8)), hostLittleEndian());
}

// ---------------------------------------------------------------------------------------------------------------------
bool parseBinaryBool(const PlistHelperData& d, int headerPosition) {
    unsigned char header = d._objectTable[headerPosition];
    bool value;
    if (header == 0x09)
        value = true;
    else if (header == 0x08)
        value = false;
    else if (header == 0x00) {
        // null byte, not sure yet what to do with this.  It's in the spec but we
        // have never encountered it.

        throw Error("Plist: null byte encountered, unsure how to parse");
    } else if (header == 0x0F) {
        // fill byte, not sure yet what to do with this.  It's in the spec but we
        // have never encountered it.

        throw Error("Plist: fill byte encountered, unsure how to parse");
    } else {
        std::stringstream ss;
        ss << "Plist: unknown header " << header;
        throw Error(ss.str().c_str());
    }

    return value;
}

// ---------------------------------------------------------------------------------------------------------------------
Date parseBinaryDate(const PlistHelperData& d, int headerPosition) {
    // date always an 8 byte float starting after full byte header
    std::vector<unsigned char> buffer = getRange(d._objectTable, headerPosition + 1, 8);

    Date date;

    // Date is stored as Apple Epoch and big endian.
    date.setTimeFromAppleEpoch(bytesToDouble(vecData(buffer), false));

    return date;
}

// ---------------------------------------------------------------------------------------------------------------------
data_type parseBinaryByteArray(const PlistHelperData& d, int headerPosition) {
    unsigned char headerByte = d._objectTable[headerPosition];
    int byteStartPosition;
    int32_t byteCount = getCount(d, headerPosition, headerByte, byteStartPosition);
    byteStartPosition += headerPosition;

    return getRange((const char*)vecData(d._objectTable), byteStartPosition, byteCount);
}

// ---------------------------------------------------------------------------------------------------------------------
int32_t getCount(const PlistHelperData& d, int bytePosition, unsigned char headerByte, int& startOffset) {
    unsigned char headerByteTrail = headerByte & 0xf;
    if (headerByteTrail < 15) {
        startOffset = 1;
        return headerByteTrail;
    } else {
        int32_t count = (int32_t)parseBinaryInt(d, bytePosition + 1, startOffset);
        startOffset += 2;
        return count;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::string stringFromValue(const T& value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

// ---------------------------------------------------------------------------------------------------------------------
template <typename IntegerType>
IntegerType bytesToInt(const unsigned char* bytes, bool littleEndian) {
    IntegerType result = 0;
    if (littleEndian)
        for (int n = sizeof(result) - 1; n >= 0; n--) result = (result << 8) + bytes[n];
    else
        for (unsigned n = 0; n < sizeof(result); n++) result = (result << 8) + bytes[n];
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
double bytesToDouble(const unsigned char* bytes, bool littleEndian) {
    double result;
    int numBytes = sizeof(double);
    if (littleEndian)
        memcpy(&result, bytes, numBytes);
    else {
        std::vector<unsigned char> bytesReverse(numBytes);
        std::reverse_copy(bytes, bytes + numBytes, bytesReverse.begin());
        memcpy(&result, vecData(bytesReverse), numBytes);
    }
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> doubleToBytes(double val, bool littleEndian) {
    std::vector<unsigned char> result(sizeof(double));
    memcpy(vecData(result), &val, sizeof(double));
    if (!littleEndian) std::reverse(result.begin(), result.end());

    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
template <typename IntegerType>
std::vector<unsigned char> intToBytes(IntegerType val, bool littleEndian) {
    unsigned int numBytes = sizeof(val);
    std::vector<unsigned char> bytes(numBytes);

    for (unsigned n = 0; n < numBytes; ++n)
        if (littleEndian)
            bytes[n] = (val >> 8 * n) & 0xff;
        else
            bytes[numBytes - 1 - n] = (val >> 8 * n) & 0xff;

    return bytes;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> getRange(const unsigned char* origBytes, int64_t index, int64_t size) {
    std::vector<unsigned char> result((std::vector<unsigned char>::size_type)size);
    std::copy(origBytes + index, origBytes + index + size, result.begin());
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<char> getRange(const char* origBytes, int64_t index, int64_t size) {
    std::vector<char> result((std::vector<char>::size_type)size);
    std::copy(origBytes + index, origBytes + index + size, result.begin());
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<unsigned char> getRange(const std::vector<unsigned char>& origBytes, int64_t index, int64_t size) {
    if ((index + size) > (int64_t)origBytes.size()) throw Error("Out of bounds getRange");
    return getRange(vecData(origBytes), index, size);
}

}  // namespace Plist
