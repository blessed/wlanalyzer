#include "raw_fd_sink.h"

#include <cassert>

#include "raw_message.h"

namespace WlAnalyzer {


void RawFdSink::processMessage(const shared_ptr<const RawMessage> &message)
{
    message->Serialize(&buffer_);
    ssize_t ret = write(fd_, &buffer_[0], buffer_.size());
    if (ret < 0 || static_cast<size_t>(ret) != buffer_.size()) {
        assert(0 && "Write was not complete! I cannot handle this anymore!");
    }
}

} // namespace WlAnalyzer
