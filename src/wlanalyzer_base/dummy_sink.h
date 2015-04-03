#ifndef WLANALYZER_DUMMY_SINK_H
#define WLANALYZER_DUMMY_SINK_H

#include "raw_message_sink.h"

namespace WlAnalyzer {

class DummySink : public RawMessageSink
{
public:
    DummySink();

    void processMessage(const shared_ptr<const RawMessage> &message);
};

} // namespace WlAnalyzer

#endif // WLANALYZER_DUMMY_SINK_H
