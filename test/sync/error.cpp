#include <gtest/gtest.h>
#include <booredissync.h>

#include "../cleaner.h"

TEST(BooRedisSync, Error) {
    BooRedisSync redis;

    redis.connect("localhost",6379);

    std::vector<std::string> cmd;
    cmd.push_back("ERROR_COMMAND");

    std::vector<RedisMessage> result = redis.command(cmd);

    ASSERT_TRUE(redis.lastError().empty());
    ASSERT_EQ(1,result.size());

    RedisMessage& msg = result.at(0);
    EXPECT_EQ(RedisMessage::Type_Error,msg.type());
    ASSERT_EQ(1,msg.array().size());
    EXPECT_STREQ("ERR unknown command 'ERROR_COMMAND'",msg.string().c_str());
}
