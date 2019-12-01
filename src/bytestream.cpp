#include "bytestream.h"
#include <iostream>
#include <cstdint>
#include <cstring>
using namespace std;

Bytestream::Bytestream()
{
  _size = 0;
  _pos = 0;
  _noOfNextBytes = 0;
  _noOfNextBytesValid = false;
  _endianness = BigEndian;
}

Bytestream::Bytestream(size_t len)
{
  _size = len;
  _data = new uint8_t[_size];
  memset(_data, 0, _size);
  _pos = 0;
  _noOfNextBytes = 0;
  _noOfNextBytesValid = false;
  _endianness = BigEndian;
}

Bytestream::Bytestream(const void* data, size_t len)
{
  _size = len;
  _data = new uint8_t[_size];
  memcpy(_data, data, _size);
  _pos = 0;
  _noOfNextBytes = 0;
  _noOfNextBytesValid = false;
  _endianness = BigEndian;
}
Bytestream::Bytestream(const void* data, size_t len, Endianness e)
{
  _size = len;
  _data = new uint8_t[_size];
  memcpy(_data, data, _size);
  _pos = 0;
  _noOfNextBytes = 0;
  _noOfNextBytesValid = false;
  _endianness = e;
}

Bytestream::Bytestream(const Bytestream& rhs)
{
  _size = rhs._size;
  _data = new uint8_t[_size];
  memcpy(_data, rhs._data, _size);
  _pos = rhs._pos;
  _noOfNextBytes = 0;
  _noOfNextBytesValid = false;
  _endianness = rhs._endianness;
}

Bytestream::~Bytestream()
{
  if(_size != 0)
  {
    delete _data;
  }
}

bool Bytestream::operator==(const Bytestream& other) const
{
  if(_size != other.size())
  {
    return false;
  }
  return memcmp(_data, other.raw(), _size) == 0;
}
bool Bytestream::operator!=(const Bytestream& other) const
{
  if(_size != other.size())
  {
    return true;
  }
  return memcmp(_data, other.raw(), _size) != 0;
}

Bytestream& Bytestream::operator=(const Bytestream& other)
{
  if(_size != 0)
  {
    delete _data;
  }
  _pos = other.pos();
  _size = other.size();
  _data = new uint8_t[_size];
  memcpy(_data, other.raw(), _size);
  return *this;
}

template <typename T>
T bswap(T u)
{
    uint8_t* const p = reinterpret_cast<uint8_t*>(&u);
    for (size_t i = 0; i < sizeof(T) / 2; i++)
    {
        std::swap(p[i], p[sizeof(T) - i - 1]);
    }
    return u;
}

#define GET(type, shorthand, len) GET_(type##len##_t, shorthand##len, len)
#define GET_(type, shortType, len) \
  type Bytestream::get##shortType() \
  {type tmp; getBytes(&tmp, sizeof(type));\
   if(needsSwap()){tmp=bswap(tmp);} return tmp;}

GET(uint, U, 8)
GET(uint, U, 16)
GET(uint, U, 32)
GET(uint, U, 64)
GET(int, S, 8)
GET(int, S, 16)
GET(int, S, 32)
GET(int, S, 64)
GET(float, F, 32)
GET(float, F, 64)

std::string Bytestream::getString()
{
  if(!_noOfNextBytesValid)
  {
    throw invalid_argument("No length given");
  }
  char* cs = new char[_noOfNextBytes+1];
  cs[_noOfNextBytes] = 0;
  getBytes(cs, _noOfNextBytes);
  string s = std::string(cs, _noOfNextBytes);
  delete cs;
  return s;
}
Bytestream Bytestream::getBytestream()
{
  if(!_noOfNextBytesValid)
  {
    throw invalid_argument("No length given");
  }
  uint8_t* cs = new uint8_t[_noOfNextBytes];
  getBytes(cs, _noOfNextBytes);
  Bytestream other = Bytestream(cs, _noOfNextBytes);
  delete cs;
  return other;
}
std::string Bytestream::getString(size_t len)
{
  if(_noOfNextBytesValid && len != _noOfNextBytes)
  {
    throw logic_error("Desired lengths does not match");
  }
  else if(!_noOfNextBytesValid)
  {
    setNoOfNextBytes(len);
  }
  return getString();
}
Bytestream Bytestream::getBytestream(size_t len)
{
  if(!_noOfNextBytesValid && len != _noOfNextBytes)
  {
    throw logic_error("Desired lengths does not match");
  }
  setNoOfNextBytes(len);
  return getBytestream();
}

void Bytestream::getBytes(void* cs,  size_t len)
 {
  _before(len);
  memcpy(cs, &(_data[_pos]), len);
  _after(len);
}


#define PEEK(type, shorthand, len) PEEK_(type##len##_t, shorthand##len, len)
#define PEEK_(type, shortType, len) \
  type Bytestream::peek##shortType() \
  {type tmp; getBytes(&tmp, sizeof(type));\
   if(needsSwap()){tmp=bswap(tmp);} (*this) -= sizeof(type); return tmp;}

PEEK(uint, U, 8)
PEEK(uint, U, 16)
PEEK(uint, U, 32)
PEEK(uint, U, 64)
PEEK(int, S, 8)
PEEK(int, S, 16)
PEEK(int, S, 32)
PEEK(int, S, 64)
PEEK(float, F, 32)
PEEK(float, F, 64)

std::string Bytestream::peekString()
{
  if(!_noOfNextBytesValid)
  {
    throw invalid_argument("No length given");
  }
  char* cs = new char[_noOfNextBytes+1];
  cs[_noOfNextBytes] = 0;
  getBytes(cs, _noOfNextBytes);
  string s = std::string(cs, _noOfNextBytes);
  delete cs;
  (*this) -= _noOfNextBytes;
  return s;
}
Bytestream Bytestream::peekBytestream()
{
  if(!_noOfNextBytesValid)
  {
    throw invalid_argument("No length given");
  }
  uint8_t* cs = new uint8_t[_noOfNextBytes];
  getBytes(cs, _noOfNextBytes);
  Bytestream other = Bytestream(cs, _noOfNextBytes);
  delete cs;
  (*this) -= _noOfNextBytes;
  return other;
}
std::string Bytestream::peekString(size_t len)
{
  if(_noOfNextBytesValid && len != _noOfNextBytes)
  {
    throw logic_error("Desired lengths does not match");
  }
  else if(!_noOfNextBytesValid)
  {
    setNoOfNextBytes(len);
  }
  return peekString();
}
Bytestream Bytestream::peekBytestream(size_t len)
{
  if(!_noOfNextBytesValid && len != _noOfNextBytes)
  {
    throw logic_error("Desired lengths does not match");
  }
  setNoOfNextBytes(len);
  return peekBytestream();
}

#define NEXT(type, shorthand, len) NEXT_(type##len##_t, shorthand##len)
#define NEXT_(type, shortType) \
  bool Bytestream::next##shortType(type u) \
  {if(u == get##shortType())\
     {return true;} \
   else\
  {(*this) -= sizeof(type);\
   return false;}}

NEXT(uint, U, 8)
NEXT(uint, U, 16)
NEXT(uint, U, 32)
NEXT(uint, U, 64)
NEXT(int, S, 8)
NEXT(int, S, 16)
NEXT(int, S, 32)
NEXT(int, S, 64)
NEXT(float, F, 32)
NEXT(float, F, 64)

bool Bytestream::nextString(const std::string& s)
{
  if(_noOfNextBytesValid && getNoOfNextBytes() != s.length())
  {
    throw logic_error("Desired length does not match const length");
  }
  else if(!_noOfNextBytesValid)
  {
    setNoOfNextBytes(s.length());
  }

  size_t noOfNextBytes = getNoOfNextBytes();

  if(noOfNextBytes > remaining())
  {
    invalidateNoOfNextBytes();
    return false;
  }

  if(getString() == s)
  {
    return true;
  }
  else
  {
    (*this) -= noOfNextBytes;
    return false;
  }
}
bool Bytestream::nextBytestream(const Bytestream& other)
{
  if(_noOfNextBytesValid && getNoOfNextBytes() != other.size())
  {
    throw logic_error("Desired length does not match const length");
  }
  else if(!_noOfNextBytesValid)
  {
    setNoOfNextBytes(other.size());
  }

  size_t noOfNextBytes = getNoOfNextBytes();

  if(noOfNextBytes > remaining())
  {
    invalidateNoOfNextBytes();
    return false;
  }

  if(getBytestream() == other)
  {
    return true;
  }
  else
  {
    (*this) -= noOfNextBytes;
    return false;
  }
}

#define PUT(type, shorthand, len) PUT_(type##len##_t, shorthand##len, len)
#define PUT_(type, shortType, len) \
  void Bytestream::put##shortType(type u) \
  {if(needsSwap()){u=bswap(u);} \
   putBytes(&u, sizeof(u));}

PUT(uint, U, 8)
PUT(uint, U, 16)
PUT(uint, U, 32)
PUT(uint, U, 64)
PUT(int, S, 8)
PUT(int, S, 16)
PUT(int, S, 32)
PUT(int, S, 64)
PUT(float, F, 32)
PUT(float, F, 64)

void Bytestream::putString(const std::string& s)
{
  putBytes(s.c_str(), s.length());
}
void Bytestream::putBytestream(const Bytestream& other)
{
  putBytes(other.raw(), other.size());
}

void Bytestream::putBytes(const void* c, size_t len)
{
  uint8_t* old = _data;
  _data = new uint8_t[_size+len];

  if (_size != 0)
  {
    memcpy(_data, old, _size);
    delete old;
  }
  memcpy((_data+_size), c, len);
  _size += len;
}

void Bytestream::setNoOfNextBytes(size_t n)
{
  _noOfNextBytes = n;
  _noOfNextBytesValid = true;
}

void Bytestream::invalidateNoOfNextBytes()
{
    _noOfNextBytes = 0;
    _noOfNextBytesValid = false;
}


void Bytestream::_before(size_t bytesToRead)
{
  if(bytesToRead > remaining())
  {
    invalidateNoOfNextBytes();
    throw out_of_range("Tried to read past end");
  }
}

void Bytestream::_after(size_t bytesRead)
{
  _pos += bytesRead;
  _noOfNextBytesValid = false;
}

Bytestream Bytestream::operator[](size_t i)
{
  Bytestream tmp(_data+i, _size-i);
  return tmp;
}

Bytestream& Bytestream::operator+=(size_t i)
{
  if((_pos+i) > _size)
  {
    invalidateNoOfNextBytes();
    throw out_of_range("Tried to address data past end");
  }
  _pos += i;
  return *this;
}

Bytestream& Bytestream::operator-=(size_t i)
{
  _pos -= i;
  return *this;
}

Bytestream& Bytestream::operator/(int i)
{
  setNoOfNextBytes(i);
  return *this;
}

#define PUTOP(type, shorthand, len) PUTOP_(type##len##_t, shorthand##len)
#define PUTOP_(type, shortType) \
  Bytestream& Bytestream::operator<<(const type& u) \
  {put##shortType(u); return *this;}

PUTOP(uint, U, 8)
PUTOP(uint, U, 16)
PUTOP(uint, U, 32)
PUTOP(uint, U, 64)
PUTOP(int, S, 8)
PUTOP(int, S, 16)
PUTOP(int, S, 32)
PUTOP(int, S, 64)
PUTOP(float, F, 32)
PUTOP(float, F, 64)

Bytestream& Bytestream::operator<<(const std::string& s)
{
  putString(s);
  return *this;
}
Bytestream& Bytestream::operator<<(const Bytestream& other)
{
  putBytestream(other);
  return *this;
}

#define GETOP(type, shorthand, len) GETOP_(type##len##_t, shorthand##len)
#define GETOP_(type, shortType) \
  Bytestream& Bytestream::operator>>(type& u) \
  {u = get##shortType(); return *this;}

GETOP(uint, U, 8)
GETOP(uint, U, 16)
GETOP(uint, U, 32)
GETOP(uint, U, 64)
GETOP(int, S, 8)
GETOP(int, S, 16)
GETOP(int, S, 32)
GETOP(int, S, 64)
GETOP(float, F, 32)
GETOP(float, F, 64)

Bytestream& Bytestream::operator>>(std::string& s)
{
  s = getString();
  return *this;
}
Bytestream& Bytestream::operator>>(Bytestream& other)
{
  other = getBytestream();
  return *this;
}

#define GETOP_CONST(type, shorthand, len) \
  GETOP_CONST_(type##len##_t, shorthand##len)
#define GETOP_CONST_(type, shortType) \
  Bytestream& Bytestream::operator>>(const type& u) \
  {type v = get##shortType();\
   if(u!=v) {(*this) -= sizeof(type);\
      throw Badmatch("Does not match const", v, u);}\
   else{return *this;}}

GETOP_CONST(uint, U, 8)
GETOP_CONST(uint, U, 16)
GETOP_CONST(uint, U, 32)
GETOP_CONST(uint, U, 64)
GETOP_CONST(int, S, 8)
GETOP_CONST(int, S, 16)
GETOP_CONST(int, S, 32)
GETOP_CONST(int, S, 64)
GETOP_CONST(float, F, 32)
GETOP_CONST(float, F, 64)

Bytestream& Bytestream::operator>>(const std::string& s)
{
  if (_noOfNextBytesValid && getNoOfNextBytes() != s.length())
  {
    throw logic_error("Desired length does not match const length");
  }
  else if(!_noOfNextBytesValid)
  {
    setNoOfNextBytes(s.length());
  }
  std::string sv = getString();
  if(sv != s)
  {
    (*this) -= s.length();
    throw Badmatch("Does not match const", sv, s);
  }
  return *this;
}

#define NEXTOP(type, shorthand, len) NEXTOP_(type##len##_t, shorthand##len)
#define NEXTOP_(type, shortType) \
  bool Bytestream::operator>>=(const type& u) \
  {return next##shortType(u);}

NEXTOP(uint, U, 8)
NEXTOP(uint, U, 16)
NEXTOP(uint, U, 32)
NEXTOP(uint, U, 64)
NEXTOP(int, S, 8)
NEXTOP(int, S, 16)
NEXTOP(int, S, 32)
NEXTOP(int, S, 64)
NEXTOP(float, F, 32)
NEXTOP(float, F, 64)
bool Bytestream::operator>>=(const std::string& s)
{
  return nextString(s);
}
bool Bytestream::operator>>=(const Bytestream& other)
{
  return nextBytestream(other);
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
bool Bytestream::needsSwap()
{
  return _endianness != Endianness::NativeEndian
      && _endianness != Endianness::LittleEndian;
}
#elif __BYTE_ORDER == __BIG_ENDIAN
bool Bytestream::needsSwap()
{
  return _endianness != Endianness::NativeEndian
      && _endianness != Endianness::BigEndian;
}
#else
#error
#endif
