#ifndef RAW_SOURCE_H
#define RAW_SOURCE_H



class RawMessage {
public:
    RawMessage(timestamp, size_t seq, const char *data, const char *ctrl);
    RawMessage(const RawMessage & other);
    ~RawMessage();
};

class RawMessageSink {
public:
    virtual void Receive(RawMessage *message) = 0;
};

class RawMessageSource {
public:
    void SetSink(RawMessageSink *sink);
protected:
    void Send(RawMessage *message) {
        sink.Receive(message);
    }
private:
    RawMessageSink sink;
};


// Raw sources
class WlaProxy : public RawMessageSource {

};

class NetworkSource : public RawMessageSource {

};

class FileSource : public RawMessageSource {

};

// Raw sinks
class FileSink : public RawMessageSink {

};

class NetworkSink : public RawMessageSink {

};

class CompositeSink : public RawMessageSink {
public:
    void AddSink(RawMessageSink *sink);
};

// source
class MessageSource {
public:
    void SetSink(MessageSink *sink);
protected:
    void Send(Message *message);
private:
    MessageSink sink;
};

// Converter
class Converter : public RawMessageSink, public MessageSource {

};


int konfiguracja() {
    RawMessageSource *source = NULL;
    RawMessageSink *sink = NULL;

    switch(source) {
    case LIVE:
        source = new WlaProxy();
        break;
    case FILE:
        source = new FileSource("asas");
        break;
    case NETWORK:
        source = new
    }

}





#endif // RAW_SOURCE_H
