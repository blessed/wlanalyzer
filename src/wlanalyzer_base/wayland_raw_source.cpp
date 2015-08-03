#include "wayland_raw_source.h"

#include <algorithm>
#include <cstring>

#include "raw_message.h"

namespace WlAnalyzer {

uint32_t WaylandRawSource::sequence_ = 0;

void WaylandRawSource::processBuffer(bool is_request, uint64_t ts_seconds, uint32_t ts_useconds, const char *buffer, size_t size)
{
    // first, try to copy missing data
    size_t buffer_index = 0;
    if (!message_complete_) {
        size_t to_copy = std::min(message_length_ - message_index_, size);
        std::memcpy(&message_buffer_[message_index_], buffer, to_copy);
        message_index_ += to_copy;
        buffer_index += to_copy;
        if (message_index_ == message_length_) {
            shared_ptr<RawMessage> message(
                        new RawMessage(message_is_request_, message_sequence_, message_seconds_,
                                       message_useconds_, &message_buffer_[0]));
            sendMessage(message);
            message_complete_ = true;
        }
    }
    // process the frame
    while (buffer_index < size) {
        message_length_ = RawMessage::GetWaylandMessageSize(buffer + buffer_index);
        message_sequence_ = sequence_++;    // Update of the static value!
        message_seconds_ = ts_seconds;
        message_useconds_ = ts_useconds;
        if (message_length_ <= size - buffer_index) {
            // in place creation
            shared_ptr<RawMessage> message(
                        new RawMessage(is_request, message_sequence_, message_seconds_,
                                       message_useconds_, buffer+buffer_index));
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
            message_is_request_ = is_request;
        }
    }
}



} // namespace WlAnalyzer
