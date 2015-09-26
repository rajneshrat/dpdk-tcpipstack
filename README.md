this provides the framework similar to that provided by berkeley socket with underlying tcp, ip layers.

api implemented -

current status -
   interfaces to create, read and write socket is done. check the sample code as provided in socket_tester. There is another sample code(samplesocketclient.c) for berkeley sockets.
These two work in pair, to test one has to run the samplesocketclient.c under normal linus environment and another one(socket tester) is part of overall code.
Furture work is to move them away and create a library which will be statically or dynamically liked to sample code.

compilation and testing -
   download the current parent folder and move it to sample examples of dpdk and run the file help (./help)



dpdk-tcpipstack
===============
