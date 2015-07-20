#ifndef WLANALYZER_WAYLAND_RAW_SOURCE_H
#define WLANALYZER_WAYLAND_RAW_SOURCE_H

#include <vector>
#include <tr1/stdint.h>

#include "raw_message_source.h"

namespace WlAnalyzer {

using std::tr1::uint32_t;
using std::tr1::uint64_t;

class WaylandRawSource : public RawMessageSource
{
public:

    static void setSequence(const uint32_t &sequence) {sequence_ = sequence;}

    explicit WaylandRawSource(bool request_source) :
        creates_requests_(request_source), message_complete_(true) {}

    void processBuffer(uint64_t ts_seconds, uint32_t ts_useconds, const char *buffer, size_t size);

private:
    static uint32_t sequence_;
    bool creates_requests_;
    bool message_complete_;
    size_t message_index_;
    size_t message_length_;
    size_t message_sequence_;
    uint64_t message_seconds_;
    uint64_t message_useconds_;
    std::vector<char> message_buffer_;
};

} // namespace WlAnalyzer

#endif // WLANALYZER_WAYLAND_RAW_SOURCE_H
