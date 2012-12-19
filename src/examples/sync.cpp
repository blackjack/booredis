#include "../booredissync.h"
#include <iostream>


int main( int argc, const char* argv[] ) {

    BooRedisSync redis;
    if (!redis.connect("localhost",6379))
        std::cout << redis.lastError() << std::endl;

    RedisMessage msg = redis.command("GET MCX_state\r\n");
    if (!msg.empty())
        std::cout << msg.string() << std::endl;
    else
        std::cout << redis.lastError() << std::endl;
    char c;
    std::cin.read(&c,1);

    redis.disconnect();
    return 0;
}

