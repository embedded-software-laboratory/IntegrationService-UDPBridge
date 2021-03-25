#ifndef _UDP_READER_HPP
#define _UDP_READER_HPP

#include "UDPBridge.hpp"

#include "ISReader.h"

#include <boost/asio.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <thread>
#include <cstdlib>
#include <sstream>


constexpr size_t _UDPREADER_RECEIVE_BUFFER_SIZE_DEFAULT = 65536;

class UDPReader : public ISReader {
   private:
      ssize_t m_socketReceiveBufferSize = _UDPREADER_RECEIVE_BUFFER_SIZE_DEFAULT;

      UDPBridge* m_bridgePtr = nullptr;

   public:
   UDPReader(ISBridge* p_bridge, const std::string &name, const std::vector<std::pair<std::string, std::string>> *config) : ISReader(name) {
      for(auto const& pair : *config) {
         if(pair.first.compare("BUFFER_SIZE") == 0) {
            m_socketReceiveBufferSize = std::stoi(pair.second);

            log("Reader: Buffersize set to", m_socketReceiveBufferSize);
         }
      }

      m_bridgePtr = dynamic_cast<UDPBridge*>(p_bridge);

      std::thread(&UDPReader::receiveLoop, this).detach();
   }

   ~UDPReader() {};

   void receiveLoop() {
      log("UDPREADER::receiveLoop");

      std::vector<unsigned char> receiveBuffer;
      receiveBuffer.resize(m_socketReceiveBufferSize + 1);

      while(1) {
         ssize_t recvSize = m_bridgePtr->receive(receiveBuffer);
         if(recvSize > 0) {

            log("UDPREADER::receiveLoop [recvSize: ", recvSize,"]");

            if (recvSize > m_socketReceiveBufferSize) {
               logError("Packet too large. [UDPREADER_RECEIVE_BUFFER_SIZE]");
               continue;
            }

            SerializedPayload_t *payload = new SerializedPayload_t(recvSize);
            payload->data = reinterpret_cast<eprosima::fastrtps::octet*>(std::malloc(recvSize));
            payload->length = recvSize;

            for(ssize_t i = 0; i < recvSize; ++i) {
               payload->data[i] = receiveBuffer[i];
            }

            // on received data frees the payload struct
            on_received_data(payload);
         }
         usleep(200);
      }
   }
};

#endif // !_UDP_READER_HPP
