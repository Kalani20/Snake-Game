
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <EEPROM.h>
#include <stdlib.h> 
#include <time.h>   

// Pin definitions
#define SCREEN_CS     10
#define SCREEN_DC     9
#define JOY_X_PIN  A0
#define JOY_Y_PIN  A1
#define BUZZER_PIN 3
#define JOY_BTN_PIN 5
#define EEPROM_HIGH_SCORE_ADDR 0
#define EEPROM_INITIALIZED_ADDR 1
#define EEPROM_INITIALIZED_VALUE 42

// Initialize display
Adafruit_ILI9341 display = Adafruit_ILI9341(SCREEN_CS, SCREEN_DC);


/* Game Variables
      - score: Current score
      - level: Current level
      - highScore: Current high score
      - overallHighScore: Overall high score
      - isGameOver: Flag to track if game is over
      - toxicFoodCount: Number of toxic food eaten
*/
int score = 0;
int level = 1;
int highScore = 0;
int overallHighScore = 0;
bool isGameOver = false;
int toxicFoodCount = 0; 


/* Enumeration for snake direction
      - UP: Move up
      - DOWN: Move down
      - LEFT: Move left
      - RIGHT: Move right
*/
enum Direction {
  UP,
  DOWN,
  LEFT,
  RIGHT
};

/*
   Obstacle Variables
     -  obstacleArray: Array to store obstacle positions
     -  obstacleSize: Number of obstacles
     -  MAX_OBSTACLES: Maximum number of obstacles allowed
     -  obstacles: Array to store current obstacles
     -  currentBarrierCount: Number of current obstacles
     -  barrierDrawn: Flag to track if obstacles are drawn

*/struct ObstacleElement {
  int x, y;
};
const int obstacleSize = 31; 
ObstacleElement obstacleArray[obstacleSize] = {{2, 2}, {3, 2}, {4, 2}, {5, 2}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 5}, {5, 5}, {4, 5}, {3, 5}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8}};
const int MAX_OBSTACLES = 100;
ObstacleElement obstacles[MAX_OBSTACLES];
int activeBarrierCount = 0;
bool barrierDrawn = false;
bool speedIncreased = false; 


/* Snake Variables
    - snake: Array to store snake segments
    - snakeSegmentCount: Number of snake segments
    - currentDirection: Current direction of snake
*/Direction currentDirection = RIGHT; 
struct SnakeSegment {
  int x, y;
};
SnakeSegment snake[50];
int snakeLength = 1;


/* Food Variables
    - FoodType enumeration to represent GOOD and BAD food
    - Food structure to store food position and type
    - isFoodPresent flag to track if food is present on the screen
    - foodSpawnTime to store the time when food was spawned
    - foodTimeLimit to store the time limit for food to disappear
    - foodDespawnTimer to store the time when food will disappear
    - isFoodDespawnTimerActive flag to track if countdown is active
*/
enum FoodType {
  GOOD,
  BAD
};

struct Food {
  int x, y;
  FoodType type;
};

Food foodItem;
bool isFoodPresent = false;
unsigned long foodSpawnTime = 0;
const unsigned long foodTimeLimit = 5000; 
unsigned long foodDespawnTimer = 0; 
const int COUNTDOWN_TIME = 5;
bool isFoodDespawnTimerActive = false; 

// Game speed control
unsigned long gameDelay = 50; // Initial game speed in milliseconds


/* Enumeration for game states
  INITIAL_SCREEN: Initial screen when game starts
  MENU: Menu screen
  PLAYING: Game is in progress
  VIEW_HIGH_SCORE: Display high score
  GAME_OVER_SCREEN: Game over screen
*/
enum GameState {
  INITIAL_SCREEN,
  MENU,
  PLAYING,
  VIEW_HIGH_SCORE,
  GAME_OVER_SCREEN
};
GameState currentState = MENU;


// Function to display the initial screen
void displayInitialScreen() {
  display.fillScreen(ILI9341_BLACK);
  display.setTextColor(ILI9341_WHITE);
  display.setTextSize(3);
  display.setCursor(50, 60);
  display.println("Snake Game");
  display.setTextSize(2);
  display.setCursor(20, 120);
  display.println("Press Joystick Button");
  display.setCursor(80, 150);
  display.println("to Start");
}
const int MENU_OPTIONS = 2;
const char* menuItems[MENU_OPTIONS] = {
  "Start New Game",
  "High-Score"
};

// Current selected menu index
int selectedMenuIndex = 0;

// Enumeration for joystick directions in menu
enum JoystickAction {
  JOY_NONE,
  JOY_UP,
  JOY_DOWN,
  JOY_SELECT
};

// Function Prototypes
void updateDirection();
JoystickAction getMenuAction();
void generateFood();
void drawSnake();
void deleteSnakeSegment(int x, int y);
void displayFood();
void updateSnakePosition();
void updateFoodPresence();
void increaseLevel();
void showMenu();
void displayMenu();
void saveHighScore();
void playSound(int frequency, int duration);
void soundOnGameOver();
void drawObstacles();
void displayStatus();
void handleMenu();
void startNewGame();
void executeGame();
void displayHighScore();
void viewHighScore();
void handleGameOver();

// Function to read joystick input and update direction
void updateDirection() {
  int xValue = analogRead(JOY_X_PIN);
  int yValue = analogRead(JOY_Y_PIN);

  if (xValue < 400 && currentDirection != LEFT) {
    currentDirection = RIGHT; // Corrected direction
  }
  else if (xValue > 600 && currentDirection != RIGHT) {
    currentDirection = LEFT; // Corrected direction
  }
  else if (yValue > 600 && currentDirection != DOWN) {
    currentDirection = UP;
  }
  else if (yValue < 400 && currentDirection != UP) {
    currentDirection = DOWN;
  }
}

// Function to get joystick action for menu navigation
JoystickAction getMenuAction() {
  int yValue = analogRead(JOY_Y_PIN);
  int btnValue = digitalRead(JOY_BTN_PIN);
    if (yValue > 600) {
    delay(200); // Debounce delay
    return JOY_UP;
  }
  else if (yValue < 400) {
    delay(200); // Debounce delay
    return JOY_DOWN;
  }
  else if (btnValue == LOW) { 
    delay(200); 
    return JOY_SELECT;
  }
  
  return JOY_NONE;
}

// Modify the generateFood function
void generateFood() {
    if (isFoodPresent) {
        display.fillRect(foodItem.x * 10, foodItem.y * 10, 10, 10, ILI9341_BLACK);
        isFoodPresent = false;
    }

    bool collision;
    const int MARGIN = 2;
    do {
        collision = false;
        foodItem.x = random(0, display.width() / 10);
        foodItem.y = random(0, display.height() / 10);

        // Check against snake
        for (int i = 0; i < snakeLength; i++) {
            if (snake[i].x == foodItem.x && snake[i].y == foodItem.y) {
                collision = true;
                break;
            }
        }

        // Check against barriers
        if (!collision) {
            for (int i = 0; i < activeBarrierCount; i++) {
                if (obstacles[i].x == foodItem.x && obstacles[i].y == foodItem.y) {
                    collision = true;
                    break;
                }
            }
        }

        // Check against barriers and their margins
        if (!collision && level >= 2) {
            for (int i = 0; i < activeBarrierCount; i++) {
                if (abs(obstacles[i].x - foodItem.x) <= MARGIN && 
                    abs(obstacles[i].y - foodItem.y) <= MARGIN) {
                    collision = true;
                    break;
                }
            }
        }

    } while (collision);

    if (level >= 5) {
        int totalFood = toxicFoodCount + 1; // toxicFoodCount + 1 good food
        int randomFood = random(0, totalFood);
        foodItem.type = (randomFood < toxicFoodCount) ? BAD : GOOD;
    } else if (level >= 4) {
        // Keep the existing logic for level 4
        foodItem.type = (random(0, 2) == 0) ? GOOD : BAD;
    } else {
        foodItem.type = GOOD;
    }

    isFoodPresent = true;
    foodSpawnTime = millis();

    if (level >= 3) {
        foodDespawnTimer = millis();
        isFoodDespawnTimerActive = true;
    } else {
        isFoodDespawnTimerActive = false;
    }

    displayFood();
}


// Function to draw food
void displayFood() {
  if (isFoodPresent) {
    if (foodItem.type == GOOD) {
      display.fillRect(foodItem.x * 10, foodItem.y * 10, 10, 10, ILI9341_GREEN);
    } else {
      display.fillRect(foodItem.x * 10, foodItem.y * 10, 10, 10, ILI9341_RED);
    }
  }
}


void drawSnake() {
  for (int i = 0; i < snakeLength; i++) {
    display.fillRect(snake[i].x * 10, snake[i].y * 10, 10, 10, ILI9341_WHITE);
  }
}

// Function to erase the last position of the snake segment
void deleteSnakeSegment(int x, int y) {
  display.fillRect(x * 10, y * 10, 10, 10, ILI9341_BLACK);
}


// Function to move the snake based on current direction
void updateSnakePosition() {
  int dirX = 0, dirY = 0;

  // Set direction deltas based on current direction
  switch (currentDirection) {
    case UP:
      dirY = -1;
      break;
    case DOWN:
      dirY = 1;
      break;
    case LEFT:
      dirX = -1;
      break;
    case RIGHT:
      dirX = 1;
      break;
  }

  display.fillRect(snake[snakeLength - 1].x * 10, snake[snakeLength - 1].y * 10, 10, 10, ILI9341_BLACK);
  for (int i = snakeLength - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }
  snake[0].x += dirX;
  snake[0].y += dirY;
  for (int i = 1; i < snakeLength; i++) {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
      isGameOver = true; // Game over
    }
  }

  // Check for collisions with barriers
  for (int i = 0; i < activeBarrierCount; i++) {
    if (snake[0].x == obstacles[i].x && snake[0].y == obstacles[i].y) {
      isGameOver = true; // Game over
      break;
    }
  }

  // Wrap around the screen edges
  if (snake[0].x < 0) snake[0].x = display.width() / 10 - 1;
  if (snake[0].x >= display.width() / 10) snake[0].x = 0;
  if (snake[0].y < 0) snake[0].y = display.height() / 10 - 1;
  if (snake[0].y >= display.height() / 10) snake[0].y = 0;
}

/* ProcessFood function
 * to process GOOD and BAD food based on the current level
 */
void updateFoodPresence() {
  if (isFoodPresent && snake[0].x == foodItem.x && snake[0].y == foodItem.y) {
    if (isFoodPresent) {
      display.fillRect(foodItem.x * 10, foodItem.y * 10, 10, 10, ILI9341_BLACK);
      isFoodPresent = false;
    }

    if (foodItem.type == GOOD) {
      snakeLength++;
      score++;
      if (score % 2 == 0) level++;
      playSound(1500, 100); 
    }
    else {
      score--;
      if (score < 0) score = 0; 
      playSound(300, 100); 
    }

    isFoodDespawnTimerActive = false; 
    generateFood();
  }
}

/* IncreaseLevel function
 * to increase the game speed and toxic food count based on the current level
 */
void increaseLevel() {
    if (level == 2 && !barrierDrawn) {
        for (int i = 0; i < obstacleSize; i++) {
            if (activeBarrierCount < MAX_OBSTACLES) {
                obstacles[activeBarrierCount++] = obstacleArray[i];
                display.fillRect(obstacleArray[i].x * 10, obstacleArray[i].y * 10, 10, 10, ILI9341_BLUE);
            }
        }
        barrierDrawn = true;
    }

  if (level >= 5) {
        gameDelay = 50 * pow(0.8, level - 4);
        toxicFoodCount = level - 4;
      
    }
}


void saveHighScore() {
  if (score > highScore) {
    EEPROM.write(EEPROM_HIGH_SCORE_ADDR, score);  // Save score to EEPROM
    highScore = score;
  }
}
// Replace your current loadHighScore() function with this:
void loadHighScore() {
  // Check if EEPROM has been initialized
  if (EEPROM.read(EEPROM_INITIALIZED_ADDR) != EEPROM_INITIALIZED_VALUE) {
    // EEPROM not initialized, set initial high score to 0
    EEPROM.write(EEPROM_HIGH_SCORE_ADDR, 0);
    EEPROM.write(EEPROM_INITIALIZED_ADDR, EEPROM_INITIALIZED_VALUE);
  }
  
  // Load the high score
  highScore = EEPROM.read(EEPROM_HIGH_SCORE_ADDR);
}

// Function to play sound
void playSound(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration);
}

// Function to play game over sound
void soundOnGameOver() {
  playSound(200, 500); // Example sound on game over
}

// Function to draw barriers
void drawObstacles() {
  for (int i = 0; i < activeBarrierCount; i++) {
    display.fillRect(obstacles[i].x * 10, obstacles[i].y * 10, 10, 10, ILI9341_BLUE);
  }
}

// Function to display the score and level
void displayStatus() {
  // Display Score
  
  display.setCursor(0, 0);
  display.setTextColor(ILI9341_WHITE);
  display.setTextSize(2);
  display.fillRect(0, 0, 120, 20, ILI9341_BLACK); // Clear previous score
  display.print("Score: ");
  display.print(score);

  // Display Level
  display.setCursor(120, 0);
  display.fillRect(120, 0, 120, 20, ILI9341_BLACK); // Clear previous level
  display.print("Level: ");
  display.print(level);
}

void displayMenu() {
  display.fillScreen(ILI9341_BLACK);
  display.setTextColor(ILI9341_WHITE);
  
  // Display title "Snake Game"
  display.setTextSize(3);
  display.setCursor(50, 60);
  display.println("Snake Game");
  
  // Display "Start Game" option
  display.setTextSize(2);
  display.drawRect(60, 120, 200, 40, ILI9341_WHITE);
  display.setCursor(90, 132);
  display.println("Start Game");
  
  // Display High Score option
  display.drawRect(60, 180, 200, 40, ILI9341_WHITE);
  display.setCursor(90, 192);
  display.println("High-Score");
  
  // Highlight the selected menu item
  if (selectedMenuIndex == 0) {
    display.fillRect(60, 120, 200, 40, ILI9341_WHITE);
    display.setTextColor(ILI9341_BLACK);
    display.setCursor(90, 132);
    display.println("Start Game");
  } else {
    display.fillRect(60, 180, 200, 40, ILI9341_WHITE);
    display.setTextColor(ILI9341_BLACK);
    display.setCursor(90, 192);
    display.println("High-Score");
  }
}

// Function to handle menu interactions
void handleMenu() {
  JoystickAction action = getMenuAction();
  
  if (action == JOY_UP || action == JOY_DOWN) {
    // Toggle between the two menu options
    selectedMenuIndex = 1 - selectedMenuIndex;
    displayMenu();
  }
  else if (action == JOY_SELECT) {
    if (selectedMenuIndex == 0) {
      // Start New Game
      startNewGame();
    }
    else if (selectedMenuIndex == 1) {
      // View High Score
      currentState = VIEW_HIGH_SCORE;
      if (overallHighScore == VIEW_HIGH_SCORE) {
        loadHighScore();
      }
      displayHighScore();
    }
  }
}

// Function to initialize and start a new game
void startNewGame() {
  // Reset game variables
  score = 0;
  level = 1;
  isGameOver = false;
  snakeLength = 3;
  activeBarrierCount = 0;
  barrierDrawn = false;
  speedIncreased = false;
  gameDelay = 100;
  currentDirection = RIGHT;
  snake[0].x = display.width() / 20; // Center horizontally
  snake[0].y = display.height() / 20; // Center vertically
  for (int i = 1; i < snakeLength; i++) {
    snake[i].x = snake[0].x - i;
    snake[i].y = snake[0].y;
  }
  
  display.fillScreen(ILI9341_BLACK);
  drawSnake();
  displayStatus(); // Display initial score and level
  generateFood(); // Generate initial food
  
  currentState = PLAYING;
}

// Function containing the main game loop
void executeGame() {
  if (isGameOver) {
    currentState = GAME_OVER_SCREEN;
    return;
  }
  
  updateDirection(); // Update direction based on joystick input
  updateSnakePosition(); // Move snake based on current direction

  // Handle food visibility and timeout based on level
  if (level >= 3) {
    if (isFoodPresent) {
      unsigned long currentTime = millis();
      unsigned long elapsedTime = currentTime - foodSpawnTime;

      // Update the countdown display
      int remainingTime = COUNTDOWN_TIME - (elapsedTime / 1000);
      if (remainingTime < 0) remainingTime = 0;

      // Display the countdown timer
      display.setCursor(0, display.height() - 30); // Position for countdown display
      display.setTextColor(ILI9341_WHITE);
      display.setTextSize(2);
      display.fillRect(0, display.height() - 30, 240, 30, ILI9341_BLACK); // Clear previous text
      display.print("Time Left: ");
      display.print(remainingTime);
      display.print("s");

      // Check if countdown has reached zero
      if (elapsedTime >= foodTimeLimit) {
        generateFood(); // Generate new food and reset timer
      }
    } else {
      generateFood(); // Generate new food if there's no food visible
    }
  } else if (!isFoodPresent) {
    generateFood(); // Always generate food if it's not visible (for levels < 3)
  }

  drawSnake();
  displayFood(); // Ensure food is drawn every frame
  drawObstacles();
  updateFoodPresence(); // Check if food is eaten
  increaseLevel(); 
  displayStatus(); // Update score and level display

  delay(gameDelay); // Control the game speed using gameDelay
}

// Function to display the high score
void displayHighScore() {
  display.fillScreen(ILI9341_BLACK);
  display.setCursor(40, 80);
  display.setTextColor(ILI9341_WHITE);
  display.setTextSize(2);
  display.println("High Score:");
  display.setTextSize(3);
  display.setCursor(80, 120);
  display.println(overallHighScore);
  display.setTextSize(2);
  display.setCursor(40, 160);
  display.println("Press Joystick");
  display.setCursor(40, 190);
  display.println("Button to Menu");
}

// Function to view high score and wait for user to return to menu
void viewHighScore() {
  JoystickAction action = getMenuAction();
  
  if (action == JOY_SELECT) { // Use joystick right as 'select' to return
    currentState = MENU;
    displayMenu();
  }
}

// Function to handle game over screen
void handleGameOver() {
  soundOnGameOver();
  overallHighScore = VIEW_HIGH_SCORE;
  if (score > highScore) {
    overallHighScore = score;
  }
  loadHighScore();
  displayHighScore();
  display.fillScreen(ILI9341_DARKCYAN);  
  display.setCursor(50, 80);
  display.setTextColor(ILI9341_WHITE);
  display.setTextSize(4);
  display.println("Game Over!");
  display.setTextSize(2);
  display.setCursor(50, 140);
  display.print("Score: ");
  display.println(score);
  display.setCursor(50, 180);
  display.print("High Score: ");
  display.println(highScore);
  
  saveHighScore(); // Save high score if needed
  
  // Wait for user to press joystick button to return to menu
  while (true) {
    JoystickAction action = getMenuAction();
    if (action == JOY_SELECT) {
      currentState = MENU;
      displayMenu();
      break;
    }
  }
}
// Add a new function to handle the initial screen
void handleInitialScreen() {
  JoystickAction action = getMenuAction();
  
  if (action == JOY_SELECT) {
    currentState = MENU;
    displayMenu();
  }
}

// Modify the setup() function
void setup() {
  display.begin();
  display.setRotation(3);  // Set rotation based on your need
  display.fillScreen(ILI9341_BLACK);
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(JOY_BTN_PIN, INPUT_PULLUP);  // Set joystick button pin as input with pull-up
  
  randomSeed(analogRead(A2)); // Seed random number generator
  
  loadHighScore(); // Load high score at the start
  
  // Display the initial screen
  displayInitialScreen();
  currentState = INITIAL_SCREEN;  // Set the initial state
}
// Modify the loop() function
void loop() {
  switch (currentState) {
    case INITIAL_SCREEN:
      handleInitialScreen();
      break;
    case MENU:
      handleMenu();
      break;
    case PLAYING:
      executeGame();
      break;
    case VIEW_HIGH_SCORE:
      viewHighScore();
      break;
    case GAME_OVER_SCREEN:
      handleGameOver();
      break;
  }
}