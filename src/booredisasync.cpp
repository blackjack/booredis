#include "booredisasync.h"

BooRedisAsync::BooRedisAsync():
    m_onceConnected(false),
    m_connected(false),
    m_writeInProgress(false),
    m_ownIoService(true),
    m_ioService(new boost::asio::io_service),
    m_socket(new boost::asio::ip::tcp::socket(*m_ioService)),
    m_connectTimer(new boost::asio::deadline_timer(*m_ioService))
{
}

BooRedisAsync::BooRedisAsync(boost::asio::io_service &io_service):
    m_onceConnected(false),
    m_connected(false),
    m_writeInProgress(false),
    m_ownIoService(false),
    m_ioService(&io_service),
    m_socket(new boost::asio::ip::tcp::socket(*m_ioService)),
    m_connectTimer(new boost::asio::deadline_timer(*m_ioService))
{
}

BooRedisAsync::~BooRedisAsync()
{
    m_connected = false;
    if (m_ownIoService) {
        m_ioService->stop();
        m_thread.join();
        m_connectTimer.reset();
        m_socket.reset();
        delete m_ioService;
    }
}

void BooRedisAsync::connect(const char *address, int port, int timeout_msec)
{
    if (m_connected)
        disconnect();

    boost::asio::ip::tcp::resolver resolver(*m_ioService);
    char aport[8];
    sprintf(aport,"%d",port); //resolver::query accepts string as second parameter

    boost::asio::ip::tcp::resolver::query query(address, aport);
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

    onLogMessage(std::string("Connecting to Redis ") + address + ":" + aport);

    m_connectionTimeout = boost::posix_time::milliseconds(timeout_msec);

    connectStart(iterator);

    if (m_ownIoService && boost::this_thread::get_id() != m_thread.get_id())
        m_thread = boost::thread(boost::bind(&boost::asio::io_service::run, m_ioService));
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

void BooRedisAsync::disconnect() {
    m_connected = false;
    if (m_socket->is_open())
        m_socket->close();
    if (m_ownIoService) {
        m_ioService->stop();
        m_thread.join();
        delete m_ioService;
        m_ioService = new boost::asio::io_service();
    }
    m_socket.reset(new boost::asio::ip::tcp::socket(*m_ioService));
    m_connectTimer.reset(new boost::asio::deadline_timer(*m_ioService));
}

bool BooRedisAsync::connected() {
    return m_connected;
}

void BooRedisAsync::connectStart(boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
    m_endpointIterator = endpoint_iterator;
    boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
    m_socket->async_connect(endpoint,boost::bind(&BooRedisAsync::connectComplete,this,boost::asio::placeholders::error));

    m_connectTimer->expires_from_now(m_connectionTimeout);
    m_connectTimer->async_wait(boost::bind(&BooRedisAsync::onError, this, boost::asio::placeholders::error));
}

void BooRedisAsync::connectComplete(const boost::system::error_code& error)
{
    if (!error)
    {
        m_connectTimer->cancel();
        readStart();
        m_onceConnected = true;
        m_connected = true;

        onLogMessage("Successfully connected to Redis " + endpointToString(getEndpointIterator()),LOG_LEVEL_INFO);
        onConnect();

        if (!m_writeInProgress && !m_writeBuffer.empty())
            writeStart();
    }
    else
        onError(error);
}

void BooRedisAsync::write(const std::string &msg) {
    m_ioService->post(boost::bind(&BooRedisAsync::doWrite, this, msg));
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
        std::vector<RedisMessage> result;
        BooRedisDecoder::DecodeResult res = m_decoder.decode(m_readBuffer,bytesTransferred,result);

        for (std::vector<RedisMessage>::iterator it=result.begin();it!=result.end();++it)
            onRedisMessage(*it);

        if (res != BooRedisDecoder::DecodeError)
            readStart();
        else {
            onLogMessage("Error decoding redis message. Reconnecting",LOG_LEVEL_ERR);
            onError(boost::system::error_code());
        }
    }
    else
        onError(error);
}


void BooRedisAsync::onError(const boost::system::error_code &error)
{
    if (error == boost::asio::error::operation_aborted)
        return;

    if (error!=boost::system::error_code()) //If not decode error
        onLogMessage("Connection to Redis " + endpointToString(m_endpointIterator) + " failed: "
                     + error.message(),LOG_LEVEL_ERR);

    m_writeInProgress = false;
    m_connected = false;
    m_connectTimer->cancel();

    try {
        closeSocket(); //close socket and cleanup
    } catch (...) {}

    m_decoder.reset();

    onDisconnect();

    boost::asio::ip::tcp::resolver::iterator it = getEndpointIterator();
    if (!onceConnected() && !isLastEndpoint(it)) { //try another address
        setEndpointIterator(++it); //switch to the next endpoint
        onLogMessage("Trying next Redis address: " + endpointToString(it),LOG_LEVEL_DEBUG);
    } else
        sleep(1);

    onLogMessage("Reconnecting to Redis " + endpointToString(it),LOG_LEVEL_INFO);

    connect(it);
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
    if (m_socket->is_open())
        m_socket->close();
    m_socket.reset(new boost::asio::ip::tcp::socket(*m_ioService));
}

