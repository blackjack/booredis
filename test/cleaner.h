#ifndef CLEANER_H
#define CLEANER_H

#include <booredissync.h>

struct Cleaner: BooRedisSync {
    Cleaner() {
        connect("localhost",6379);
    }
    Cleaner& operator << (const std::string& key) {
        std::vector<std::string> cmd;
        cmd.push_back("DEL");
        cmd.push_back(key);
        command(cmd);
        return *this;
    }
};


#endif // CLEANER_H
