#include "raw_buffer_source.h"

#include <cstring>
#include <cstdio>

#include "raw_message.h"

namespace WlAnalyzer {

void RawBufferSource::processBuffer(const char *buffer, size_t size)
{
    // first, try to copy missing data
    size_t buffer_index = 0;
    if (!message_complete_) {
        size_t to_copy = std::min(message_length_ - message_index_, size);
        std::memcpy(&message_buffer_[message_index_], buffer, to_copy);
        message_index_ += to_copy;
        buffer_index += to_copy;
        if (message_index_ == message_length_) {
            shared_ptr<RawMessage> message(new RawMessage());
            message->Deserialize(&message_buffer_[0], message_length_);
            sendMessage(message);
            message_complete_ = true;
        }
    }
    // process the frame
    while (buffer_index < size) {
        message_length_ = RawMessage::GetRawMessageSize(buffer + buffer_index);
        if (message_length_ <= size - buffer_index) {
            // in place creation
            shared_ptr<RawMessage> message(new RawMessage());
            message->Deserialize(buffer + buffer_index, message_length_);
            sendMessage(message);
            buffer_index += message_length_;

        } else {
            // make a copy
            if (message_length_ > message_buffer_.size()) {
                message_buffer_.resize(message_length_);
            }
            memcpy(&message_buffer_[0], buffer + buffer_index, size - buffer_index);
            message_index_ = size - buffer_index;
            buffer_index = size;
            message_complete_ = false;
        }
    }
}

} // namespace WlAnalyzer

