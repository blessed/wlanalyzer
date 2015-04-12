#include "dummy_sink.h"

#include <iostream>
#include "raw_message.h"

namespace WlAnalyzer {

DummySink::DummySink()
{

}

void DummySink::processMessage(const shared_ptr<const RawMessage> &message)
{
    std::cout << "Got " << message->data().size() << " bytes! R=" << message->is_request() << std::endl;
}

} // namespace WlAnalyzer
