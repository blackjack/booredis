#ifndef BOOREDISASYNC_H
#define BOOREDISASYNC_H

#include "booredisdecoder.h"

#include <deque>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/scoped_ptr.hpp>


class BooRedisAsync
{
public:
    BooRedisAsync();
    BooRedisAsync(boost::asio::io_service& io_service);
    virtual ~BooRedisAsync();
    void connect(const char* address, int port, int timeout_msec);
    void disconnect();
    bool connected();

    void command(const std::vector<std::string> &command_and_arguments); //for binary-safe multiline commands
    void command(const std::string &command) {write(command);} //for raw commands

    virtual void onRedisMessage(const RedisMessage& msg) {} //implement this to get redis messages
    virtual void onRedisMessage(const RedisMessage& msg, int port) {} //implement this to get redis messages and port number
    virtual void onLogMessage(const std::string& msg, int logLevel = LOG_LEVEL_INFO) {} //not implemented in base class
    virtual void onDisconnect() {}
    virtual void onConnect() {}
    virtual void onDisconnect(int port) {}
    virtual void onConnect(int port) {}

public:
    static const int LOG_LEVEL_EMERG = 0;
    static const int LOG_LEVEL_ALERT = 1;
    static const int LOG_LEVEL_CRIT = 2;
    static const int LOG_LEVEL_ERR = 3;
    static const int LOG_LEVEL_WARNING = 4;
    static const int LOG_LEVEL_NOTICE = 5;
    static const int LOG_LEVEL_INFO = 6;
    static const int LOG_LEVEL_DEBUG = 7;

protected:
    void write(const std::string& msg);
    boost::asio::io_service& io_service() { return *m_ioService; }

    boost::asio::ip::tcp::resolver::iterator getEndpointIterator();
    void setEndpointIterator(boost::asio::ip::tcp::resolver::iterator iterator);
    bool onceConnected(); //if any endpoint was valid
    static bool isLastEndpoint(boost::asio::ip::tcp::resolver::iterator iterator);
    static std::string endpointToString(boost::asio::ip::tcp::resolver::iterator iterator);

    void connect(boost::asio::ip::tcp::resolver::iterator iterator);

    void closeSocket();

    std::deque<std::string> m_writeBuffer; // buffered write data

private:
    void connectStart(boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void connectComplete(const boost::system::error_code& error);
    void readStart();
    void readComplete(const boost::system::error_code& error, size_t bytesTransferred);
    void doWrite(const std::string& msg);
    void writeStart();
    void writeComplete(const boost::system::error_code& error);
    void onError(const boost::system::error_code& error);

private:
    bool m_onceConnected; //if any endpoint was valid
    bool m_connected; //if connected
    bool m_writeInProgress; //if write is in progress
    bool m_port; //store current port number
    boost::asio::ip::tcp::resolver::iterator m_endpointIterator;

    static const int maxReadLength = 1023; // maximum amount of data to read in one operation
    char m_readBuffer[maxReadLength]; //raw data from the socket

    bool m_ownIoService;
    boost::asio::io_service* m_ioService;
    boost::scoped_ptr<boost::asio::ip::tcp::socket> m_socket;
    boost::scoped_ptr<boost::asio::deadline_timer> m_connectTimer;
    boost::posix_time::time_duration m_connectionTimeout;

    boost::thread m_thread; //io_service thread

    BooRedisDecoder m_decoder;
};

#endif // BOOREDISASYNC_H
