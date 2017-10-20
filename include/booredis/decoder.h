#ifndef BOOREDISDECODER_H
#define BOOREDISDECODER_H

#include <boost/function.hpp>
#include <boost/system/error_code.hpp>

#include "redismessage.h"

class BooRedisDecoder
{
public:
   BooRedisDecoder();

   char* buffer();
   size_t buffer_length() const;

   enum DecodeResult { DecodeFinished, DecodeNeedMoreData, DecodeError };
   DecodeResult decode(char* data, size_t len, std::vector<RedisMessage>& result);
   void reset();
private:
   std::string m_redisMsgBuf;
   RedisMessage m_message;

   int m_bytesToRead;
   int m_messagesToRead;

   enum ReadState { ReadUntilNewLine, ReadUntilBytes };
   enum AnalyzeState { GetType, GetCount, GetLength, GetData };
   ReadState m_readState;
   AnalyzeState m_analyzeState;


   inline BooRedisDecoder::DecodeResult processMsgBuffer();

   BooRedisDecoder::DecodeResult processType();
   BooRedisDecoder::DecodeResult processCount();
   BooRedisDecoder::DecodeResult processLength();
   BooRedisDecoder::DecodeResult processData();
};

#endif // BOOREDISDECODER_H
