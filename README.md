# arduino-tetris
A Tetris game made on Arduino Uno Rev3 with a joystick, holding platform and an AzDelivery 1.77" TFT 160x128 RGB display.
## Wiring
<img src="./images/arduino_tetris_wiring.svg" width="85%"/>

The resistors are used, because Arduino is outputting 5V and the display expects 3.3V.

## Platform
<img src="./images/arduino_tetris_floorplan.png" width="40%"/> <img src="./images/arduino_tetris_platform_cornerview.png" width="50%"/>

## How to play
### Movement
To start press the joystick like a button. Left and right movement of the stick moves the tetrominoes, holding
it up rotates them and pulling downwards makes them fall faster.
### Scoring
Every filled/destroyed row gives you 1 point. (There are no special points like in standard Tetris.)
With every filled row the falling speed/time to move the tetrominoes is getting faster. If the second top most row gets filled
the game ends (second and not first, because the first row is never filled :') ).

## Pictures
<img src="./images/arduino_tetris_picture1.jpg" width="40%" /> <img src="./images/arduino_tetris_picture2.jpg" width="40%" />
<img src="./images/arduino_tetris_picture3.jpg" width="40%" /> <img src="./images/arduino_tetris_picture4.jpg" width="40%" />
