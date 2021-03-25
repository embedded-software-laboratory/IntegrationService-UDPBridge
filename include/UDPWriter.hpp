#ifndef _UDP_WRITER_HPP_
#define _UDP_WRITER_HPP_

#include "ISWriter.h"
#include "UDPBridge.hpp"

#include "logger.hpp"

#include <ctime>
#include <atomic>

#include <thread>
#include <unistd.h>

constexpr time_t _HEARTBEAT_SLEEP_TIMER_USEC = 10000;
constexpr time_t _HEARTBEAT_PERIOD_USEC = 1000000;

class UDPWriter : public ISWriter {
   private:
   UDPBridge* m_bridgePtr = nullptr;
   std::atomic<std::time_t> m_lastSendTime;

   std::time_t m_heartbeatTimer = _HEARTBEAT_PERIOD_USEC;

   public:
   UDPWriter(ISBridge* p_bridge, const std::string& name, const std::vector<std::pair<std::string, std::string>>* config) : ISWriter(name) {
      for(auto const& pair : *config) {
         if(pair.first.compare("HEARTBEAT_TIMER") == 0) {
            m_heartbeatTimer = static_cast<std::time_t>(std::stoi(pair.second));

            log("Set Heartbeat Timer to ", m_heartbeatTimer, " useconds.");
         }
      }


      m_bridgePtr = dynamic_cast<UDPBridge*>(p_bridge);

      m_lastSendTime = static_cast<std::time_t>(0);


      if(!m_bridgePtr->m_configServer && m_heartbeatTimer > 0) { // client heartbeat to keep connection
         std::thread(&UDPWriter::heartbeatExecutorLoop, this).detach();
      }
   }

   ~UDPWriter() {};

   bool write(SerializedPayload_t* p_payload) override {
      log("UDPWRITER::write");

      m_lastSendTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      return m_bridgePtr->sendToRemoteEndpoint(p_payload);
   }

   bool write(eprosima::fastrtps::types::DynamicData*) override { return false; }

   void heartbeatExecutorLoop() {
      log("UDPWRITER::heartbeatExecutorLoop");

      while(true) {
         std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

         // log("CURR - MLAST: ", currentTime - m_lastSendTime);

         if (currentTime - m_lastSendTime > m_heartbeatTimer){
            m_bridgePtr->sendHeartbeat();
            
            m_lastSendTime = currentTime;
         }

         usleep(_HEARTBEAT_SLEEP_TIMER_USEC);
      }
   }
};

#endif // !_UDP_WRITER_HPP_
