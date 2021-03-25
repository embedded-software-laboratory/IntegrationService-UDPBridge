# UDPBridge for eProsima's IntegrationService
The UDPBridge Library is a communication bridge for eProsima's Integration Service. It can be used to tunnel FastRTPS communications bidirectional between two local networks using UDP.

## Prerequisites
### [Boost 1.67.0](https://dl.bintray.com/boostorg/release/1.67.0/source/)
  ```bash
  $ wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.gz
  $ tar -xf boost_1_67_0.tar.gz && cd boost_1_67_0
  $ sudo ./bootstrap.sh --prefix=/usr/local && sudo ./b2 install
  ```
### [eProsima's Integration Service](https://github.com/eProsima/Integration-Service)
  ```bash
  $ git clone --recursive https://github.com/eProsima/integration-service
  $ cd integration-service
  $ git checkout Release/1.0.0
  $ mkdir build && cd build
  $ cmake .. -DTHIRDPARTY=ON # -DTHIRDPARTY=ON for FastRTPS as third-party
  $ make -j4 && sudo make -j4 install
  ```
## Build
Clone the repository:
```bash
 $ git clone https://github.com/embedded-software-laboratory/IntegrationService-UDPBridge
 $ cd IntegrationService-UDPBridge
```
Compile the UDPBridge Library:
```bash
 $ mkdir build && cd build
 $ cmake ..
 $ make -j4
```

## Usage
Configure XML Integration-Service file and run:
```bash
 $ integration_service config.xml
```

## Template Configuration
### XML Client Parameters

```xml
<?xml version="1.0"?>
<is>
...
    <bridge name="udpbridge">
      <library>./build/libis_udp_bridge.so</library> <!-- path to UDPBridge library file -->

      <properties>
        <property>
          <name>SERVER</name>
          <value>FALSE</value>
        </property>

        <property>
          <name>SERVER_IP</name>
          <value>123.123.1.123</value> <!-- IPv4 or IPv6 Address -->
        </property>

        <property>    
          <name>IP_TYPE</name>
          <value>IPv4</value> <!-- IPv4 or IPv6 -->
        </property>

        <property>
          <name>SERVER_PORT</name>
          <value>55555</value>
        </property>

        <property> <!-- optional (default: random) -->
          <name>CLIENT_PORT</name>
          <value>55555</value>
        </property>
      </properties>

      <writer name="udp_writer">
        <property>
          <name>HEARTBEAT_TIMER</name>
          <value>1000000</value> <!-- in microseconds, default: 1000000, 0 to disable heartbeats -->
        </property>
      </writer>

      <reader name="udp_reader">
        <property>
          <name>BUFFER_SIZE</name>
          <value>8192</value> <!-- size of buffer for messages, default: 65536, minimum size must be as large as the maximum packet size -->
        </property>
      </reader>
    </bridge>

   <connector name="UDPConnectorWriter">
      <reader participant_profile="local_fastrtps_reader" subscriber_profile="local_fastrtps_subscriber"/>
      <writer bridge_name="udpbridge" writer_name="udp_writer"/>
   </connector>

   <connector name="UDPConnectorReader">
      <reader bridge_name="udpbridge" reader_name="udp_reader"/>
      <writer participant_profile="local_fastrtps_writer" publisher_profile="local_fastrtps_publisher"/>
   </connector>
...

</is>

```

### XML Server Parameters

```xml
<?xml version="1.0"?>
<is>
...
    <bridge name="udpbridge">
      <library>./build/libis_udp_bridge.so</library> <!-- path to UDPBridge library file -->

      <properties>
        <property>
          <name>SERVER</name>
          <value>TRUE</value>
        </property>

        <property>    
          <name>IP_TYPE</name>
          <value>IPv4</value> <!-- IPv4 or IPv6 -->
        </property>

        <property>
          <name>SERVER_PORT</name>
          <value>55555</value>
        </property>
      </properties>

      <writer name="udp_writer"></writer> <!-- optional -->

      <reader name="udp_reader">
        <property>
          <name>BUFFER_SIZE</name>
          <value>8192</value> <!-- size of buffer for messages, default: 65536, minimum size must be as large as the maximum packet size -->
        </property>
      </reader>
    </bridge>

   <connector name="UDPConnectorWriter">
      <reader participant_profile="local_fastrtps_reader" subscriber_profile="local_fastrtps_subscriber"/>
      <writer bridge_name="udpbridge" writer_name="udp_writer"/>
   </connector>

   <connector name="UDPConnectorReader">
      <reader bridge_name="udpbridge" reader_name="udp_reader"/>
      <writer participant_profile="local_fastrtps_writer" publisher_profile="local_fastrtps_publisher"/>
   </connector>
...

</is>

```