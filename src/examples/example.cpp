#include "../redisasyncclient.cpp"
#include <iostream>


class RedisWorker: public RedisAsyncClient {
public:
    void onRedisMessage(const RedisMessage& msg) {
        std::cout << "Redis " << type2str(msg.type()) << " message received:\n";
        for (size_t i = 0; i<msg.array().size(); ++i)
            std::cout << msg.array()[i] << std::endl;
    }
    void onLogMessage(const std::string& msg, int logLevel = RedisAsyncClient::LOG_LEVEL_INFO) {
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

    std::vector<std::string> args;
    args.push_back("HMSET");
    args.push_back("hashmap");
    args.push_back("key1");
    args.push_back("value1");
    args.push_back("key2");
    args.push_back("value2");
    args.push_back("key3");
    args.push_back("value3");
    redis.command(args);

    args.clear();
    args.push_back("HGET");
    args.push_back("hashmap");
    args.push_back("key1");
    redis.command(args);

    args.clear();
    args.push_back("HGETALL");
    args.push_back("hashmap");
    redis.command(args);

    char c;
    std::cin.read(&c,1);

    redis.close();
    return 0;
}

