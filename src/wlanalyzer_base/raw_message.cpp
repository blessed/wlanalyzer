#include "raw_message.h"

#include <cassert>
#include <cstring>
#include <arpa/inet.h>

#include "common.h"

namespace WlAnalyzer {

using std::memcpy;

static const size_t WORD_SIZE = 4;

static void writeSingleWord(char *dst, uint32_t value);
static uint32_t readSingleWord(const char *src);
static void writeDoubleWord(char *dst, uint64_t value);
static uint64_t readDoubleWord(const char *src);

RawMessage::RawMessage(bool is_request, uint32_t sequence, uint64_t seconds, uint32_t useconds, const char *data) :
    sequence_(sequence), ts_seconds_(seconds), ts_useconds_(useconds), flags_(0)
{
    if (is_request) {
        flags_ |= FLAG_IS_REQUEST;
    }
    copyFromWayland(data);
    is_initialized_ = true;
}

RawMessage::RawMessage(const RawMessage &other) :
        is_initialized_(other.is_initialized_), sequence_(other.sequence_),
        ts_seconds_(other.ts_useconds_), ts_useconds_(other.ts_useconds_),
        data_(other.data_), flags_(other.flags_)
{
}

static void writeSingleWord(char *dst, uint32_t value)
{
    value = htonl(value);
    memcpy(dst, &value, sizeof(value));
}

static uint32_t readSingleWord(const char *src)
{
    uint32_t value;
    memcpy(&value, src, sizeof(value));
    return ntohl(value);
}


static void writeDoubleWord(char *dst, uint64_t value)
{
    writeSingleWord(dst, value >> 32);
    writeSingleWord(dst + WORD_SIZE, value & UINT32_MAX);
}

static uint64_t readDoubleWord(const char *src)
{
    uint64_t value;
    value = readSingleWord(src);
    value <<= 32;
    value |= readSingleWord(src + WORD_SIZE);
    return value;
}

/*
 * Serialization uses network order for all values.
 *
 * Structure:
 * - uint32_t -
 *      - 3 upper bytes - total byte size of serialization message
 *      - 1 lower byte - message flags
 * - uint32_t - sequence number
 * - uint64_t - timestamp seconds
 * - uint32_t - timestamp microseconds
 * - uint32_t - byte size of Wayland message
 * - payload with Wayland message - all words are in network order!
 *
 * Alignment to 4 bytes of the header fields is intentional.
 */

static const size_t SERIALIZATION_HEADER_SIZE = (1 + 1 + 2 + 1 + 1)*WORD_SIZE;

void RawMessage::Serialize(ByteVector *output) const
{
    assert(is_initialized_ && "Data is not initialized");
    uint32_t total_size = SERIALIZATION_HEADER_SIZE + data_.size();
    output->resize(total_size);
    uint32_t idx = 0;
    uint32_t temp;
    // total size
    temp = total_size << 8;
    temp |= flags_;
    writeSingleWord(&(*output)[idx], temp);
    idx += WORD_SIZE;
    // sequence
    writeSingleWord(&(*output)[idx], sequence_);
    idx += WORD_SIZE;
    // timestamp seconds
    writeDoubleWord(&(*output)[idx], ts_seconds_);
    idx += 2*WORD_SIZE;
    // timestamp microseconds
    writeSingleWord(&(*output)[idx], ts_useconds_);
    idx += WORD_SIZE;
    // byte size of Wayland message & byte order
    writeSingleWord(&(*output)[idx], data_.size());
    idx += WORD_SIZE;
    // payload
    memcpy(&(*output)[idx], &data_[0], data_.size());
    idx += data_.size();
    assert(idx == total_size);
}

void RawMessage::Deserialize(const char *input, size_t size)
{
    assert(size >= SERIALIZATION_HEADER_SIZE);
    uint32_t total_size;
    uint32_t idx = 0;
    uint32_t temp;
    // total size
    temp = readSingleWord(input+idx);
    total_size = temp >> 8;
    flags_ = temp & 0xff;
    idx += WORD_SIZE;
    // sequence
    sequence_ = readSingleWord(input+idx);
    idx += WORD_SIZE;
    // timestamp seconds
    ts_seconds_ = readDoubleWord(input+idx);
    idx += 2*WORD_SIZE;
    // timestamp microseconds
    ts_useconds_ = readSingleWord(input+idx);
    idx += WORD_SIZE;
    // byte size of Wayland message & byte order
    uint32_t data_length = readSingleWord(input+idx);
    data_.resize(data_length);
    idx += WORD_SIZE;
    // payload
    memcpy(&data_[0], input+idx, data_.size());
    idx += data_.size();
    assert(idx == total_size);
}

void RawMessage::Deserialize(const ByteVector &input)
{
    Deserialize(&input[0], input.size());
}

void RawMessage::copyFromWayland(const char *data)
{
    size_t data_length = GetWaylandMessageSize(data);
    data_.resize(data_length);
    memcpy(&data_[0], data, data_length);
    // check byte order
    bool is_big_endian = (0x12345678 == htonl(0x12345678));
    if (is_big_endian) {
        flags_ |= FLAG_IS_BIG_ENDIAN;
    }
}


uint32_t RawMessage::GetWaylandMessageSize(const char *data)
{
    /*
     * Wayland message starts with 2 words. Each word is uint32_t in host byte order.
     * | word0 | word1 |
     * Upper 16 bits of the word1 contains the total byte size of message
     */
    uint32_t word;
    memcpy(&word, data + WORD_SIZE, WORD_SIZE);
    uint32_t data_length = word >> 16;
    assert(data_length % 4 == 0 && "Message length must be multiple of a word (4 bytes)");
    return data_length;
}

uint32_t RawMessage::GetRawMessageSize(const char *data)
{
    /*
     * Wayland message starts with 2 words. Each word is uint32_t in host byte order.
     * | word0 | word1 |
     * Upper 16 bits of the word1 contains the total byte size of message
     */
    uint32_t word = readSingleWord(data);
    uint32_t data_length = word >> 8;
    assert(data_length % 4 == 0 && "Message length must be multiple of a word (4 bytes)");
    return data_length;
}

}
