#include "curlrequester.h"
#include <algorithm>

CurlRequester::CurlRequester(QUrl addr) : _addr(addr), _canWrite(1), _canRead(), _reading(false), _done(false), _dest(nullptr), _size(0), _offset(0), _performer(addr, this)
{

    connect(&_performer, &CurlWorker::done, this, &CurlRequester::done);
    _performer.start();
}

CurlRequester::~CurlRequester()
{
    _canWrite.acquire();
    _done = true;
    _canRead.release();
    _performer.wait();

    if(_dest != nullptr)
    {
        delete _dest;
    }
}

void CurlRequester::write(const char *data, size_t size)
{
    qDebug() << "write " << size;
    _canWrite.acquire();

    if(_dest != nullptr)
    {
        delete _dest;
    }
    _dest = new char[size];
    memcpy(_dest, data, size);
    _size = size;
    _offset = 0;
    _canRead.release();
}

size_t CurlRequester::requestWrite(char* dest, size_t size)
{
    if(!_reading)
    {
        _canRead.acquire();
        if(_done) // Can only have been set by write() - only relevant to check if strating to write
        {
            return 0;
        }
        _reading = true;
    }

    size_t remaining = _size - _offset;

    size_t actualSize = std::min(size, remaining);

    memcpy(dest, (_dest+_offset), actualSize);
    _offset += actualSize;

    remaining = _size - _offset;
    if(remaining == 0)
    {
        _reading = false;
        _canWrite.release();
    }
    return actualSize;
}

void CurlRequester::done(CURLcode)
{

}
