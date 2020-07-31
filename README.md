# InfinityLoop
Generator and Solver for the Infinity Loop game

I initially wrote separate programs to generate and solve Infinity Loop puzzles back in 2016 as an answer to this Reddit Daily Programmer challenge: https://www.reddit.com/r/dailyprogrammer/comments/4rug59/20160708_challenge_274_hard_loop_solver/. My initial solution is posted there as /u/gabyjunior.

This new version implements the generator and the solver in one program, with some additional options available when generating puzzles.

## How to compile the program

The main module infinity_loop.c is using functions implemented in the below modules:
- mp_utils.c: big integer management
- mtrand.c: Mersenne Twister pseudorandom number generator

They need to be compiled and linked with the main module by either using the makefile infinity_loop.make if gcc is available in your environment, or your preferred IDE. The source code is not OS specific.

## How to use the program

The below table shows which parameters are expected on the standard input depending on if you want to use the generator or the solver

| Parameter | Used by generator | Used by solver | Constraint |
|-----------|-------------------|----------------|------------|
| Number of rows | X | X | > 0 |
| Number of columns | X | X | > 0 |
| Generate flag | X | X | 1 for generator,  0 for solver |
| Unique solution flag | X || 1 to generate a puzzle that has a unique solution, 0 otherwise |
| Minimum number of edges | X || 1 or 2 |
| Tiles data  || X | rows x columns numbers between 0 and 15 (\*) |

(\*) Each tile is described by the sum of the below values
- 1 = a pipe connects with the tile in previous row / same column
- 2 = a pipe connects with the tile in same row / next column
- 4 = a pipe connects with the tile in next row / same column
- 8 = a pipe connects with the tile in same row / previous column
