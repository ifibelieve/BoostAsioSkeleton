# BoostAsioSkeleton
Boost Asio Skeleton

ToDo

* add rabbitMQ client
* boost streambuf must be handled delicately

Bug

* has problems in run on visual studio debuger

Feature

* Boost Asio
* Flatbuffers

Project

* BoostServer
  * ChannelDispatcher    accept client connection and send port number of allocated channel 
  * Channel              has dedicated io_service and tcp acceptor. run on dedicated thread group
  * ChannelPool          allocate channel to client
  * TalkToClient         handle connection to client

* BoostClient
  *  TalkToServer handle connection to server
