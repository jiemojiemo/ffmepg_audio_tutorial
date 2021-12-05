//
// Created by bytedance on 2021/12/3.
//

#ifndef TESTING_FILE_BYTE_STREAM_H
#define TESTING_FILE_BYTE_STREAM_H
#include "byte_stream.h"
#include <iostream>
namespace mammon
{
class FileByteStream : public ByteStream
{
public:
    FileByteStream() = default;
    explicit FileByteStream(const std::string& path);
    
    ~FileByteStream() override;

    bool isOpen() const;

    int open(const std::string& path);

    void close();

    int64_t write(const uint8_t* source_buf, int64_t size) override;

    int64_t read(uint8_t* dest_buf, int64_t size) override;

    int64_t tellp() const override;

    int seekp(int64_t pos, int mode) override;

    int64_t length() override;

private:
    FILE* fp_{nullptr};
};

FileByteStream::FileByteStream(const std::string& path)
{
    open(path);
}

FileByteStream::~FileByteStream()
{
    close();
}

bool FileByteStream::isOpen() const
{
    return fp_ != nullptr;
}

int FileByteStream::open(const std::string& path)
{
    fp_ = fopen(path.c_str(), "wb+");
    if(!fp_){
        return -1;
    }
    return 0;
}

void FileByteStream::close()
{
    if(fp_)
    {
        fclose(fp_);
        fp_ = nullptr;
    }
}

int64_t FileByteStream::write(const uint8_t* source_buf, int64_t size)
{
    if(!isOpen()){
        return -1;
    }

    return fwrite(source_buf, sizeof(uint8_t), size, fp_);
}

int64_t FileByteStream::read(uint8_t* dest_buf, int64_t size)
{
    if(!isOpen()){
        return -1;
    }
    auto ret = fread(dest_buf, 1, size, fp_);
    if(ret != size){
        if(ferror(fp_))
        {
            std::cerr << "error reading" << std::endl;
            return -1;
        }
    }
    return ret;
}


int64_t FileByteStream::tellp() const
{
    if(!isOpen()){
        return 0;
    }

    return ftell(fp_);
}

int FileByteStream::seekp(int64_t pos, int mode)
{
    if(!isOpen()){
        return -1;
    }
    return fseek(fp_, pos, mode);
}

int64_t FileByteStream::length()
{
    if(!isOpen()){
        return 0;
    }else
    {
        auto cur_pos = tellp();
        seekp(0, SEEK_END);
        auto length = tellp();
        seekp(cur_pos, SEEK_SET);

        return length;
    }

}

}


#endif  // TESTING_FILE_BYTE_STREAM_H
