#include <iostream>

#include "booredis/sync.h"


int main( int argc, const char* argv[] ) {

    BooRedisSync redis;
    if (!redis.connect("localhost",6379))
        std::cout << redis.lastError() << std::endl;

    std::vector<RedisMessage> msgs = redis.command("GET MCX_state\r\n");

    if (!redis.lastError().empty()) {
        std::cout << "Error: " << redis.lastError() << std::endl;
    }

    //There still can be decoded messages even if error occured
    for (size_t i = 0; i<msgs.size(); ++i) {
        RedisMessage& msg = msgs[i];
        if (!msg.empty())
            std::cout << msg.string() << std::endl;
    }

    char c;
    std::cin.read(&c,1);

    redis.disconnect();
    return 0;
}

