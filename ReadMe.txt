Hidato is a puzzle where you are given an array of boxes that will contain a 
sequence of numbers from 1 to N.  You will be given some numbers in the array
including the first number 1 and the last number.  You are asked to solve the 
puzzle by starting at the digit 1 and going horizontal, vertical or diagonal
to an adjacent entry for the next number.  The puzzle should always have only 
one solution.

So to enter a new puzzle select New in the File Menu and enter the width and height
you desire.  The game accepts anywhere from 2 to 12 for the width and height.
Next you will be asked to select areas that need to be deleted from the puzzle.
The next phase requires you to input numbers that are given to you.  You must 
include the starting and ending numbers.
The third pahse is the solution phase.  You may select Solve from the Game Menu
or select Step or press the space key to provide obvious steps in the solution.

It is easy to solve the puzzle by using a recursive algorithm.  However, this 
is difficult for people to understand why.  So I am doing something different here.
I try to solve the puzzle in "steps" by finding obvious choices.  

When I create a possible path between A and B I create a list of open areas and 
count the number of entries in this area as well as a list of end segments that 
need to be used for each area.  I then cull the list of end segments, removing 
any ends that cannot reach the next end through this area.  Now several situations
will help give us more information:
1. Any path that leaves "dead" areas cannot be the correct path so disregard it.
I define a dead area as any open area with 0 or 1 end segments.  I also include 
in this list any "isloated" end segments that do not have an open area next to them.
See "puzzles/20_04_24 10x10 23 is known.txt".
2. The end segments will give us a count of the number of tiles that will be filled
and if this cannot fill the open area then we can reject this path.  See
"Puzzles/20_05_05 8x8 play 2 at 4,4.txt" and "20_05_05 8x8 play 2 at 4,4.txt".
3. If there is only one way to get from one segment end A to the next end B
then fill this path.  
4. Some times all paths include portions that are same although other entries may 
vary, so fill in the common tiles. See "Puzzles/20_05_23 10x10 rule 4.txt".
5. Because the above condition is necessary it is not sufficient.  It is possible
that all possible paths for all end segments leave some tiles with only one possible 
value.  In that case we use those values.  See "20_05_23 10x10 rule 5.txt".
6. Just in case I missed something I added a brute force method that finds all
solutions to the puzzle and any common tiles are filled in.  I do not believe this
method is needed using the above steps.
7. However, the above algorithm fails if there are more than one solution to 
the puzzle.  See "puzzles/20_04_30 10x10 two solutions.txt".  You can see this 
method stops to show you where there multiple solutions are possible.  There are 
examples in the Puzzles folder.

I have provided a means to load and save puzzles.  Please look in the puzzles
folder for samples of each obvious step.  Most of these puzzles I got from
the Mercury News.  However, you can also find some at:
http://www.shockwave.com/gamelanding/hidato.jsp

By the way the debug version of the program creates files which provide the 
solutions for all the segments it tries.

Ed Logg

I would like to thank the author who posted their code at:
https://rosettacode.org/wiki/Solve_a_Hidato_puzzle#C.2B.2B
Their code will not determine multiple solutions, but even worse it will not 
correctly determine when there no solution.  See "puzzles/no solution.txt".  
I fixed this in my copy of the code.

