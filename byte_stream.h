//
// Created by william on 2021/12/3.
//

#ifndef BYTE_STREAM_H
#define BYTE_STREAM_H

namespace mammon
{
class ByteStream
{
public:
    virtual ~ByteStream() = default;
    /**
     * write to file
     * @param source_buf the source of data
     * @param size number of size to write
     * @return number of written bytes, returns -1 if failed
     */
    virtual int64_t write(const uint8_t* source_buf, int64_t size) = 0;

    /**
     * read from file
     * @param dest_buf the destination buffer
     * @param size size of bytes
     * @return number of read bytes, return -1 if failed
     */
    virtual int64_t read(uint8_t* dest_buf, int64_t size) = 0;

    /**
     * returns the write pointer position
     */
    virtual int64_t tellp() const = 0;

    /**
     * seek the write pointer
     * @param pos the new pos
     * @param mode SEEK_SET	Beginning of file
                   SEEK_CUR	Current position of the file pointer
                   SEEK_END End of file
     * @return 0 if seek successfully, others failed
     */
    virtual int seekp(int64_t pos, int mode) = 0;

    /**
     * return the length of output stream
     */
    virtual int64_t length() = 0;
};
}

#endif  // BYTE_STREAM_H
