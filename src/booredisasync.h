#ifndef REDISASYNCCLIENT_H
#define REDISASYNCCLIENT_H

#include <deque>
#include <vector>
#include <string>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

struct RedisMessage {
    enum MessageType { Type_Unknown, Type_String, Type_Integer, Type_Array, Type_Error };
    MessageType type() const;
    int integer() const;
    const std::string& string() const;
    const std::vector<std::string>& array() const;
    const std::string& error() const;
private:
    friend class BooRedisAsync;
    MessageType m_type;
    std::vector<std::string> m_data;
};


class BooRedisAsync
{
public:
    BooRedisAsync();
    void connect(const char* address, int port, int timeout_msec);
    virtual void close();
    bool connected();

    void command(const std::vector<std::string> &command_and_arguments); //for binary-safe multiline commands
    void command(const std::string &command) {write(command);} //for raw commands

    //main callback
    virtual void onRedisMessage(const RedisMessage& msg) {} //implement this to get redis messages
    virtual void onLogMessage(const std::string& msg, int logLevel = LOG_LEVEL_INFO) {} //not implemented in base class

    //session level callbacks

    //default behaviour - reconnect after 1 second OR
    //try next endpoint if not connected to any endpoint yet
    virtual void onDisconnect() {
        boost::asio::ip::tcp::resolver::iterator it = getEndpointIterator();
        if (!onceConnected() && !isLastEndpoint(it)) { //try another address
            setEndpointIterator(++it); //switch to the next endpoint
            onLogMessage("Trying next Redis address: " + endpointToString(it),LOG_LEVEL_DEBUG);
        } else
            sleep(1);

        onLogMessage("Reconnecting to Redis " + endpointToString(it),LOG_LEVEL_INFO);

        connect(it);
    }
    virtual void onConnect() {
        onLogMessage("Successfully connected to Redis " + endpointToString(getEndpointIterator()),LOG_LEVEL_INFO);
    }

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
    boost::asio::io_service& io_service() { return m_io_service; } 

    boost::asio::ip::tcp::resolver::iterator getEndpointIterator();
    void setEndpointIterator(boost::asio::ip::tcp::resolver::iterator iterator);
    bool onceConnected(); //if any endpoint was valid
    static bool isLastEndpoint(boost::asio::ip::tcp::resolver::iterator iterator);
    static std::string endpointToString(boost::asio::ip::tcp::resolver::iterator iterator);

    void connect(boost::asio::ip::tcp::resolver::iterator iterator);

    void closeSocket();
    void reset(); //resets all Redis protocol-related state variables

private:
    static const int maxReadLength = 1023; // maximum amount of data to read in one operation
    void connectStart(boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void connectComplete(const boost::system::error_code& error);
    void readStart();
    void readComplete(const boost::system::error_code& error, size_t bytesTransferred);
    void doWrite(const std::string& msg);
    void writeStart();
    void writeComplete(const boost::system::error_code& error);
    void onError(const boost::system::error_code& error);

    void processRawBuffer(size_t bytesTransferred);
    void processMsgBuffer();

private:
    bool m_onceConnected; //if any endpoint was valid
    bool m_connected; //if connected
    bool m_writeInProgress; //if write is in progress
    boost::asio::ip::tcp::resolver::iterator m_endpointIterator;

    boost::asio::io_service m_io_service;
    boost::scoped_ptr<boost::asio::ip::tcp::socket> m_socket;
    boost::asio::deadline_timer m_connectTimer;
    boost::posix_time::time_duration m_connectionTimeout;

    char m_readBuffer[maxReadLength]; //raw data from the socket
    std::string m_redisMsgBuf; //result data
    RedisMessage m_bufferMessage; //final Redis message

    int m_bytesToRead;
    int m_messagesToRead;


    enum ReadState { ReadUntilNewLine, ReadUntilBytes };
    enum AnalyzeState { GetType, GetCount, GetLength, GetData };
    ReadState m_readState;
    AnalyzeState m_analyzeState;

    std::deque<std::string> m_writeBuffer; // buffered write data
    boost::thread m_thread; //io_service thread
};

#endif // REDISASYNCCLIENT_H
