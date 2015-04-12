#ifndef WLANALYZER_RAWBUFFERSOURCE_H
#define WLANALYZER_RAWBUFFERSOURCE_H

#include "raw_message_source.h"

#include <vector>

namespace WlAnalyzer {

/**
 * @brief The RawBufferSource class
 *        processes raw input and extracts
 *        \see RawMessage objects from input
 *        data.
 */
class RawBufferSource : public RawMessageSource
{
public:
    RawBufferSource() : message_complete_(true) {}

    /**
     * @brief processBuffer
     * @param buffer data from which to extract \see RawMessage objects
     * @param size size of the input buffer
     */
    void processBuffer(const char *buffer, size_t size);

private:
    bool message_complete_;
    size_t message_index_;
    size_t message_length_;
    std::vector<char> message_buffer_;

};

} // namespace WlAnalyzer

#endif // WLANALYZER_RAWBUFFERSOURCE_H
