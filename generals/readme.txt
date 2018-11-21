The following program implements the recursive algorithm OM(m) from 
Lamport's paper on the Byzantine Generals problem. The algorithm uses a simple
linked list of generals who recursively take turns as commanders.
They do this by removing themselves from the linked list, iterating on all the 
generals and evaluating the recursive algorithm down to the base case, before
finally making a decision.

The test files provided are pass.bat and fail.bat,
which run the program with various inputs that should 
pass or fail respectively. 

(Note of course, that the tests in fail.bat don't ALWAYS fail. 
However, the ones in pass.bat should, and do, always pass.)

The tests work as follows, with the simulation being set up 
as is stated in the source, with command line arguments of:
	"\tn - number of generals\n"
	"\tt - number of traitors\n"
	"\tm - recursion depth\n"
	"\tc - 0 or 1, whether commander is a traitor\n"

The algorithm is run, and for our purposes the commander's order will always attack
because retreat is just a semantic difference. (Our traitors just act randomly.)
We repeat the simulation 100 times, and if IC1 or IC2 are violated we report the
error and fail. No failure indicates success.

	// IC1. All loyal lieutenants obey the same order. 
	// IC2. If the commanding general is loyal, then every loyal lieutenant obeys the order he sends.
	
I have provided pass.txt and fail.txt, and as you can see, pass succeeds all 100 rounds
for each set of inputs. In fail.txt there were only three out five failures, but this is not unexpected.

Test inputs were chosen using theorem one from the paper which is:
	
	// THEOREM 1. For any m, Algorithm OM(m) 
	// satisfies conditions IC1 and IC2 if there are more than 
	// 3m generals and at most m traitors. 