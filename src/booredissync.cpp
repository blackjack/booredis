#include "booredissync.h"

BooRedisSync::BooRedisSync():
    m_connected(false),
    m_socket(new boost::asio::ip::tcp::socket(m_ioService)),
    m_bytesToRead(1),
    m_messagesToRead(0),
    m_readState(ReadUntilBytes),
    m_analyzeState(GetType)
{
    m_bufferMessage.m_type = RedisMessage::Type_Unknown;
}

BooRedisSync::~BooRedisSync()
{
    disconnect();
}

bool BooRedisSync::connect(const char *address, int port)
{
    boost::asio::ip::tcp::resolver resolver(m_ioService);
    char aport[8];
    sprintf(aport,"%d",port); //resolver::query accepts string as second parameter

    boost::asio::ip::tcp::resolver::query query(address, aport);
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

    boost::asio::ip::tcp::endpoint endpoint = *iterator;
    boost::system::error_code error;
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
    while (it!=boost::asio::ip::tcp::resolver::iterator()) {
        error = m_socket->connect(endpoint,error);
        if (!error) {
            m_connected = true;
            return true;
        }
        ++it;
    }
    onError(error);
    return false;
}

bool BooRedisSync::connected() {
    return m_connected;
}

RedisMessage BooRedisSync::command(const std::vector<std::string> &command_and_arguments)
{
    std::stringstream cmd;
    cmd << "*" << command_and_arguments.size() << "\r\n";

    for (std::vector<std::string>::const_iterator it = command_and_arguments.begin(); it!=command_and_arguments.end(); ++it) {
        const std::string& line = *it;
        cmd << "$" << line.size() << "\r\n" << line << "\r\n";
    }
    return write(cmd.str());
}

void BooRedisSync::disconnect() {
    if (m_socket->is_open())
        m_socket->close();
    reset();
}

RedisMessage BooRedisSync::write(const std::string &msg) {
    boost::system::error_code error;
    boost::asio::write(*m_socket,boost::asio::buffer(msg), boost::asio::transfer_all(), error);
    if (error) { onError(error); return RedisMessage(); }
    size_t size = m_socket->read_some(boost::asio::buffer(m_readBuffer, maxReadLength),error);
    if (error) { onError(error); return RedisMessage(); }
    while (!processRawBuffer(size)) {
        size = m_socket->read_some(boost::asio::buffer(m_readBuffer, maxReadLength),error);
        if (error) { onError(error); return RedisMessage(); }
    }
    RedisMessage result = m_bufferMessage;
    reset();
    return result;
}

bool BooRedisSync::processRawBuffer(size_t bytesTransferred)
{
    unsigned int carret = 0;
    while ( carret < bytesTransferred ) {
        if ( m_readState == ReadUntilBytes ) {
            if ( bytesTransferred >= carret+m_bytesToRead) {
                m_redisMsgBuf.append(&m_readBuffer[carret],m_bytesToRead);
                carret+=m_bytesToRead;
                m_bytesToRead = 0;
                if (processMsgBuffer()) return true;
            } else {
                m_redisMsgBuf.append(&m_readBuffer[carret],bytesTransferred-carret);
                m_bytesToRead-=bytesTransferred-carret;
                carret=bytesTransferred;
            }
        } else if ( m_readState == ReadUntilNewLine ) {
            bool found = false;
            size_t newLinePos = 0; //new line char position from carret, e.g. \n is on carret+newLinePos position
            char* newLine;
            //own implementation of strchr with size
            for (newLine = &m_readBuffer[carret]; newLine < &m_readBuffer[bytesTransferred]; ++newLine) {
                if (*newLine == '\n') {
                    found = true;
                    newLinePos = (newLine-&m_readBuffer[carret]);
                    break;
                }
            }

            if (found && carret+newLinePos<bytesTransferred) {
                m_redisMsgBuf.append(&m_readBuffer[carret],newLinePos+1);
                carret+=newLinePos+1;
                if (processMsgBuffer()) return true;
            } else {
                m_redisMsgBuf.append(&m_readBuffer[carret],bytesTransferred-carret);
                m_bytesToRead-=bytesTransferred-carret;
                carret=bytesTransferred;
            }
        }
    }
    return false;
}


bool BooRedisSync::processMsgBuffer() {
    switch (m_analyzeState) {
    case GetType: {
        switch (m_redisMsgBuf.at(0)) {
        case '+': {
            m_readState = ReadUntilNewLine;
            m_analyzeState = GetData;
            m_bufferMessage.m_data.resize(1);
            m_messagesToRead = 1;
            m_bufferMessage.m_type = RedisMessage::Type_String;
            break;
        }
        case '-': {
            m_readState = ReadUntilNewLine;
            m_analyzeState = GetData;
            m_bufferMessage.m_data.resize(1);
            m_messagesToRead = 1;
            m_bufferMessage.m_type = RedisMessage::Type_Error;
            break;
        }
        case ':': {
            m_readState = ReadUntilNewLine;
            m_analyzeState = GetData;
            if (m_bufferMessage.m_type == RedisMessage::Type_Unknown) {
                m_bufferMessage.m_data.resize(1);
                m_messagesToRead = 1;
                m_bufferMessage.m_type = RedisMessage::Type_Integer;
            }
            break;
        }
        case '$': {
            m_readState = ReadUntilNewLine;
            m_analyzeState = GetLength;
            if (m_bufferMessage.m_type == RedisMessage::Type_Unknown) {//if not array
                m_bufferMessage.m_data.resize(1);
                m_messagesToRead = 1;
                m_bufferMessage.m_type = RedisMessage::Type_String;
            }
            break;
        }
        case '*': {
            m_readState = ReadUntilNewLine;
            m_analyzeState = GetCount;
            m_bufferMessage.m_type = RedisMessage::Type_Array;
            break;
        }
        default: {
            m_bufferMessage.m_data.clear();
            m_bufferMessage.m_type = RedisMessage::Type_Unknown;
            m_messagesToRead = 0;
            m_bytesToRead = 1;
            m_readState = ReadUntilBytes;
            m_analyzeState = GetType;
            onError(boost::system::error_code());
        }
        }
        break;
    }
    case GetCount: {
        m_messagesToRead = strtol(m_redisMsgBuf.c_str(),NULL,10);
        if (m_messagesToRead==0)
            return true;
        m_bufferMessage.m_data.resize(m_messagesToRead);
        m_bytesToRead = 1;
        m_readState = ReadUntilBytes;
        m_analyzeState = GetType;
        break;
    }
    case GetLength: {
        m_bytesToRead = strtol(m_redisMsgBuf.c_str(),NULL,10)+2; //with trailing \r\n
        if (m_bytesToRead == 1) { //was -1, e.g. empty message
            return true;
        } else {
            m_readState = ReadUntilBytes;
            m_analyzeState = GetData;
            break;
        }
    }
    case GetData: {
        m_redisMsgBuf.erase(m_redisMsgBuf.size()-2,2); //remove trailing \r\n
        m_bufferMessage.m_data[m_bufferMessage.m_data.size()-m_messagesToRead] = m_redisMsgBuf;
        if (--m_messagesToRead <= 0) {
            return true;
        } else {
            m_bytesToRead = 1;
            m_readState = ReadUntilBytes;
            m_analyzeState = GetType;
        }
        break;
    }
    }
    m_redisMsgBuf.clear();
    return false;
}

void BooRedisSync::onError(const boost::system::error_code &error)
{
    if (error == boost::asio::error::operation_aborted)
        return;

    m_connected = false;
    m_lastError = error.message();

    closeSocket(); //close socket and cleanup
    reset();
}

void BooRedisSync::closeSocket()
{
    m_socket->close();
    m_socket.reset(new boost::asio::ip::tcp::socket(m_ioService));
}

void BooRedisSync::reset()
{
    m_bytesToRead = 1;
    m_messagesToRead = 0;
    m_readState = ReadUntilBytes;
    m_analyzeState = GetType;
    m_redisMsgBuf.clear();
    m_bufferMessage.m_data.clear();
    m_bufferMessage.m_type = RedisMessage::Type_Unknown;
}

