# ADVANCED-FLAPPY-BIRD-IS1200-KTH
Flappy Bird
Made by: Sai Santhoshi Srinithya Darbha and Sreenija Veladri


1. Running the Code in Terminal:
   - Navigate to the Flappy Bird directory.
   - Execute the following commands:
     $ make
     $ make install 

3. Encoding -the buttons on the chipkit were encoded to ensure that it is easy to play the game 
   - BTN1 is called btn4
   - BTN2 is called btn3
   - BTN3 is called btn2
   - BTN4 is called btn1
   - In the below steps the encoded button names will be used to explain the steps to play the game 
   
4.  Game 
   - Select player mode - 1 player or 2 players 
   - Select difficulty- easy, medium, or hard (the gaps decrease in size for different levels -least gap for hard levels)
   - Input name
       btn1 - moves backward through the alphabet
       btn2 - selects the current alphabet 
       btn3 - moves forward through the alphabet
   - Press any button to enter the game screen - displays bird and pipes
   - To play the game - i.e to start moving the pipes and the bird - press any button 
   - Button encoding to move the bird to play the game 
      btn1 - move left
      btn2 - move up 
      btn3 - move right
   - Press btn2 to continuously play the  game - if it hits the bottom of the screen: the attempt has ended
   - If bird touches pipes: attempt ended
   - If bird passes through a pipe - the score is incremented by 1
   - Once the attempt has ended - the score is calculated 
   - Each player has 3 attempts 
   
4. Scoring, podium (top 3 highest scores), and "Play again"
   - Sum of scores from 3 attempts - final score 
   - 1 player mode - final score displayed and after pressing any button
   - Press the button again - select button 1 to display the podium, select button 2 to replay the game- the game can be played again  
   - 2-player mode - final scores for both players displayed 
   - The final scores of the 2 players are compared and the one with the highest score is the winner 
   - Winner displayed along with name and score 
   - If scores are the same - "It's a tie" displayed
   - Press the button again - select button 1 to see the podium, and select button 2 to replay the game.


  
