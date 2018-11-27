Here is my implementation of a DHT using the Kademlia protocol.
Unfortunately I haven't completely finished testing and debugging the code.
I have written a few tests for individual components like the ordinary hash
table, so you can see some products of my work.

I implemented a subset of the specification available here: http://xlattice.sourceforge.net/components/protocol/kademlia/specs.html
I decided not to worry about anything to do with data expiration
and I chose my parameters to simplify the logic. Aside from that,
the core functionality is largely reflected in my code.
	
There are bugs in the protocol functions for storing and
retrieving data from the DHT, so I'm not sure how correct they are. 
At the very least, I've completed the outline of how the code should
look, but I will need to continue working on it use it in my project.


