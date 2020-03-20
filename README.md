# MQTT-Reactive
MQTT-Reactive is a [MQTT v3.1.1](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html) client derived from 
[LiamBindle’s MQTT-C](https://github.com/LiamBindle/MQTT-C) library. The aim 
of MQTT-Reactive is to provide a portable and non-blocking MQTT client written in C in order to be used in reactive 
embedded systems.

MQTT-Reactive was used through a state machine as shown in Figure 1, which models the basic behavior of a MQTT-Reactive 
client. The state machine actions in Figure 1 demonstrate how the MQTT-Reactive library could be used from a state machine. 
Even though the C language was used as the action language in Figure 1, any computer or formal language can be used. 

![Figure 1. State machine of a MQTT-Reactive client (Source: VortexMakes)](/model/mqtt-reactive-model-code.png)

The state machine in Figure 1 starts in the WaitingForNetConnection state. After a network connection is established to a server, the WaitingForNetConnection receives the Activate event, and then the state machine transitions to WaitingForSync state. Only in this state can the state machine stage MQTT messages to be sent to the broker such as CONNECT or PUBLISH through the Connect and Publish events respectively. The Sync state uses an UML’s special mechanism for deferring the Publish event that is specified by the defer keyword included in the internal compartment of the Sync state. If the  Publish event occurs when Sync is the current state, it will be saved (deferred) for future processing until the SM enters in a state in which the Publish event is not in its deferred event list such as WaitingForSync or WaitingForNetConnection. Upon entry to such states, the state machine will automatically recall any saved Publish event and will then either consume or discard this event according to the transition target state.

Every SyncTime milliseconds the state machine transitions to the Sync composite state, which does the actual sending and receiving of traffic from the network by posting Receive and Send events to the network manager. It is a concurrent entity that deals with network issues.
Even though the introduced MqttMgr only supports the CONNECT and PUBLISH packets, it could support the SUBSCRIBE packet with rather simple changes.

The state machine actions access to the parameters of the consumed event by using the params keyword. For example, in the following transition, the Connect event carries two parameters, clientId and keepAlive, whose values are used to update the corresponding MqttMgr object’s attributes:

```c
Connect(clientId, keepAlive)/
        me->clientId = params->clientId;
        me->keepAlive = params->keepAlive;
        me->operRes = mqtt_connect(&me->client, me->clientId, NULL, NULL, 0, 
                                   NULL, NULL, 0, me->keepAlive);
```

In this example, the Connect(clientId, keepAlive) event is the trigger of the transition and the mqtt_connect() call is part of the action that is executed as a result. In other words, when the MqttMgr object receives a Connect(clientId, keepAlive) event with the parameters of ‘publishing_client’ and ‘400’, Connect(“publishing_client”, 400), the MqttMgr’s clientId and keepAlive attributes are updated with the values ‘publishing_client’ and ‘400’ consequently.

In order to create and send events the state machine’s actions use the GEN() macro. For example, the following statement sends a Receive event to the Collector object, which is referenced as a MqttMgr object’s attribute  by itsCollector pointer:

```c
GEN(me->itsCollector, Receive());
```

The first argument of the GEN() statement is the object that receives the event, whereas the second argument is the event being sent, including event arguments (if there are any). The arguments must agree with the event parameters. For example, the following statement generates a ConnRefused(code) event and sends it to the Collector object passing the code returned by the broker as an event parameter: 

```c
GEN(me->itsCollector, ConRefused(code));
```

The idea of using params keyword to access the consumed event’s parameters and GEN() macro to generate events from actions was adopted from Rational Rhapsody Developer’s code generator for purely illustrative purposes. 

The state machine‘s default action in Figure 1 sets the callback that is called by MQTT-Reactive whenever a connection acceptance is received from the broker. This callback should be implemented within MqttMgr code. This callback must generate either ConnAccepted or ConnRefused(code) events for being sent to the Collector object as it shown below. 

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

This state machine was executed from an active object called MqttMgr (MQTT Manager), which provided strict encapsulation of the MQTT-Reactive code and it was the only entity allowed to call any MQTT-Reactive function or access MQTT-Reactive data. The other concurrent entities in the system as well as any ISRs were only able to use MQTT-Reactive indirectly by exchanging events with MqttMgr. The usage of this mechanism to synchronize concurrent entities and to share data among them avoids dealing with the perils of traditional blocking mechanisms like semaphores, mutex, delays, or event-flags. Those mechanisms could cause unexpected malfunctions that are difficult and tedious to diagnose and fix.

The MqttMgr active object encapsulates its attributes as a set of data items. A data item designates a variable with a name and a type, where the type is actually a data type. A data item for MqttMgr object is mapped to a member of the object’s structure. The member’s name and type are the same as those of the object’s data. For example, the client attribute of the MqttMgr object type is embedded by value as a data member inside the MqttMgr structure:

```c
struct MqttMgr 
{
    /* ... */
    struct mqtt_client client; /* attribute client */
    LocalRecvAll localRecv;    /* attribute localRecv */
};
```

The MqttMgr object’s data are accessed and modified directly without using accessor or mutator operations. For example, client and localRecv are accessed through the me pointer, which points to an instance of MqttMgr.

```c
mqtt_recvMsgError(&me->client, &me->localRecv);
```

## License
This project is licensed under the [MIT License](https://opensource.org/licenses/MIT). See the 
`"LICENSE"` file for more details.

## Authors
MQTT-C was initially developed as a CMPT 434 (Winter Term, 2018) final project at the University of 
Saskatchewan by:
- **Liam Bindle**
- **Demilade Adeoye**
