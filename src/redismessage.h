#ifndef REDISMESSAGE_H
#define REDISMESSAGE_H

#include <string>
#include <vector>

class BooRedisSync;

struct RedisMessage {
    enum MessageType { Type_Unknown, Type_String, Type_Integer, Type_Array, Type_Error };
    MessageType type() const;
    int integer() const;
    bool empty();
    const std::string& string() const;
    const std::vector<std::string>& array() const;
    const std::string& error() const;
private:
    friend class BooRedisSync;
    friend class BooRedisAsync;
    MessageType m_type;
    std::vector<std::string> m_data;
};

#endif // REDISMESSAGE_H
