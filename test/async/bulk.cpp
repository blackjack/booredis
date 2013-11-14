#include <gtest/gtest.h>
#include <booredisasync.h>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include "../cleaner.h"
#include "asynctestobject.h"

TEST(BooRedisAsync, Bulk)
{
    const std::string TEST_KEY = "__BOOREDIS_TEST_KEY";
    const std::string TEST_VALUE = "VALUE";
    Cleaner cleaner; cleaner << TEST_KEY;


    AsyncTestObject redis;
    redis.connect("127.0.0.1",6379,1000);

    std::vector<std::string> cmd;
    cmd.push_back("SET");
    cmd.push_back(TEST_KEY);
    cmd.push_back(TEST_VALUE);
    redis.command(cmd);
    cmd.clear();

    cmd.push_back("GET");
    cmd.push_back(TEST_KEY);
    boost::unique_future<RedisMessage> future = redis.command(cmd);

    ASSERT_TRUE(future.timed_wait(boost::posix_time::seconds(1)));

    RedisMessage msg = future.get();
    EXPECT_EQ(RedisMessage::Type_String,msg.type());
    ASSERT_EQ(1,msg.array().size());
    EXPECT_STREQ(TEST_VALUE.c_str(),msg.array().at(0).c_str());
}
