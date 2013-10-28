#include "booredissync.h"

BooRedisSync::BooRedisSync():
    m_connected(false),
    m_ownIoService(true),
    m_ioService(new boost::asio::io_service()),
    m_socket(new boost::asio::ip::tcp::socket(*m_ioService))
{
}

BooRedisSync::BooRedisSync(boost::asio::io_service &io_service):
    m_connected(false),
    m_ownIoService(false),
    m_ioService(&io_service),
    m_socket(new boost::asio::ip::tcp::socket(*m_ioService))
{
}

BooRedisSync::~BooRedisSync()
{
    disconnect();
    if (m_ownIoService)
        delete m_ioService;
}

bool BooRedisSync::connect(const char *address, int port)
{
    boost::asio::ip::tcp::resolver resolver(*m_ioService);
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

std::vector<RedisMessage> BooRedisSync::command(const std::vector<std::string> &command_and_arguments)
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
}

std::vector<RedisMessage> BooRedisSync::write(const std::string &msg) {
    boost::system::error_code error;
    std::vector<RedisMessage> result;

    boost::asio::write(*m_socket,boost::asio::buffer(msg),boost::asio::transfer_all(),error);
    if (error) {
        onError(error);
        return result;
    }

    m_decoder.reset();
    size_t size;
    BooRedisDecoder::DecodeResult decode_result;
    do {
        size = m_socket->read_some(boost::asio::buffer(m_readBuffer, maxReadLength),error);
        if (error) {
            onError(error);
            return result;
        }
        decode_result = m_decoder.decode(m_readBuffer,size,result);
    } while (decode_result == BooRedisDecoder::DecodeNeedMoreData);

    if (decode_result == BooRedisDecoder::DecodeError) {
        onError(boost::system::error_code());
        m_lastError = "Could not decode redis stream";
    }

    return result;
}

void BooRedisSync::onError(const boost::system::error_code &error)
{
    if (error == boost::asio::error::operation_aborted)
        return;

    m_connected = false;
    m_lastError = error.message();

    closeSocket(); //close socket and cleanup
}

void BooRedisSync::closeSocket()
{
    if (m_socket->is_open())
        m_socket->close();
    m_socket.reset(new boost::asio::ip::tcp::socket(*m_ioService));
}

