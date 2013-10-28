#include <gtest/gtest.h>
#include <booredissync.h>

struct Cleaner {
    Cleaner() {
        keys.push_back("DEL");
    }
    ~Cleaner() {
        BooRedisSync redis;
        redis.connect("localhost",6379);
        redis.command(keys);
    }
    Cleaner& operator << (const std::string& key) {
        keys.push_back(key);
        return *this;
    }
    std::vector<std::string> keys;
};


TEST(BooRedisSync,String)
{
    const std::string TEST_KEY = "__BOOREDIS_TEST_KEY";
    const std::string TEST_VALUE = "VALUE";

    BooRedisSync redis;
    Cleaner cleaner; cleaner << TEST_KEY;

    redis.connect("localhost",6379);

    std::vector<std::string> cmd;
    cmd.push_back("SET");
    cmd.push_back(TEST_KEY);
    cmd.push_back(TEST_VALUE);

    std::vector<RedisMessage> result = redis.command(cmd);

    EXPECT_TRUE(redis.lastError().empty());
    EXPECT_EQ(1,result.size());

    RedisMessage& msg = result.at(0);
    EXPECT_EQ(RedisMessage::Type_String,msg.type());
    EXPECT_EQ(1,msg.array().size());
    EXPECT_STREQ("OK",msg.string().c_str());
}

TEST(BooRedisSync, Error) {
    BooRedisSync redis;

    redis.connect("localhost",6379);

    std::vector<std::string> cmd;
    cmd.push_back("ERROR_COMMAND");

    std::vector<RedisMessage> result = redis.command(cmd);

    EXPECT_TRUE(redis.lastError().empty());
    EXPECT_EQ(1,result.size());

    RedisMessage& msg = result.at(0);
    EXPECT_EQ(RedisMessage::Type_Error,msg.type());
    EXPECT_EQ(1,msg.array().size());
    EXPECT_STREQ("ERR unknown command 'ERROR_COMMAND'",msg.string().c_str());
}

TEST(BooRedisSync, Integer)
{
    const std::string TEST_KEY = "__BOOREDIS_TEST_KEY";
    const std::string TEST_VALUE = "VALUE";

    BooRedisSync redis;
    redis.connect("localhost",6379);

    std::vector<std::string> cmd;
    cmd.push_back("SET");
    cmd.push_back(TEST_KEY);
    cmd.push_back(TEST_VALUE);
    redis.command(cmd);
    cmd.clear();

    cmd.push_back("DEL");
    cmd.push_back(TEST_KEY);

    std::vector<RedisMessage> result = redis.command(cmd);

    EXPECT_TRUE(redis.lastError().empty());
    EXPECT_EQ(1,result.size());

    RedisMessage& msg = result.at(0);
    EXPECT_EQ(RedisMessage::Type_Integer,msg.type());
    EXPECT_EQ(1,msg.array().size());
    EXPECT_EQ(1,msg.integer());
}

TEST(BooRedisSync, Bulk)
{
    const std::string TEST_KEY = "__BOOREDIS_TEST_KEY";
    const std::string TEST_VALUE = "VALUE";

    Cleaner cleaner; cleaner << TEST_KEY;

    BooRedisSync redis;
    redis.connect("localhost",6379);

    std::vector<std::string> cmd;
    cmd.push_back("SET");
    cmd.push_back(TEST_KEY);
    cmd.push_back(TEST_VALUE);
    redis.command(cmd);
    cmd.clear();

    cmd.push_back("GET");
    cmd.push_back(TEST_KEY);

    std::vector<RedisMessage> result = redis.command(cmd);

    EXPECT_TRUE(redis.lastError().empty());
    EXPECT_EQ(1,result.size());

    RedisMessage& msg = result.at(0);
    EXPECT_EQ(RedisMessage::Type_String,msg.type());
    EXPECT_EQ(1,msg.array().size());
    EXPECT_STREQ(TEST_VALUE.c_str(),msg.string().c_str());
}

TEST(BooRedisSync, MultiBulk)
{
    const std::string TEST_KEY = "__BOOREDIS_TEST_KEY";
    const std::string TEST_HASH_KEY1 = "HKEY1";
    const std::string TEST_VALUE1 = "VALUE1";
    const std::string TEST_HASH_KEY2 = "HKEY2";
    const std::string TEST_VALUE2 = "VALUE2";

    Cleaner cleaner; cleaner << TEST_KEY;

    BooRedisSync redis;
    redis.connect("localhost",6379);

    std::vector<std::string> cmd;
    cmd.push_back("HMSET");
    cmd.push_back(TEST_KEY);
    cmd.push_back(TEST_HASH_KEY1);
    cmd.push_back(TEST_VALUE1);
    cmd.push_back(TEST_HASH_KEY2);
    cmd.push_back(TEST_VALUE2);
    redis.command(cmd);
    cmd.clear();

    cmd.push_back("HMGET");
    cmd.push_back(TEST_KEY);
    cmd.push_back(TEST_HASH_KEY1);
    cmd.push_back("NOT_EXISTING_HASH_KEY");
    cmd.push_back(TEST_HASH_KEY2);

    std::vector<RedisMessage> result = redis.command(cmd);

    EXPECT_TRUE(redis.lastError().empty());
    EXPECT_EQ(1,result.size());

    RedisMessage& msg = result.at(0);
    EXPECT_EQ(RedisMessage::Type_Array,msg.type());
    EXPECT_EQ(3,msg.array().size());
    EXPECT_STREQ(TEST_VALUE1.c_str(),msg.array().at(0).c_str());
    EXPECT_TRUE(msg.array()[1].empty());
    EXPECT_STREQ(TEST_VALUE2.c_str(),msg.array().at(2).c_str());
}
