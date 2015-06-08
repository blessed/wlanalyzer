#ifndef WLANALYZER_RAW_COMPOSITE_SINK_H
#define WLANALYZER_RAW_COMPOSITE_SINK_H

#include "raw_message_sink.h"

#include <unordered_set>

namespace WlAnalyzer {

using std::shared_ptr;

class RawCompositeSink : public RawMessageSink
{
public:
    void AddSink(const shared_ptr<RawMessageSink> &sink);
    void RemoveSink(const shared_ptr<RawMessageSink> &sink);

    void processMessage(const shared_ptr<const RawMessage> &message);

private:
    struct Hasher {
        size_t operator()(const shared_ptr<RawMessageSink> &rms) const {
			return std::hash<const RawMessageSink *>()(rms.get());
        }
    };

	typedef std::unordered_set<shared_ptr<RawMessageSink>, Hasher > SinkSet;

    SinkSet sinks_;
};

} // namespace WlAnalyzer

#endif // WLANALYZER_RAW_COMPOSITE_SINK_H
