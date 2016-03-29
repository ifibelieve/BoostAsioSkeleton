# BoostAsioSkeleton
Boost Asio Skeleton

Feature

* Boost Asio
* Flatbuffers

Project

* BoostServer
  * Acceptor     accept client connection and send port number of allocated channel 
  * Channel      has dedicated io_service and tcp acceptor. run on dedicated thread group
  * ChannelPool  allocate channel to client
  * TalkToClient connection to client

* BoostClient
  *  TalkToServer connection to server
