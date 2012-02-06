#include "booredisasync.h"

BooRedisAsync::BooRedisAsync():
    m_onceConnected(false),
    m_connected(false),
    m_writeInProgress(false),
    m_io_service(),
    m_socket(new boost::asio::ip::tcp::socket(m_io_service)),
    m_connectTimer(m_io_service),
    //initial state - wait for type char
    m_bytesToRead(1),
    m_messagesToRead(0),
    m_readState(ReadUntilBytes),
    m_analyzeState(GetType)
{
    m_bufferMessage.m_type = RedisMessage::Type_Unknown;
}

void BooRedisAsync::connect(const char *address, int port, int timeout_msec)
{
    boost::asio::ip::tcp::resolver resolver(m_io_service);
    char aport[8];
    sprintf(aport,"%d",port); //resolver::query accepts string as second parameter

    boost::asio::ip::tcp::resolver::query query(address, aport);
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

    onLogMessage(std::string("Connecting to Redis ") + address + ":" + aport);

    m_connectionTimeout = boost::posix_time::milliseconds(timeout_msec);

    connectStart(iterator);

    if (boost::this_thread::get_id() != m_thread.get_id())
        m_thread = boost::thread(boost::bind(&boost::asio::io_service::run, &m_io_service));
}


void BooRedisAsync::command(const std::vector<std::string> &command_and_arguments)
{
    std::stringstream cmd;
    cmd << "*" << command_and_arguments.size() << "\r\n";

    for (std::vector<std::string>::const_iterator it = command_and_arguments.begin(); it!=command_and_arguments.end(); ++it) {
        const std::string& line = *it;
        cmd << "$" << line.size() << "\r\n" << line << "\r\n";
    }

    write(cmd.str());

}

void BooRedisAsync::close() {
    m_socket->close();
    if (boost::this_thread::get_id() != m_thread.get_id())
        m_thread.join();
}

bool BooRedisAsync::connected() {
    return m_connected;
}

void BooRedisAsync::connectStart(boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
    m_endpointIterator = endpoint_iterator;
    boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
    m_socket->async_connect(endpoint,boost::bind(&BooRedisAsync::connectComplete,this,boost::asio::placeholders::error));

    m_connectTimer.expires_from_now(m_connectionTimeout);
    m_connectTimer.async_wait(boost::bind(&BooRedisAsync::onError, this, boost::asio::placeholders::error));
}

void BooRedisAsync::connectComplete(const boost::system::error_code& error)
{
    if (!error)
    {
        m_connectTimer.cancel();
        readStart();
        m_onceConnected = true;
        m_connected = true;

        onConnect();

        if (!m_writeInProgress && !m_writeBuffer.empty())
            writeStart();
    }
    else
        onError(error);
}

void BooRedisAsync::write(const std::string &msg) {
    m_io_service.post(boost::bind(&BooRedisAsync::doWrite, this, msg));
}


void BooRedisAsync::doWrite(const std::string &msg)
{
    m_writeBuffer.push_back(msg);
    if (connected() && !m_writeInProgress)
        writeStart();
}

void BooRedisAsync::writeStart()
{
    m_writeInProgress = true;
    const std::string& msg = m_writeBuffer.front();
    boost::asio::async_write(*m_socket,
                             boost::asio::buffer(msg),
                             boost::bind(&BooRedisAsync::writeComplete,
                                         this,
                                         boost::asio::placeholders::error));
}

void BooRedisAsync::writeComplete(const boost::system::error_code &error)
{
    if (!error)
    {
        m_writeBuffer.pop_front();
        if (!m_writeBuffer.empty())
            writeStart();
        else
            m_writeInProgress = false;
    }
    else
        onError(error);
}

void BooRedisAsync::readStart()
{
    m_socket->async_read_some(boost::asio::buffer(m_readBuffer, maxReadLength),
                             boost::bind(&BooRedisAsync::readComplete,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void BooRedisAsync::readComplete(const boost::system::error_code &error, size_t bytesTransferred)
{
    if (!error)
    {
        processRawBuffer(bytesTransferred);
        readStart();
    }
    else
        onError(error);
}

void BooRedisAsync::processRawBuffer(size_t bytesTransferred)
{
    unsigned int carret = 0;
    while ( carret < bytesTransferred ) {
        if ( m_readState == ReadUntilBytes ) {
            if ( bytesTransferred >= carret+m_bytesToRead) {
                m_redisMsgBuf.append(&m_readBuffer[carret],m_bytesToRead);
                carret+=m_bytesToRead;
                m_bytesToRead = 0;
                processMsgBuffer();
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
                processMsgBuffer();
            } else {
                m_redisMsgBuf.append(&m_readBuffer[carret],bytesTransferred-carret);
                m_bytesToRead-=bytesTransferred-carret;
                carret=bytesTransferred;
            }
        }
    }
}


void BooRedisAsync::processMsgBuffer() {
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
            onLogMessage("Error processing Redis answer, reconnecting",LOG_LEVEL_ERR);
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
        m_bufferMessage.m_data.resize(m_messagesToRead);
        m_bytesToRead = 1;
        m_readState = ReadUntilBytes;
        m_analyzeState = GetType;
        break;
    }
    case GetLength: {
        m_bytesToRead = strtol(m_redisMsgBuf.c_str(),NULL,10)+2; //with trailing \r\n
        if (m_bytesToRead == -1) {
            onRedisMessage(m_bufferMessage);
            reset();
            return;
        }
        m_readState = ReadUntilBytes;
        m_analyzeState = GetData;
        break;
    }
    case GetData: {
        m_redisMsgBuf.erase(m_redisMsgBuf.size()-2,2); //remove trailing \r\n
        m_bufferMessage.m_data[m_bufferMessage.m_data.size()-m_messagesToRead] = m_redisMsgBuf;
        if (--m_messagesToRead <= 0) {
            onRedisMessage(m_bufferMessage);
            reset();
            return;
        } else {
            m_bytesToRead = 1;
            m_readState = ReadUntilBytes;
            m_analyzeState = GetType;
        }
        break;
    }
    }
    m_redisMsgBuf.clear();
}

void BooRedisAsync::onError(const boost::system::error_code &error)
{
    if (error == boost::asio::error::operation_aborted)
        return;

    if (error)
        onLogMessage("Connection to Redis " + endpointToString(m_endpointIterator) + " failed: "
                     + error.message(),LOG_LEVEL_ERR);

    m_writeInProgress = false;
    m_connected = false;
    m_connectTimer.cancel();

    closeSocket(); //close socket and cleanup
    reset();

    onDisconnect();
}

boost::asio::ip::tcp::resolver::iterator BooRedisAsync::getEndpointIterator()
{
    return m_endpointIterator;
}

void BooRedisAsync::setEndpointIterator(boost::asio::ip::tcp::resolver::iterator iterator)
{
    m_endpointIterator = iterator;
}

bool BooRedisAsync::onceConnected()
{
    return m_onceConnected;
}

bool BooRedisAsync::isLastEndpoint(boost::asio::ip::tcp::resolver::iterator iterator)
{
    return (++iterator == boost::asio::ip::tcp::resolver::iterator());
}

std::string BooRedisAsync::endpointToString(boost::asio::ip::tcp::resolver::iterator iterator)
{
    boost::asio::ip::tcp::endpoint endpoint = *iterator;
    std::stringstream s;
    s << endpoint.address().to_string() << ":" << endpoint.port();
    return s.str();
}

void BooRedisAsync::connect(boost::asio::ip::tcp::resolver::iterator iterator)
{
    connectStart(iterator);
}

void BooRedisAsync::closeSocket()
{
    m_socket->close();
    m_socket.reset(new boost::asio::ip::tcp::socket(m_io_service));
}

void BooRedisAsync::reset()
{
    m_bytesToRead = 1;
    m_messagesToRead = 0;
    m_readState = ReadUntilBytes;
    m_analyzeState = GetType;
    m_redisMsgBuf.clear();
    m_bufferMessage.m_data.clear();
    m_bufferMessage.m_type = RedisMessage::Type_Unknown;
}

RedisMessage::MessageType RedisMessage::type() const
{
    return m_type;
}

int RedisMessage::integer() const
{
    return atoi(m_data.at(0).c_str());
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

