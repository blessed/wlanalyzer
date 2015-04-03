#include "raw_composite_sink.h"

namespace WlAnalyzer {

void RawCompositeSink::AddSink(const shared_ptr<RawMessageSink> &sink)
{
    sinks_.insert(sink);
}

void RawCompositeSink::RemoveSink(const shared_ptr<RawMessageSink> &sink)
{
    sinks_.erase(sink);
}

void RawCompositeSink::processMessage(const shared_ptr<const RawMessage> &message)
{
    for (SinkSet::iterator it = sinks_.begin(); it != sinks_.end(); ++it) {
        (*it)->processMessage(message);
    }
}

} // namespace WlAnalyzer

namespace std {

}
