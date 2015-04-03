#ifndef WLANALYZER_RAW_FD_SINK_H
#define WLANALYZER_RAW_FD_SINK_H

#include <unistd.h>

#include "raw_message_sink.h"
#include "raw_message.h"

namespace WlAnalyzer {


class RawFdSink : public RawMessageSink
{
public:
    explicit RawFdSink(int fd) : fd_(fd) {}
    ~RawFdSink() {close(fd_);}

    void processMessage(const shared_ptr<const RawMessage> &message);

private:
    int fd_;
    RawMessage::ByteVector buffer_;

};

} // namespace WlAnalyzer

#endif // WLANALYZER_RAW_FD_SINK_H
