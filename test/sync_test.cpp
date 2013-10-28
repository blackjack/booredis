#include <gtest/gtest.h>
#include <booredissync.h>

TEST(BooRedisSync,String)
{
    const std::string TEST_KEY = "__BOOREDIS_TEST_KEY";
    const std::string TEST_VALUE = "VALUE";

    BooRedisSync redis;

    redis.connect("localhost",6379);

    std::vector<std::string> cmd;
    cmd.push_back("SET");
    cmd.push_back(TEST_KEY);
    cmd.push_back(TEST_VALUE);

    std::vector<RedisMessage> result = redis.command(cmd);

    EXPECT_TRUE(redis.lastError().empty());
    EXPECT_EQ(1,result.size());

    RedisMessage& msg = result[0];
    EXPECT_EQ(RedisMessage::Type_String,msg.type());
    EXPECT_EQ(1,msg.array().size());
    EXPECT_STREQ("OK",msg.string().c_str());
}

