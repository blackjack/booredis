#include <gtest/gtest.h>

#include <booredis/sync.h>

#include "../cleaner.h"

TEST(BooRedisSync,String)
{
    const std::string TEST_KEY = "__BOOREDIS_TEST_KEY";
    const std::string TEST_VALUE = "VALUE";
    Cleaner cleaner; cleaner << TEST_KEY;

    BooRedisSync redis;

    redis.connect("127.0.0.1",6379);

    std::vector<std::string> cmd;
    cmd.push_back("SET");
    cmd.push_back(TEST_KEY);
    cmd.push_back(TEST_VALUE);

    std::vector<RedisMessage> result = redis.command(cmd);

    ASSERT_STREQ("",redis.lastError().c_str());
    ASSERT_EQ(1,result.size());

    RedisMessage& msg = result.at(0);
    EXPECT_EQ(RedisMessage::Type_String,msg.type());
    ASSERT_EQ(1,msg.array().size());
    EXPECT_STREQ("OK",msg.string().c_str());
}
