#include <cstdlib>

#include "booredis/redismessage.h"

RedisMessage::MessageType RedisMessage::type() const
{
    return m_type;
}

int RedisMessage::integer() const
{
    return atoi(m_data.at(0).c_str());
}

bool RedisMessage::empty() const
{
    return m_data.empty();
}

const std::string &RedisMessage::string() const
{
    return m_data.at(0);
}

const std::vector<std::string>& RedisMessage::array() const
{
    return m_data;
}

const std::string &RedisMessage::error() const
{
    return m_data.at(0);
}
