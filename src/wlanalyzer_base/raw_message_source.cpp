#include "raw_message_source.h"

#include "raw_message_sink.h"


namespace WlAnalyzer {

void RawMessageSource::setSink(const shared_ptr<RawMessageSink> &sink)
{
    sink_ = sink;
}

void RawMessageSource::sendMessage(const shared_ptr<RawMessage> &message)
{
    sink_->processMessage(message);
}

}   // namespace WlAnalyzer
