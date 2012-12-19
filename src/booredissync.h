#ifndef BOOREDISSYNC_H
#define BOOREDISSYNC_H

#include <deque>
#include <vector>
#include <string>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "redismessage.h"

class BooRedisSync
{
public:
    BooRedisSync();
    virtual ~BooRedisSync();

    bool connect(const char* address, int port);
    void disconnect();
    bool connected();

    RedisMessage command(const std::vector<std::string> &command_and_arguments); //for binary-safe multiline commands
    RedisMessage command(const std::string &command) { return write(command);} //for raw commands

    const std::string& lastError() { return m_lastError; }
protected:
    RedisMessage write(const std::string& msg);

    void closeSocket();
    void reset(); //resets all Redis protocol-related state variables

private:
    static const int maxReadLength = 1023; // maximum amount of data to read in one operation

    void onError(const boost::system::error_code& error);

    bool processRawBuffer(size_t bytesTransferred);
    bool processMsgBuffer();

private:
    bool m_connected;
    boost::asio::io_service m_ioService;
    boost::scoped_ptr<boost::asio::ip::tcp::socket> m_socket;

    char m_readBuffer[maxReadLength]; //raw data from the socket
    std::string m_redisMsgBuf; //result data
    RedisMessage m_bufferMessage; //final Redis message

    int m_bytesToRead;
    int m_messagesToRead;

    enum ReadState { ReadUntilNewLine, ReadUntilBytes };
    enum AnalyzeState { GetType, GetCount, GetLength, GetData };
    ReadState m_readState;
    AnalyzeState m_analyzeState;

    std::string m_lastError;
};

#endif // BOOREDISSYNC
