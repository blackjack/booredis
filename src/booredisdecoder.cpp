#include "booredisdecoder.h"

namespace {
//own implementation of strchr with size
inline const char* find_newline(const char* begin, const char* end, char c = '\n') {
    for (const char* newLine = begin; newLine < end; ++newLine) {
        if (*newLine == c)
            return newLine;
    }
    return NULL;
}

}

BooRedisDecoder::BooRedisDecoder():
    m_bytesToRead(1),
    m_messagesToRead(0),
    m_readState(ReadUntilBytes),
    m_analyzeState(GetType)
{
    m_message.m_type = RedisMessage::Type_Unknown;
}

BooRedisDecoder::DecodeResult BooRedisDecoder::decode(char* data, size_t len, std::vector<RedisMessage>& result)
{
    size_t carret = 0;
    while ( carret < len ) {
        size_t current_len = len-carret;

        if (m_readState==ReadUntilBytes) {
            if ( current_len >= m_bytesToRead) {
                m_redisMsgBuf.append(&data[carret],m_bytesToRead);
                carret+=m_bytesToRead;
                m_bytesToRead = 0;
            } else {
                m_redisMsgBuf.append(&data[carret],current_len);
                m_bytesToRead-=current_len;
                return DecodeNeedMoreData;
            }

        } else { //m_readState == ReadUntilNewLine
            const char* newLine = ::find_newline(&data[carret],&data[len]);

            if (newLine) {
                size_t newLinePos = newLine-&data[carret];
                m_redisMsgBuf.append(&data[carret],newLinePos+1);
                carret+=newLinePos+1;
            } else {
                m_redisMsgBuf.append(&data[carret],current_len);
                return DecodeNeedMoreData;
            }

        }

        DecodeResult ret = processMsgBuffer();
        switch (ret) {
        case DecodeFinished:
            result.push_back(m_message);
            reset();
            break;
        case DecodeError:
            reset();
            return ret;
        }
    }
}

void BooRedisDecoder::reset()
{
    m_bytesToRead = 1;
    m_messagesToRead = 0;
    m_readState = ReadUntilBytes;
    m_analyzeState = GetType;
    m_redisMsgBuf.clear();
    m_message.m_data.clear();
    m_message.m_type = RedisMessage::Type_Unknown;
}



BooRedisDecoder::DecodeResult BooRedisDecoder::processMsgBuffer() {
    DecodeResult result;
    switch (m_analyzeState) {
    case GetType:
        result = processType();
        break;
    case GetCount:
        result = processCount();
        break;
    case GetLength:
        result = processLength();
        break;
    case GetData:
        result = processData();
        break;
    }
    m_redisMsgBuf.clear();
    return result;
}

BooRedisDecoder::DecodeResult BooRedisDecoder::processType()
{
    switch (m_redisMsgBuf.at(0)) {
    case '+': {
        m_readState = ReadUntilNewLine;
        m_analyzeState = GetData;
        m_message.m_data.resize(1);
        m_messagesToRead = 1;
        m_message.m_type = RedisMessage::Type_String;
        break;
    }
    case '-': {
        m_readState = ReadUntilNewLine;
        m_analyzeState = GetData;
        m_message.m_data.resize(1);
        m_messagesToRead = 1;
        m_message.m_type = RedisMessage::Type_Error;
        break;
    }
    case ':': {
        m_readState = ReadUntilNewLine;
        m_analyzeState = GetData;
        if (m_message.m_type == RedisMessage::Type_Unknown) {
            m_message.m_data.resize(1);
            m_messagesToRead = 1;
            m_message.m_type = RedisMessage::Type_Integer;
        }
        break;
    }
    case '$': {
        m_readState = ReadUntilNewLine;
        m_analyzeState = GetLength;
        if (m_message.m_type == RedisMessage::Type_Unknown) {//if not array
            m_message.m_data.resize(1);
            m_messagesToRead = 1;
            m_message.m_type = RedisMessage::Type_String;
        }
        break;
    }
    case '*': {
        m_readState = ReadUntilNewLine;
        m_analyzeState = GetCount;
        m_message.m_type = RedisMessage::Type_Array;
        break;
    }
    default: {
        return DecodeError;
    }
    }
    return DecodeNeedMoreData;
}

BooRedisDecoder::DecodeResult BooRedisDecoder::processCount()
{
    m_messagesToRead = strtol(m_redisMsgBuf.c_str(),NULL,10);
    if (m_messagesToRead==0)
        return DecodeFinished;

    m_message.m_data.resize(m_messagesToRead);
    m_bytesToRead = 1;
    m_readState = ReadUntilBytes;
    m_analyzeState = GetType;
    return DecodeNeedMoreData;
}

BooRedisDecoder::DecodeResult BooRedisDecoder::processLength()
{
    m_bytesToRead = strtol(m_redisMsgBuf.c_str(),NULL,10)+2; //with trailing \r\n
    if (m_bytesToRead == 1) {
        if (--m_messagesToRead > 0) {
            m_analyzeState = GetType;
            return DecodeNeedMoreData;
        }
        else
            return DecodeFinished;
    }

    m_readState = ReadUntilBytes;
    m_analyzeState = GetData;
    return DecodeNeedMoreData;
}

BooRedisDecoder::DecodeResult BooRedisDecoder::processData()
{
    if (m_redisMsgBuf.size()<2) return DecodeError;

    m_redisMsgBuf.erase(m_redisMsgBuf.size()-2,2); //remove trailing \r\n
    m_message.m_data[m_message.m_data.size()-m_messagesToRead] = m_redisMsgBuf;

    if (--m_messagesToRead > 0) {
        m_bytesToRead = 1;
        m_readState = ReadUntilBytes;
        m_analyzeState = GetType;
        return DecodeNeedMoreData;
    } else
        return DecodeFinished;
}


