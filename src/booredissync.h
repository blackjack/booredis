#ifndef BOOREDISSYNC_H
#define BOOREDISSYNC_H

#include "booredisdecoder.h"

#include <vector>
#include <string>
#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>

class BooRedisSync
{
public:
    BooRedisSync();
    BooRedisSync(boost::asio::io_service& io_service);
    virtual ~BooRedisSync();

    bool connect(const char* address, int port);
    void disconnect();
    bool connected();

    std::vector<RedisMessage> command(const std::vector<std::string> &command_and_arguments); //for binary-safe multiline commands
    std::vector<RedisMessage> command(const std::string &command) { return write(command);} //for raw commands

    const std::string& lastError() { return m_lastError; }
protected:
    std::vector<RedisMessage> write(const std::string& msg);

    void closeSocket();
private:
    void onError(const boost::system::error_code& error);

private:
    bool m_connected;
    bool m_ownIoService;
    boost::asio::io_service *m_ioService;
    boost::scoped_ptr<boost::asio::ip::tcp::socket> m_socket;

    static const int maxReadLength = 1023; // maximum amount of data to read in one operation
    char m_readBuffer[maxReadLength]; //raw data from the socket

    std::string m_lastError;

    BooRedisDecoder m_decoder;
};

#endif // BOOREDISSYNC
