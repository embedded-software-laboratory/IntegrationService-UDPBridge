#ifndef _UDP_BRIDGE_HPP
#define _UDP_BRIDGE_HPP

#include "ISBridge.h"
#include "log/ISLog.h"
#include "logger.hpp"

#include <boost/asio.hpp>
#include <exception>

#include <mutex>
#include <functional>
#include <sstream>

#include <cstdlib>

using namespace boost::asio;
using boost::asio::ip::udp;
using boost::asio::ip::address;

class UDPBridge : public ISBridge {
public:
    bool m_configServer, m_configIpType;
    std::string m_configServerIp = "127.0.0.1", m_configServerPort = "0", m_configClientPort = "0";

protected:
    bool m_ub_server = false, m_ub_inited = false;
    io_context m_ub_ioContext;
    udp::socket m_ub_socket{m_ub_ioContext};
    ip::udp::endpoint* m_ub_remoteEndpoint = nullptr;
    ip::udp::endpoint m_ub_tmpRemoteEndpoint;
    std::mutex* m_ub_mutexSocketEndpoint;

    SerializedPayload_t m_ub_heartbeatPacket;
    
    public : UDPBridge(const std::string &name, const std::vector<std::pair<std::string, std::string>> *config) : ISBridge(name)
    {
        log("UDPBRIDGE::UDPBRIDGE");

        for(auto& pair : *config) {
            if(pair.first.compare("SERVER") == 0) {
                m_configServer = pair.second.compare("TRUE") == 0;
            }
            else if(pair.first.compare("SERVER_IP") == 0) {
                m_configServerIp = pair.second;
            }
            else if(pair.first.compare("IP_TYPE") == 0) {
                m_configIpType = pair.second.compare("IPv6") == 0;
            }
            else if(pair.first.compare("SERVER_PORT") == 0) {
                m_configServerPort = pair.second;
            }
            else if(pair.first.compare("CLIENT_PORT") == 0) {
                m_configClientPort = pair.second;
            }
        }

        // init mutex
        m_ub_mutexSocketEndpoint = new std::mutex();

        if (m_configServer) { // server
            log("UDPBRIDGE::UDPBRIDGE [init server] (", m_configClientPort, ")");
            
            init(m_configServer, m_configIpType, m_configServerPort);
        }
        else { // client
            log("UDPBRIDGE::UDPBRIDGE [init client] (", m_configClientPort, ")");

            init(m_configServer, m_configIpType, m_configClientPort);

            // set server as remote endpoint
            ip::udp::endpoint serverEndpoint;
            serverEndpoint.address(ip::address::from_string(m_configServerIp));
            serverEndpoint.port(convertPortStrToInt(m_configServerPort));

            setRemoteEndpoint(serverEndpoint);
        }

        // init Heartbeat Packet
        m_ub_heartbeatPacket = SerializedPayload_t(1);
        m_ub_heartbeatPacket.data = reinterpret_cast<eprosima::fastrtps::rtps::octet *>(std::malloc(1));
        m_ub_heartbeatPacket.data[0] = 0b10101010;
        m_ub_heartbeatPacket.length = 1;
    }

    ~UDPBridge() {
        if(m_ub_remoteEndpoint != nullptr) delete m_ub_remoteEndpoint;
        std::free(m_ub_heartbeatPacket.data);
    }

    int convertPortStrToInt(std::string const& p_port) const {
	    int port = std::stoi(p_port.c_str());
        return port;
    }

    void init(bool p_server, bool p_ipType, std::string const& p_bindingPortStr) {
        if(m_ub_inited) return;

        m_ub_server = p_server;

        if(p_ipType == false) { // false = ipv4
            m_ub_socket.open(udp::v4());
            m_ub_socket.bind(udp::endpoint(udp::v4(), convertPortStrToInt(p_bindingPortStr)));
        }
        else {
            m_ub_socket.open(udp::v6());
            m_ub_socket.bind(udp::endpoint(udp::v6(), convertPortStrToInt(p_bindingPortStr)));
        }
        m_ub_socket.non_blocking(true);

        LOG_INFO("(UDPBridge) Socket created and binded.");

        m_ub_inited = true;
    }

    // void runReceiving(SerializedPayload_t* p_payload, std::function<void(const boost::system::error_code&, std::size_t)> func) {
    //     //m_ub_socket.async_receive_from(boost::asio::buffer(recv_buffer_), remote_endpoint_, 
    //     //                               boost::bind(&udp_server::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    //     m_ub_ioContext.run();
    // }

    void setRemoteEndpoint(ip::udp::endpoint const& p_endpoint) {
        mutexLock();

        if(m_ub_remoteEndpoint == nullptr) {
            m_ub_remoteEndpoint = new ip::udp::endpoint(p_endpoint.address(), p_endpoint.port());

            if(m_ub_server) LOG_INFO("(UDPBridge) Remote Endpoint registered.");
        }
        else {
            m_ub_remoteEndpoint->address(p_endpoint.address());
            m_ub_remoteEndpoint->port(p_endpoint.port());
        }

        LOG_INFO("(UDPBridge) Remote Endpoint updated.");

        mutexUnlock();
    }
    
    bool sendToRemoteEndpoint(SerializedPayload_t const* p_payload) {
        mutexLock();

        if(m_ub_remoteEndpoint != nullptr) {
            
            log("LENGTH OF PAYLOAD: ", p_payload->length);

            try {
                m_ub_socket.send_to(boost::asio::buffer(p_payload->data, p_payload->length), *m_ub_remoteEndpoint);
            }
            catch(std::exception &e) {
                logError("Failed to send Message to Remote Endpoint!");
                mutexUnlock();
                return false;
            }

            mutexUnlock();
            return true;
        }
        else {
            logError("(UDPBridge) No Remote Endpoint set!");
            mutexUnlock();
            return false;
        }
    }

    void sendHeartbeat() {
        log("UDPBRIDGE::sendHeartbeat");
        sendToRemoteEndpoint(&m_ub_heartbeatPacket);
    }

    std::size_t receive(std::vector<unsigned char>& p_payload) {
        udp::endpoint senderEndpoint;
        std::size_t recvSize = 0;

        try {
            mutexLock();
            if(m_ub_socket.available()) {
                log("UDPBRIDGE::receive: PACKET RECEIVED!");
                recvSize = m_ub_socket.receive_from(boost::asio::buffer(p_payload), senderEndpoint);
            }
            mutexUnlock();

            if (m_ub_server && recvSize > 0) { // if server => updateRemoteEndpoint
                setRemoteEndpoint(senderEndpoint);
            }
        }
        catch(std::exception &e) {
            // pass
        }

        if (recvSize == 1) {
            if (p_payload[0] == 0b10101010) {
                log("HEARTBEAT RECV");
                return 0;
            }
        }

        return recvSize;
    }

    void mutexLock() {
        // std::cout << " -- MUTEX LOCK -- " << std::endl;
        m_ub_mutexSocketEndpoint->lock();
        // std::cout << " -- MUTEX LOCKED -- " << std::endl;
    }

    void mutexUnlock() {
        // std::cout << " -- MUTEX UNLOCK -- " << std::endl;
        m_ub_mutexSocketEndpoint->unlock();
        // std::cout << " -- MUTEX UNLOCKED -- " << std::endl;
    }
};



#endif // !_UDP_BRIDGE_HPP