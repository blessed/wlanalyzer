#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../wlanalyzer_base/raw_buffer_source.h"
#include "../wlanalyzer_base/dummy_sink.h"

using namespace WlAnalyzer;

void die()
{
    puts("Arghhhhhhhhhhhhhhhhhhhhhhhhhhhhh....\n");
    exit(EXIT_FAILURE);
}

int main()
{
    RawBufferSource source;
    source.setSink(shared_ptr<RawMessageSink>(new DummySink()));
    int fd = open("./file1.wldump", O_RDONLY);
    if (fd < 0) {
        perror(NULL);
        die();
    }
    char buf[4096];
    int ret = read(fd, buf, sizeof(buf));
    while (ret > 0) {
        source.processBuffer(buf, ret);
        ret = read(fd, buf, sizeof(buf));
    }
    puts("I'm done with you!");
    return EXIT_SUCCESS;
}
