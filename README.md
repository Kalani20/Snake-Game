This is the work done for the Embedded Systems and Design module. 
Objective : Design and implement a Snake game using the ILI9341 display and a joystick.
The game will feature multiple levels, each increasing in complexity. This
 will test the skills in embedded programming, interfacing with
hardware components, and game logic design.

Here, we have criterias to go through the game from level 1 to level 5. 
The game features are described as below. 
1. Basic Gameplay:
o The snake moves on the display, controlled by the joystick.
o The snake's direction changes according to joystick input.
o Food appears randomly on the screen.
o When the snake eats food, its length increases by 1 unit.
o If the snake eats its own body, the game is over.
o There are no walls at the screen's edges; if the snake goes out from one edge, it will reappear on the
  opposite edge.
o Each food consumed adds 1 point to the score, displayed in a corner.
o The game starts at Level 1, with the level increasing by 1 for every 2 points scored.

2. Level Progression:
o Level 1: Basic gameplay as described above.
o Level 2:
▪ A digit corresponding to the last digit of your group number will appear as a barrier.
▪ If the snake hits this digit, the game is over.
▪ Food will not appear too close to this digit, ensuring a margin.
▪ If the snake is near the center when this digit appears, it should be automatically shifted to
  a corner.
o Level 3:
▪ Food starts disappearing 5 seconds after it appears.
▪ A countdown timer will be displayed each time food appears.
o Level 4:
▪ Red food will start appearing; eating it will reduce the score.
o Level 5 and Onwards:
▪ The snake's speed increases by 20% at each new level.
▪ The number of "bad" (red) foods will increase by 1 at each new level.
