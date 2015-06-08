#ifndef RAW_MESSAGE_SINK_H
#define RAW_MESSAGE_SINK_H

#include <memory>

namespace WlAnalyzer {

using std::shared_ptr;

class RawMessage;

class RawMessageSink {
public:
    virtual ~RawMessageSink() {}
	virtual void processMessage(const shared_ptr<const RawMessage> &message) = 0;
};

}   // namespace WlAnalyzer

#endif // RAW_MESSAGE_SINK_H
