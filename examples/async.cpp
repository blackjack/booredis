#include <iostream>

#include "booredis/async.h"


class RedisWorker: public BooRedisAsync {
public:
    void onRedisMessage(const RedisMessage& msg) {
        std::cout << "Redis " << type2str(msg.type()) << " message received:\n";
        for (size_t i = 0; i<msg.array().size(); ++i)
            std::cout << msg.array()[i] << std::endl;
    }
    void onLogMessage(const std::string& msg, int logLevel = BooRedisAsync::LOG_LEVEL_INFO) {
        std::cout << "Redis log message: " << msg << std::endl;
    }

private:
    std::string type2str(RedisMessage::MessageType type) {
        switch (type) {
        case RedisMessage::Type_String: return "String";
        case RedisMessage::Type_Integer: return "Integer";
        case RedisMessage::Type_Array: return "Array";
        default: return "Error";
        }
    }
};


int main( int argc, const char* argv[] ) {

    RedisWorker redis;
    redis.connect("localhost",6379,10000);

    redis.command("SELECT 2\r\n");
    for (;;) {
        std::vector<std::string> args;
        args.push_back("SUBSCRIBE");
        args.push_back("SOMESHIT");
        redis.command(args);
        sleep(1);
    }

    char c;
    std::cin.read(&c,1);

    redis.disconnect();
    return 0;
}

