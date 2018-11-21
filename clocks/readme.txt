Vector Clocks

I've implemented vector clocks as a simple array 
which can be updated with clock_send() and clock_receive().

To test the implementation I've taken the code from my chatbot in A1
and given every user a clock which is updated as they receive messages
from other users. I tested this by generating an event log like before,
but now checking to ensure that the clocks impose an ordering on events.

Take this message for example:

	Hello xcd, edc left this message: "Hello from edc!"
	clock xcd = :24::12::25::24::15::18::21::21::33::17:

	I look backwards in the log for the latest version of edc's clock
	
	clock edc = :21::12::21::23::15::18::21::15::33::17:

	and we can see that every value in xcd's clock has been updated correctly.

I would have preferred to use a more rigorous test, but I already implemented this,
and I think it verifies the clocks work, even if it is simplistic.