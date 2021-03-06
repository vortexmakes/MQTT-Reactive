# MQTT-Reactive
MQTT-Reactive is a [MQTT v3.1.1](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html) client derived from 
[LiamBindle’s MQTT-C](https://github.com/LiamBindle/MQTT-C) library. The aim 
of MQTT-Reactive is to provide a portable and non-blocking MQTT client written in C in order to be used in reactive 
embedded systems.

Many embedded systems are reactive, i.e. they react to internal or external events. Once these reactions are completed, the software goes back to wait for the next event. That is why event-driven systems are alternatively called reactive systems.
The event-driven programming or simply reactive programming is one of the most suitable programming paradigms to achieve a flexible, predictable and maintainable software for reactive systems. In this paradigm the flow of the program is determined by events. Frequently, the reactive software’s structure is composed of several concurrent units, as known as active objects, which wait and process different kinds of events. Each active object owns a thread of control and an event queue through which it processes its incoming events. In reactive systems, the active objects have typically state-based behavior defined in a statechart.

In order to explore how to use the MQTT-Reactive library in a reactive system with multiple and  concurrent tasks and using both a state machine and the event-driven paradigm, we use an IoT device as an example.

The idea of using MQTT protocol was born while an IoT device was being developed for a railway company. This device was a clear reactive system that was able to:
- detect and store changes of several digital inputs
- acquire, filter and store several analog signals
- send stored information to a remote server periodically 
- send and receive information through MQTT protocol over GSM network

MQTT was chosen because it is a lightweight publisher-subscriber-based messaging protocol that is commonly used in IoT and networking applications where high-latency and low data-rate links are expected such as the GSM networks.

The MQTT capability for the mentioned IoT device was accomplished by using a modified version of LiamBindle’s MQTT-C. Since the software of that device had been designed as a reactive software, MQTT-C had to be modified to communicate it with the rest of the system by exchanging asynchronous events. These events were used for receiving and sending traffic over the network as well as for connecting and publishing sensitive information to a server. The resulting software library was called MQTT-Reactive.

MQTT-Reactive was used through a state machine as shown in Figure 1, which models the basic behavior of a MQTT-Reactive 
client. It was an active object called `MqttMgr` (MQTT Manager), which provided strict encapsulation of the MQTT-Reactive code and it was the only entity allowed to call any MQTT-Reactive function or access MQTT-Reactive data.
The state machine actions in Figure 1 demonstrate how the MQTT-Reactive library could be used from a state machine. 
Even though the C language was used as the action language in Figure 1, any computer or formal language can be used. 

![Figure 1. State machine of a MQTT-Reactive client (Source: VortexMakes)](/model/mqtt-reactive-model-code.png)

The state machine in Figure 1 starts in the `WaitingForNetConnection` state. After a network connection is established to a server, the `WaitingForNetConnection` receives the `Activate` event, and then the state machine transitions to `WaitingForSync` state. Only in this state can the state machine stage MQTT messages to be sent to the broker such as [CONNECT](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028) or [PUBLISH](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718037) through the `Connect` and `Publish` events respectively. The `Sync` state uses an UML’s special mechanism for deferring the `Publish` event that is specified by the `defer` keyword included in the internal compartment of the `Sync` state. If the `Publish` event occurs when `Sync` is the current state, it will be saved (deferred) for future processing until the SM enters in a state in which the `Publish` event is not in its deferred event list such as `WaitingForSync` or `WaitingForNetConnection`. Upon entry to such states, the state machine will automatically recall any saved `Publish` event and will then either consume or discard this event according to the transition target state.

Every `SyncTime` milliseconds the state machine transitions to the `Sync` composite state, which does the actual sending and receiving of traffic from the network by posting `Receive` and `Send` events to the network manager. It is a concurrent entity that deals with network issues.
Even though the introduced `MqttMgr` SM only supports the CONNECT and PUBLISH packets, it could support the [SUBSCRIBE](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718063) packet with rather simple changes.

The following table summarizes the involved events in the shown state machine.

Event | Parameter Name | Parameter Type
------|----------------|---------------
Active | - | -
Deactive | - | -
NetDisconnected | - | -
Connect | clientId | char [23]
Connect | keepAlive | uint16_t
Publish | data | uint8_t [2048]
Publish | size | int
Publish | topic| char [20]
Publish | qos | uint8_t
ConnAccepted | - | -
ConnRefused | code | MQTTConnackReturnCode
Receive | - | -
Received | data | uint8_t [1024]
Received | size | int
Send | data | uint8_t [2048]
Send | size | int
Sent | - | -
ReceiveFail | - | -
SendFail | - | -


The state machine actions access to the parameters of the consumed event by using the `params` keyword. For example, in the following transition, the `Connect` event carries two parameters, `clientId` and `keepAlive`, whose values are used to update the corresponding `MqttMgr` object’s attributes:

```c
Connect(clientId, keepAlive)/
        me->clientId = params->clientId;
        me->keepAlive = params->keepAlive;
        me->operRes = mqtt_connect(&me->client, me->clientId, NULL, NULL, 0, 
                                   NULL, NULL, 0, me->keepAlive);
```

In this example, the `Connect(clientId, keepAlive)` event is the trigger of the transition and the `mqtt_connect()` call is part of the action that is executed as a result. In other words, when the `MqttMgr` object receives a `Connect(clientId, keepAlive)` event with the parameters of `publishing_client` and `400`, `Connect(“publishing_client”, 400)`, the `MqttMgr`’s `clientId` and `keepAlive` attributes are updated with the values `publishing_client` and `400` consequently.

In order to create and send events the state machine’s actions use the `GEN()` macro. For example, the following statement sends a `Receive` event to the `Collector` object, which is referenced as a `MqttMgr` object’s attribute by `itsCollector` pointer:

```c
GEN(me->itsCollector, Receive());
```

The first argument of the `GEN()` statement is the object that receives the event, whereas the second argument is the event being sent, including event arguments (if there are any). The arguments must agree with the event parameters. For example, the following statement generates a `ConnRefused(code)` event and sends it to the `Collector` object passing the code returned by the broker as an event parameter: 

```c
GEN(me->itsCollector, ConRefused(code));
```

The idea of using `params` keyword to access the consumed event’s parameters and `GEN()` macro to generate events from actions was adopted from Rational Rhapsody Developer’s code generator for purely illustrative purposes. 

The state machine‘s default action in Figure 1 sets the callback that is called by MQTT-Reactive whenever a connection acceptance is received from the broker. This callback should be implemented within `MqttMgr` code. This callback must generate either `ConnAccepted` or `ConnRefused(code)` events for being sent to the `Collector` object as it shown below. 

```c
static void
connack_response_callback(enum MQTTConnackReturnCode return_code)
{
    /*...*/
    if (return_code == MQTT_CONNACK_ACCEPTED)
    {
        GEN(me->itsCollector, ConnAccepted());
    }
    else
    {
        GEN(me->itsCollector, ConnRefused(return_code));
    }
}
```

The model in Figure 1 could be implemented in C or C++ by using either your favourite software tool or just your own state machine implementation. There are different tools available on the Internet to do that, such as RKH framework, QP framework, Yakindu Statechart Tool, or Rational Rhapsody Developer, among others. All of them support Statecharts and C/C++ languages. Moreover, some of them include a tool to draw a Statechart diagram and to generate code from it.

The state machine was executed from an active object called `MqttMgr` (MQTT Manager), which provided strict encapsulation of the MQTT-Reactive code and it was the only entity allowed to call any MQTT-Reactive function or access MQTT-Reactive data. The other concurrent entities in the system as well as any ISRs were only able to use MQTT-Reactive indirectly by exchanging events with MqttMgr. The usage of this mechanism to synchronize concurrent entities and to share data among them avoids dealing with the perils of traditional blocking mechanisms like semaphores, mutex, delays, or event-flags. Those mechanisms could cause unexpected malfunctions that are difficult and tedious to diagnose and fix.

The `MqttMgr` active object encapsulates its attributes as a set of data items. A data item designates a variable with a name and a type, where the type is actually a data type. A data item for `MqttMgr` object is mapped to a member of the object’s structure. The member’s name and type are the same as those of the object’s data. For example, the client attribute of the `MqttMgr` object type is embedded by value as a data member inside the `MqttMgr` structure:

```c
struct MqttMgr 
{
    /* ... */
    struct mqtt_client client; /* attribute client */
    LocalRecvAll localRecv;    /* attribute localRecv */
};
```

The `MqttMgr` object’s data are accessed and modified directly without using accessor or mutator operations. For example, `client` and `localRecv` are accessed through the `me` pointer, which points to an instance of `MqttMgr`.

```c
mqtt_recvMsgError(&me->client, &me->localRecv);
```

The `MqttMgr` has the list of attributes shown in following table.

Attribute | Type | Description
----------|------|------------
clientId | char [23] | It identifies the client to the server. See MQTT v3.1.1 section 3.1.3.1
keepAlive | uint16_t | Time interval measured in seconds. See MQTT v3.1.1 section 3.1.2.10
topic | char [20] | It identifies the information channel to which payload data is published. See MQTT v3.1.1 section 3.3.2.1
qos | uint8_t | Level of assurance for delivery of this message . See MQTT v3.1.1 section 3.3.2.1
client | struct mqtt_client | Instance of MQTT client
sendbuf | uint8_t [2048] | A buffer that will be used for sending messages to the broker
recvbuf | uint8_t [1024] | A buffer that will be used for receiving messages from the broker
operRes | enum MQTTErrors | An enumeration of error codes
errorStr | const char * | String which indicates the last error message 
localSend | LocalSendAll | Data context on sending messages
localRecv | LocalRecvAll | Data context on receiving messages

## License
This project is licensed under the [MIT License](https://opensource.org/licenses/MIT). See the [LICENSE](/LICENSE) file for more details.
