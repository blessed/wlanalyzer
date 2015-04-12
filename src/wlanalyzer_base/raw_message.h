#ifndef RAW_MESSAGE_H
#define RAW_MESSAGE_H

#include <cstddef>
#include <vector>
#include <tr1/stdint.h>

namespace WlAnalyzer {

using std::vector;
using std::tr1::uint8_t;
using std::tr1::uint32_t;
using std::tr1::uint64_t;

/**
 * @brief The RawMessage class
 *        represents a parsed message intercepted
 *        from the traffic between the client and the compositor.
 */
class RawMessage
{
public:
    typedef vector<char> ByteVector;

    RawMessage() : is_initialized_(false) {}
    RawMessage(bool is_request, uint32_t sequence, uint64_t seconds, uint32_t useconds, const char *data);
    RawMessage(const RawMessage &other);

    uint32_t sequence() const {return sequence_;}
    uint64_t ts_seconds() const {return ts_seconds_;}
    uint32_t ts_useconds() const {return ts_useconds_;}
    const ByteVector &data() const {return data_;}

    bool is_big_endian() const {return (flags_ & FLAG_IS_BIG_ENDIAN) != 0;}
    bool is_request() const {return (flags_ & FLAG_IS_REQUEST) != 0;}
    bool is_event() const {return !is_request();}

    void Serialize(ByteVector *output) const;
    void Deserialize(const char *input, size_t size);
    void Deserialize(const ByteVector &input);

    static uint32_t GetWaylandMessageSize(const char *data);
    static uint32_t GetRawMessageSize(const char *data);

private:

    static const uint32_t FLAG_IS_REQUEST = 0x01;
    static const uint32_t FLAG_IS_BIG_ENDIAN = 0x02;

    void copyFromWayland(const char *data);

    bool is_initialized_;
    uint32_t sequence_;
    uint64_t ts_seconds_;
    uint32_t ts_useconds_;
    ByteVector data_;
    uint8_t flags_;
};

}   // namespace WlAnalyzer

#endif // RAW_MESSAGE_H
