#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <gtest/gtest.h>

#include <booredis/async.h>

#include "../cleaner.h"
#include "asynctestobject.h"

TEST(BooRedisAsync, Error)
{
    AsyncTestObject redis;
    redis.connect("127.0.0.1",6379,1000);

    std::vector<std::string> cmd;
    cmd.push_back("ERROR_COMMAND");
    boost::unique_future<RedisMessage> future = redis.command(cmd);

    ASSERT_TRUE(future.timed_wait(boost::posix_time::seconds(1)));

    RedisMessage msg = future.get();
    EXPECT_EQ(RedisMessage::Type_Error,msg.type());
    ASSERT_EQ(1,msg.array().size());
    EXPECT_STREQ("ERR unknown command 'ERROR_COMMAND'",msg.string().c_str());
}

