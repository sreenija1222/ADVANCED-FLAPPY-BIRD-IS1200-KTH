/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):
   The file has been modified by Sai Santhoshi Srinithya Darbha and Sreenija Veladri.

   This file modified 2017-04-31 by Ture Teknolog

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */
#include <stdbool.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>


// all other game requirements definitions
#define MAXLEN              3
#define ROUNDS              3
#define MAX_ATTEMPS         100
#define PODIUM              3
#define BUTTON1             0x4 // (PORTD>>5) & BUTTON1
#define BUTTON2             0x2 // (PORTD>>5) & BUTTON2
#define BUTTON3             0x1 // (PORTD>>5) & BUTTON3
#define BUTTON4             0x1 // PORTF & BUTTON4
#define BUTTON1_STAT        ((PORTD >> 5) & 0x4)
#define BUTTON2_STAT        ((PORTD >> 5) & 0x2)
#define BUTTON3_STAT        ((PORTD >> 5) & 0x1)

// screen pixel definitions
#define SCREEN_H            32
#define SCREEN_W            128

// all bird definitions
#define BIRDWIDTH           7
#define BIRDHEIGHT          7
#define GRAVITY_PIXELS      1 
#define JUMP_PIXELS         2

// all pipe definitions
#define LOWERLIMIT_PIPE     5
#define GAP_EASY            15
#define GAP_MEDIUM          13
#define GAP_HARD            10
#define MAX_PIPES           200 
#define PIPESPERSCREEN      5


// funtion prototypes
void enable_interrupt(void);
void init();

// Gloabal variables

// main
int mode;
char *difficulty;
char name[MAXLEN + 1];
int noOfPlayers;


// bird and bird functions
unsigned int xbird;
unsigned int ybird;
int last_xbird = -1;
int last_ybird = -1;

// pipe change by difficulty
int vertical_gap;
int horizontal_gap;

// pipe generation
int random_Bheight[MAX_PIPES];
int top_pipe_height[MAX_PIPES];
int pipeWidths[MAX_PIPES];
int upperlimit_pipe;
int pipeX[PIPESPERSCREEN];

// players, rounds, scores and leaderboard
int currentPlayer = 0; // 0 for Player 1, 1 for Player 2
int currentRound = 0;  // Tracks the current round (0 to ROUNDS - 1)

int game_attempt = 0;
int score;
int scored[MAX_PIPES]= {0} ;// to keep track of wether bird has passed through the pipe 

int highestscores[PODIUM] = {0}; // Initialize array with 0s
int total_scores[MAX_ATTEMPS] = {0};
int currentSize = 0; // Track the current size of the array


int errno;
FILE* const stdout;

// binary matrix to draw bird
const uint8_t bird_matrix[BIRDHEIGHT][BIRDWIDTH] = {
    {0, 0, 1, 1, 1, 0, 0}, // Row 1 
    {0, 1, 1, 1, 1, 1, 0}, // Row 2 
    {1, 1, 1, 1, 1, 1, 1}, // Row 3 
    {1, 1, 1, 1, 1, 1, 1}, // Row 4 
    {0, 1, 1, 1, 1, 1, 0}, // Row 5 
    {0, 0, 1, 1, 1, 0, 0}, // Row 6 
    {0, 0, 1, 0, 1, 0, 0}, // Row 7 
};

// to group all the data (name, scores, final score) per player
struct player {
    char name[MAXLEN + 1];
    int scores[ROUNDS];
    int finalscore;
};

struct player players[2];


// blocking call
int get_button() {
    int val;
    // wait until press
    while ((val = (PORTD >> 5) & 0b00000111) == 0);
    // wait until depess
    while (((PORTD >> 5) & 0b00000111) != 0);
    return val;
}

// 4th button
int get_extrabutton() {
    return PORTF & 0b0001;
}

// makes sure the program continues even if button 2 is not pressed or released
int press_btn(){
    int i;
    int timeout = 100000;
    // wait until depess or timeout
    for (i = 0; i < timeout && BUTTON2_STAT; i++) {
        ;
    }
    if (i > 0)
        return 1;
    else
        return 0;
}

// returns the button thats pressed, else 0
int buttons() {
    if (BUTTON1_STAT) {
        return BUTTON1; 
    }
    if (BUTTON2_STAT) {
        return BUTTON2; 
    }
    if (BUTTON3_STAT) {
        return BUTTON3;
    }
    return 0;
}

// selecting number of players (single or dual players)
int select_mode(void) {
    display_string(0, "1 player: b1");
    display_string(2, "2 players: b2");
    display_update();

    int button_pressed;
    do {
        button_pressed = get_button(); // Blocking call to wait for button press
    } while (button_pressed == 0); // Loop until a button is pressed

    if (button_pressed & BUTTON1) {
        noOfPlayers = 1;
        return noOfPlayers;
    } else if (button_pressed & BUTTON2) {
        noOfPlayers = 2;
        return noOfPlayers;
    }
    return 0;
}

// selecting difficulty mode (determines gap between top and bottom pipe)
char* select_difficulty() {
    int button_pressed;
    display_string(0, "Select Level:");
    display_string(1, "1-Easy");
    display_string(2, "2-Medium");
    display_string(3, "3-Hard");
    display_update();

    do {
        button_pressed = get_button();
    } while (button_pressed != BUTTON1 && button_pressed != BUTTON2 && button_pressed != BUTTON3);

    switch (button_pressed) {
        case BUTTON1:
            vertical_gap = GAP_EASY;
            difficulty = "Level Easy";
            break;
        case BUTTON2:
            vertical_gap = GAP_MEDIUM;
            difficulty = "Level Medium";
            break;
        case BUTTON3:
            vertical_gap = GAP_HARD;
            difficulty = "Level Hard";
            break;
    }
    return difficulty;
}

// sets the player initials/name
char* get_name(int maxlen) {
    char alphabet[26] = "abcdefghijklmnopqrstuvwxyz";
    int button_pressed, i = 0, j = 0;

    do {
        button_pressed = get_button();
        if (button_pressed == BUTTON3) { // Forward through alphabet
            i = (i + 1) % 26;
            display_char(3, 3, alphabet[i]);
            display_update();
        }
        else if (button_pressed == BUTTON1) { // Backward through alphabet
            i = (i == 0) ? 25 : i - 1;
            display_char(3, 3, alphabet[i]);
            display_update();
        }
        else if (button_pressed == BUTTON2) { // Select current character
            if (j < maxlen) {
                name[j] = alphabet[i];
                display_char(2, j, alphabet[i]);
                display_update();
                j++;
            }
        }
    } while ((j < maxlen));

    name[j] = '\0'; // Null-terminate the name
    return name;
}


int draw_bird(int xbird, int ybird) {
    int i, j;
    
    // Clear the previous bird's position if it exists
    if (last_xbird != -1 && last_ybird != -1) {
        // Clear the previous bird's position from the screen
        for (i = 0; i < BIRDHEIGHT; i++) {
            for (j = 0; j < BIRDWIDTH; j++) {
                // Only clear the pixels where the bird was drawn previously
                if (bird_matrix[i][j] == 1) {
                    // Ensure we clear within screen bounds
                    if (last_xbird + j >= 0 && last_xbird + j < SCREEN_W &&
                        last_ybird + i >= 0 && last_ybird + i < SCREEN_H) {
                        TwoDScreen[last_ybird + i][last_xbird + j] = 0;  // Clear the previous bird's pixels
                    }
                }
            }
        }
    }
    
    // Check vertical boundary (ybird must be between 0 and SCREEN_H - BIRDHEIGHT)
    if (ybird < 0 || ybird + BIRDHEIGHT > SCREEN_H) {
        return 1;
    }

    // Check horizontal boundary (xbird must be between 0 and SCREEN_W - BIRDWIDTH)
    if (xbird < 0 || xbird + BIRDWIDTH > SCREEN_W) {
        return 1;
    }


    // Check for collision with pipes
    for (i = 0; i < BIRDHEIGHT; i++) {
        for (j = 0; j < BIRDWIDTH; j++) {
            if (bird_matrix[i][j] == 1) {
                if (xbird + j >= 0 && xbird + j < SCREEN_W &&
                    ybird + i >= 0 && ybird + i < SCREEN_H) {
                    if (TwoDScreen[ybird + i][xbird + j] != 0) {
                        return 1;  // Collision detected with a pipe or obstacle
                    }
                }
            }
        }
    }


    // Draw the bird at the new position
    for (i = 0; i < BIRDHEIGHT; i++) {
        for (j = 0; j < BIRDWIDTH; j++) {
            if (bird_matrix[i][j] == 1) {
                if (xbird + j >= 0 && xbird + j < SCREEN_W &&
                    ybird + i >= 0 && ybird + i < SCREEN_H) {
                    TwoDScreen[ybird + i][xbird + j] = 1;
                }
            }
        }
    }

    // Update the last known position of the bird
    last_xbird = xbird;
    last_ybird = ybird;

    return 0; 
}


int get_random(int lowerlimit, int upperlimit) {
    return rand() % (upperlimit - lowerlimit + 1) + lowerlimit;
}

void get_height_for_bottom_pipe() {
    upperlimit_pipe = SCREEN_H - (LOWERLIMIT_PIPE + vertical_gap);
    for (int i = 0; i < MAX_PIPES; i++) {
        random_Bheight[i] = get_random(LOWERLIMIT_PIPE, upperlimit_pipe);
    }
}


// calculating the top pipe height based on bottom pipe height
void get_toppipeheight() {
    for (int i = 0; i < MAX_PIPES; i++) {
        top_pipe_height[i] = SCREEN_H - (random_Bheight[i] + vertical_gap);
    }
}

// Function to draw a single pipe
void draw_pipe(int x, int bottomHeight, int topHeight, int pipeWidth) {
    int i, j;

    // Draw the top pipe
    for (i = 0; i < pipeWidth; i++) {
        for (j = 0; j < topHeight; j++) {
            TwoDScreen[j][x + i] = 1;
        }
    }

    // Draw the bottom pipe
    for (i = 0; i < pipeWidth; i++) {
        for (j = topHeight + vertical_gap; j < SCREEN_H; j++) {
            TwoDScreen[j][x + i] = 1;
        }
    }
}

// move pipes and adjust speed dynamically
void move_pipes(int numPipes) {
    int i, j;

    for (i = 0; i < numPipes; i++) {    
        // Clear top pipe section
        for (j = 0; j < top_pipe_height[i]; j++) {
            for (int k = 0; k < pipeWidths[i]; k++) {
                TwoDScreen[j][pipeX[i] + k] = 0;
            }
        }

        // Clear bottom pipe section
        for (j = top_pipe_height[i] + vertical_gap; j < SCREEN_H; j++) {
            for (int k = 0; k < pipeWidths[i]; k++) {
                TwoDScreen[j][pipeX[i] + k] = 0;
            }
        }

        // Move the pipe to the left
        pipeX[i]--;


        //Moving faster and smaller spaces
        if (players[currentPlayer].scores[currentRound] >= 6 && players[currentPlayer].scores[currentRound] <= 10){
            pipeX[i]--;
            pipeWidths[i] = 6;
        } 

        if (players[currentPlayer].scores[currentRound] >= 11 && players[currentPlayer].scores[currentRound] <= 15){
            pipeX[i]--;
            pipeWidths[i] = 7;
        } 

        if (players[currentPlayer].scores[currentRound] >= 16 && players[currentPlayer].scores[currentRound] <= 20){
            pipeX[i]--;
            pipeX[i]--;
            pipeWidths[i] = 8;
        } 

        if (players[currentPlayer].scores[currentRound] >= 21 && players[currentPlayer].scores[currentRound] <= 200){
            pipeX[i]--;
            pipeX[i]--;
            pipeWidths[i] = 9;
        } 


        // Increment score if the bird passes the pipe
        if (pipeX[i] + pipeWidths[i] < xbird && !scored[i]) {
            players[currentPlayer].scores[currentRound]++;
            scored[i] = 1;     // Mark this pipe as scored
        }

        // When pipe moves off screen, reset it to the far right with new random heights
        if (pipeX[i] + pipeWidths[i] < 0) {
            pipeX[i] = 119;

            scored[i]=0;

            // Recalculate random heights for top and bottom pipes
            random_Bheight[i] = get_random(LOWERLIMIT_PIPE, upperlimit_pipe);
            top_pipe_height[i] = SCREEN_H - (random_Bheight[i] + vertical_gap);
        }

        // Draw the pipe at the new position
        draw_pipe(pipeX[i], random_Bheight[i], top_pipe_height[i], pipeWidths[i]);
    }
}



void bird_y_movement() {
    if (press_btn()) {
        TMR2 = 0x0;            // Reset Timer 2
        ybird -= JUMP_PIXELS;  // Move the bird up when the button is pressed
    }
}

void bird_x_movement() {
    int button_pressed = buttons();

    if (button_pressed == BUTTON3) {  
        xbird += 1;
    } 
    else if (button_pressed == BUTTON1) {  
        xbird -= 1;
    }
}

// clear the entire TwoDScreen buffer
void clear_screen() {
    int i, j;
    for (i = 0; i < SCREEN_H; i++) {
        for (j = 0; j < SCREEN_W; j++) {
            TwoDScreen[i][j] = 0;
        }
    }
}

void display_game_over() {
    display_clear();
    display_string(1, "Game Over");    
    display_update();
}

void labinit(void) {
    // Configure PORTD for buttons
    TRISDSET = 0xFE0;  // Set bits 5-11 as inputs (buttons)

    // Configure PORTF for LEDs
    TRISFCLR = 0x70;   // Set bits 4-6 as outputs (LEDs)
    PORTFCLR = 0x70;   // Clear initial LED states
}

void user_isr(void) {
    if (IFS(0) & 0x100) { // Check Timer 2 interrupt flag
        IFS(0) &= ~0x100; // Clear Timer 2 interrupt flag
        ybird += GRAVITY_PIXELS;
        draw_bird(xbird, ybird);
    }
}


void configureTMR2() {
    T2CON = 0x0;                 // Stop Timer 2 and clear control register
    T2CONSET = 0x8000 | 0x0070;  // Turn on 32-bit mode and set prescaler to 1:256
    TMR2 = 0x0;                  // Clear Timer 2 register
    PR2 = 62499;                 // Set period register for a 0.5-second timeout

    IFS(0) &= ~0x100;            // Clear Timer 2 interrupt flag
    IPC(2) |= 0x1F;              // Set Timer 2 priority to 7
    IEC(0) |= 0x100;             // Enable Timer 2 interrupts

    T2CONSET = 0x8000;           // Start Timer 2
    enable_interrupt();          // Enable interrupts globally
}

// adding only the top 3 scores into the leaderboard using the sorting algorithm
void add_element(int highestscores[], int allscores) {
    if (currentSize < PODIUM) {
        highestscores[currentSize] = allscores;
        currentSize++;
    } else {
        // Find the smallest element in the array
        int minIndex = 0;
        for (int i = 1; i < PODIUM; i++) {
            if (highestscores[i] < highestscores[minIndex]) {
                minIndex = i;
            }
        }
        // Replace the smallest element if the new element is larger
        if (allscores > highestscores[minIndex]) {
            highestscores[minIndex] = allscores;
        }
    }

    // Sort the array in descending order
    for (int i = 0; i < currentSize - 1; i++) {
        for (int j = i + 1; j < currentSize; j++) {
            if (highestscores[i] < highestscores[j]) {
                int temp = highestscores[i];
                highestscores[i] = highestscores[j];
                highestscores[j] = temp;
            }
        }
    }
}



int play(struct player *p, int currentRound) {
    players[currentPlayer].scores[currentRound] = 0;

    labinit();
    configureTMR2();
    
    while (1) {
        draw_bird(xbird, ybird);
        move_pipes(PIPESPERSCREEN);
        convert_and_display_image();

        // move bird if the button is pressed
        bird_x_movement();
        bird_y_movement();

        // exit the game if collision occurs
        if(draw_bird(xbird, ybird))
            break;
    
        // some delay so that the pipes dont move so fast
        quicksleep(500000);
    }
    // Stop Timer 2
    T2CONCLR = 0x8000;
    IEC(0) &= ~0x100;

    display_game_over();
    return p->scores[currentRound];  // Return the player's score for this round
}

void game() {
    init();
    display_init();
    display_clear();
    display_update();
    display_string(1, "Welcome!!! ");
    display_string(2, "Flappy Bird Game");
    display_update();
    quicksleep(10000000);
    display_clear();
    display_update();

    display_clear();
    display_update();
    display_string(0, "ENCODING NOTICE ");
    display_string(1, "Button order ");
    display_string(2, "swapped");
    display_update();
    quicksleep(20000000);
    display_clear();
    display_update();

    display_clear();
    display_update();
    display_string(0, "BTN4- b1");
    display_string(1, "BTN3- b2");
    display_string(2, "BTN2- b3");
    display_string(3, "BTN1- b4");
    display_update();
    quicksleep(30000000);
    display_clear();
    display_update();


    select_mode();
    display_clear();
    display_update();

    char temp1[16];
    sprintf(temp1, "%d Player(s)", noOfPlayers);
    display_string(1, temp1);
    display_update();
    quicksleep(10000000);
    display_clear();
    
    // Select difficulty level
    select_difficulty();
    display_clear();
    display_update();
    display_string(1, difficulty);
    display_update();
    quicksleep(10000000);

    display_clear();
    display_update();

    // Set up names for each player
    for (int i = 0; i < noOfPlayers; i++) {
        char temp2[16];
        sprintf(temp2, "Player %d name:", i+1);
        display_string(0, temp2);
        display_update();
    
        char* result = get_name(MAXLEN);  // Store the returned pointer
        for (int k = 0; k < MAXLEN; k++) {
            players[i].name[k] = result[k];
        }

        display_clear();
        display_update();

        char temp3[16];
        sprintf(temp3, "%d: %s", i+1, players[i].name);
        display_string(1, temp3);
        display_update();
        quicksleep(10000000);
    
        display_clear();
        display_update();
    }

    display_clear();
    display_update();
    display_string(0, "Buttons to play");
    display_string(1, "b1- left");
    display_string(2, "b2- up");
    display_string(3, "b3- right");
    display_update();
    quicksleep(40000000);
    display_clear();
    display_update();

    // each player plays three rounds
    for (currentRound = 0; currentRound < ROUNDS; currentRound++) {
        for (currentPlayer = 0; currentPlayer < noOfPlayers; currentPlayer++) {
            display_string(1, players[currentPlayer].name);
            display_string(2, "Round ");
            display_char(2, 6, '1' + currentRound);
            display_update();
            get_button();
            
            // generate initial heights for pipes
            get_height_for_bottom_pipe();  
            get_toppipeheight();

            // initialise bird position
            xbird = 3;
            ybird = 15;

            // initialise pipe positions
            pipeX[0] = 15;
            pipeX[1] = 41;
            pipeX[2] = 67;
            pipeX[3] = 93;
            pipeX[4] = 119;

            // initialise pipe widths
            pipeWidths[0] = 5;
            pipeWidths[1] = 5;
            pipeWidths[2] = 5;
            pipeWidths[3] = 5;
            pipeWidths[4] = 5;


            draw_bird(xbird, ybird);
            move_pipes(PIPESPERSCREEN);
            convert_and_display_image();

            get_button();  // wait for any button press to start the game
    
            play(&players[currentPlayer], currentRound);
            
            // display score per round
            char temp4[16];
            sprintf(temp4, "%s: %d", "Score", players[currentPlayer].scores[currentRound]);
            display_string(1, temp4);
            display_update();
            get_button();


            quicksleep(1000000);
            display_clear();

            clear_screen();
            convert_and_display_image();
        }
    }

    // calculate and display final scores
    for (currentPlayer = 0; currentPlayer < noOfPlayers; currentPlayer++) {
        players[currentPlayer].finalscore = 0;
        for (currentRound = 0; currentRound < ROUNDS; currentRound++) {
            players[currentPlayer].finalscore += players[currentPlayer].scores[currentRound];
        }
        
        char temp5[32];
        sprintf(temp5, "%s's %s: %d", players[currentPlayer].name, "Total", players[currentPlayer].finalscore);
        display_string(currentPlayer, temp5);

        // Determine and display the winner
        if(noOfPlayers == 2) {
            if (players[0].finalscore > players[1].finalscore) {
                char temp6[32];
                sprintf(temp6, "%s %s", "Winner Is", players[0].name);
                display_string(2, temp6);
            } else if (players[1].finalscore > players[0].finalscore) {
                char temp7[32];
                sprintf(temp7, "%s %s", "Winner Is", players[1].name);
                display_string(2, temp7);
            } else {
                display_string(2, "It's A Tie!");
            }
        }
        display_update();
        get_button();

        quicksleep(1000000);
        display_clear();
    }

    if(noOfPlayers == 1) {
        total_scores[game_attempt] = players[0].finalscore;
        for (int i = 0; i < game_attempt + 1; i++) {
            add_element(highestscores, total_scores[i]);
        }
    }
}

void main() {
    while(game_attempt < MAX_ATTEMPS) {
        game();
        display_clear();
        display_string(0, "b1: Podium");
        display_string(1, "b2: Play Again");
        display_update();

        int button_pressed = get_button();
        if (button_pressed == BUTTON1) {
            display_clear();
            display_string(0, "Higest Scores: ");
            display_num(1, 12, highestscores[0]);
            display_num(2, 12, highestscores[1]);
            display_num(3, 12, highestscores[2]);
            display_update();
            display_clear();

            // Wait for button 2 to restart the main while loop
            int next_button_pressed = 0;
            while (next_button_pressed != BUTTON2) {
                next_button_pressed = get_button();
            }

        } else if (button_pressed == BUTTON2) {
            game_attempt++;
            continue;
        }
    }
}



