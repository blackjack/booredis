#include <gtest/gtest.h>
#include <booredissync.h>

#include "../cleaner.h"

TEST(BooRedisSync, MultiBulk)
{
    const std::string TEST_KEY = "__BOOREDIS_TEST_KEY";
    const std::string TEST_HASH_KEY1 = "HKEY1";
    const std::string TEST_VALUE1 = "VALUE1";
    const std::string TEST_HASH_KEY2 = "HKEY2";
    const std::string TEST_VALUE2 = "VALUE2";
    Cleaner cleaner; cleaner << TEST_KEY;

    BooRedisSync redis;
    redis.connect("127.0.0.1",6379);

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

    ASSERT_STREQ("",redis.lastError().c_str());
    ASSERT_EQ(1,result.size());

    RedisMessage& msg = result.at(0);
    ASSERT_EQ(RedisMessage::Type_Array,msg.type());
    ASSERT_EQ(3,msg.array().size());
    EXPECT_STREQ(TEST_VALUE1.c_str(),msg.array().at(0).c_str());
    EXPECT_TRUE(msg.array().at(1).empty());
    EXPECT_STREQ(TEST_VALUE2.c_str(),msg.array().at(2).c_str());
}
