# InfinityLoop
Generator and Solver for the Infinity Loop game

I initially wrote separate programs to generate and solve Infinity Loop puzzles back in 2016 as an answer to this Reddit Daily Programmer challenge: https://www.reddit.com/r/dailyprogrammer/comments/4rug59/20160708_challenge_274_hard_loop_solver/. My initial solution is posted there as /u/gabyjunior.

This new version implements the generator and the solver in one program, with some additional options available when generating puzzles that are described below.

* How to compile the program

The main module infinity_loop.c is using functions implemented in the below modules:
- mp_utils.c: big integer management
- mtrand.c: Mersenne Twister pseudorandom number generator

They need to be compiled and linked with the main module by either using the makefile infinity_loop.make if gcc is available in your environment, or your preferred IDE. The source code is not OS specific.
