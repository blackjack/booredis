#ifndef ASYNCTESTOBJECT_H
#define ASYNCTESTOBJECT_H

#include <booredisasync.h>
#include <boost/thread/future.hpp>
#include <boost/shared_ptr.hpp>

struct AsyncTestObject: BooRedisAsync {
    boost::unique_future<RedisMessage> command(const std::vector<std::string> &command_and_arguments) {
        BooRedisAsync::command(command_and_arguments);
        boost::shared_ptr< boost::promise<RedisMessage> > promise (new boost::promise<RedisMessage>() );
        queue.push_back(promise);
        return queue.back()->get_future();
    }

    void onRedisMessage(const RedisMessage &msg) {
        queue.front()->set_value(msg);
        queue.pop_front();
    }

    std::deque< boost::shared_ptr<
                    boost::promise<RedisMessage>
                    >
              > queue;
};

#endif // ASYNCTESTOBJECT_H
