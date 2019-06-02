/**
* @file Breakout_FINAL.ino
* @author Tomas Koranda
*
* @brief Arduino project file
*
*/
#include <Esplora.h>
#include <SPI.h>
#include <TFT.h>

/**
 * \defgroup const Constants
 * @{
 */
#define TFT_WIDTH  160          ///< Display width definition
#define TFT_HEIGHT 128          ///< Display height definition
#define TFT_BAR_OFFSET 13       ///< Game offset definition

#define BALL_INIT_POS_X 80      ///< Ball initial position x-axis coordinate definition
#define BALL_INIT_POS_Y 110     ///< Ball initial position y-axis coordinate definition

#define BRICK_COLUMN_COUNT 6    ///< BrickField number of columns definition
#define BRICK_ROW_COUNT    4    ///< BrickField number of rows definition

#define DOWN  SWITCH_1         ///< Down KEY definition
#define UP    SWITCH_3         ///< Up KEY definition
#define ENTER SWITCH_4         ///< Enter KEY definition
#define BACK  SWITCH_2         ///< Back KEY definition
/**@}*/

/**
 * List of all possible states
 */
enum states {
    HOME_SEL_BEGIN_GAME,        ///< Home menu pointing to button begin game
    HOME_SEL_SHOW_BEST_SCORE,   ///< Home menu pointing to button show best score
    IN_GAME,                    ///< Actual game
    SHOW_BEST_SCORE,            ///< List top 3 player scores
    END_GAME,                   ///< End Menu screen, shows score and lets you play again or enter your name
    ENTER_NAME,                 ///< Enter name screen
    END_SEL_ENTER_NAME,         ///< End menu pointing to button enter name
    END_SEL_HOME_MENU           ///< End menu pointing to button go back to home menu
};
/**
 * \defgroup states The automata states
 * @{
 */
enum states STATE;              ///< Current state
enum states NEXT_STATE;         ///< Next state
/**@}*/

/**
 * \defgroup vars Global variables
 * @{
 */

/** @addtogroup ball  Ball variables
  * \ingroup vars
  *  @{
  */

int bx = BALL_INIT_POS_X;   ///< x-axis pos
int by = BALL_INIT_POS_Y;   ///< y-axis pos
int bxl = BALL_INIT_POS_X;  ///< previous x-axis pos
int byl = BALL_INIT_POS_Y;  ///< previous y-axis pos
int ballRadius = 3;         ///< ball raidus

int dx = 5;                  ///< x-axis speed and direction
int dy = -5;                ///< y-axis speed  and direction
/** @} */

/** @addtogroup paddle  Paddle variables
  * \ingroup vars
  *  @{
  */
int paddleHeight = 5;      ///< paddle height
int paddleWidth = 36;      ///< paddle width
int px = TFT_WIDTH / 2 - paddleWidth / 2;    ///< x-axis pos
int py = 123;                                ///< y-axis pos
int pxl = 0;                ///< previous x-axis pos
int pyl = 0;                ///< previous y-axis pos
/** @} */

/** @addtogroup brick  Brick variables
  * \ingroup vars
  *  @{
  */
//bricks version 3
int brickRowCount = BRICK_ROW_COUNT;            ///< number of brick rows
int brickColumnCount = BRICK_COLUMN_COUNT;      ///< number of brick columns
int brickWidth = 25;                            ///< brick width
int brickHeight = 10;                           ///< brick height
int brickPadding = 0;                           ///< space between bricks
int brickOffsetTop = TFT_BAR_OFFSET + 15;       ///< offset of brickfield from top of the screen
int brickOffsetLeft = 5;                        ///< offset of brickfield from left of the screen
int brx = 0;                                    ///< x-axis pos
int bry = 0;                                    ///< y-axis pos
bool brickChange = true;                        ///< state checking if any brick was hit
bool brickDestroyed[BRICK_ROW_COUNT][BRICK_COLUMN_COUNT];   ///< brick existence at given position
int brickRowColor[BRICK_ROW_COUNT][3];          ///< brick row colors
int brickCount = brickRowCount * brickColumnCount; ///< brick field total count
/** @} */

/** @addtogroup stats  Game statss
  * \ingroup vars
  *  @{
  */
short lives = 3;                                    ///< player live count
int score = 0;                                      ///< player in game score
char stringScore[5];                                ///< char aray used for conversion and printing player score
bool statChange = true;                             ///< state checking change of lives and score

char bestPlayers[3][3][2] = {{"P", "_", "1"},
                             {"P", "_", "2"},
                             {"P", "_", "3"}};      ///< top 3 player names
int bestScore[3] = {0, 0, 0};                       ///< top 3 players score
char stringPos[10];                                 ///< char aray used for conversion and printing players position
char stringBestScore[3][5];                         ///< string aray used for conversion and printing players score
char p_name[3][2] = {"A", "A", "A"};                ///< string aray used for reading players name
int char_pos = 0;                                   ///< position of players players cursor when inputing his name
/** @} */

byte buttonFlag = 0;                                ///< used for reading button events
/**@}*/

void setBrickColors() {

    brickRowColor[0][0] = 255;
    brickRowColor[0][1] = 255;
    brickRowColor[0][2] = 204;

    brickRowColor[1][0] = 102;
    brickRowColor[1][1] = 255;
    brickRowColor[1][2] = 178;

    brickRowColor[2][0] = 178;
    brickRowColor[2][1] = 102;
    brickRowColor[2][2] = 255;

    brickRowColor[3][0] = 255;
    brickRowColor[3][1] = 128;
    brickRowColor[3][2] = 0;


}
//--------------------------------------------------------------------------------------------------------------------



/**
 * \defgroup collision Collision procedures
 * @{
 */

/**
 * @brief Detect ball collision with bricks
 * *  iterates from top of brick field
 *
 */
void detect_brick_collisionTop() {
    bry = brickOffsetTop;
    brx = brickOffsetLeft;

    for ( int i = 0; i < brickRowCount; ++i ) {
        for ( int j = 0; j < brickColumnCount; ++j ) {
            if ( !brickDestroyed[i][j]
                 && brx <= bx + ballRadius && bx - ballRadius <= brx + brickWidth
                 && bry <= by + ballRadius && by - ballRadius <= bry + brickHeight
                    ) {
                if ( brx <= bx && bx <= brx + brickWidth )
                    dy = -dy;
                else
                    dx = -dx;

                brickDestroyed[i][j] = true;
                brickChange = true;
                score += lives * (brickRowCount - i);
                brickCount--;
                statChange = true;
                return;
            }
            brx += brickWidth + brickPadding;
        }
        brx = brickOffsetLeft;
        bry += brickPadding + brickHeight;
    }
}

/**
 * @brief Detect ball collision with bricks
 *  iterates from bottom of brick field
 */
void detect_brick_collisionBottom() {

    bry = brickOffsetTop + (brickRowCount - 1) * (brickPadding + brickHeight);
    brx = brickOffsetLeft;
    for ( int i = brickRowCount - 1; i >= 0; --i ) {
        for ( int j = 0; j < brickColumnCount; ++j ) {
            if ( !brickDestroyed[i][j]
                 && brx <= bx + ballRadius && bx - ballRadius <= brx + brickWidth
                 && bry <= by + ballRadius && by - ballRadius <= bry + brickHeight
                    ) {
                if ( brx <= bx && bx <= brx + brickWidth )
                    dy = -dy;
                else
                    dx = -dx;

                brickDestroyed[i][j] = true;
                brickChange = true;
                score += lives * (brickRowCount - i);
                brickCount--;
                statChange = true;
                return;
            }
            brx += brickWidth + brickPadding;
        }
        brx = brickOffsetLeft;
        bry -= brickPadding + brickHeight;


    }

}

/**
 * @brief Detect ball collision with paddle
 *  based on what part of paddle was hit effects the direction of reflection
 */
void detect_paddle_collision() {

    if ( px <= bx + ballRadius && bx - ballRadius <= px + paddleWidth ) { //within paddle
        //direction change based on which part of paddle is hit
        //todo use constants
        if ( bx < px + paddleWidth / 2 ) {
            dx = -5;
        } else if ( bx > px + paddleWidth / 2 ) {
            dx = 5;
        }
        dy = -dy;
    }

}

/**
 * @brief Detect ball collision with walls
 *  if ball hits bottom of screen resets the game and waith for the player to begin
 */
void detect_wall_collision() {

    if ( bx + ballRadius + dx >= TFT_WIDTH || bx - ballRadius + dx <= 0 ) { //sides of screen
        dx = -dx;
    }

    if ( by - ballRadius <= 0 + TFT_BAR_OFFSET ) { //top of screen + padding for info bar //why without + dy => because no padle at top
        dy = -dy;
    } else if ( by + ballRadius >= TFT_HEIGHT ) {
        lives--;
        statChange = true;
        if( lives < 1){
            return;
        }else{
            reset_level();
            wait_for_player();
        }
    } else if ( by + ballRadius >= TFT_HEIGHT - paddleHeight ) { //botom of screen

        detect_paddle_collision();

    }

}

/**
 * @brief Detect any ball collision
 *
 */
void detect_collision() {

    if ( dy > 0 )
        detect_brick_collisionTop();
    else
        detect_brick_collisionBottom();

    detect_wall_collision();//rename to boundry/box etc..?

}
/**@}*/
//-----------------------------------------------
/**
 * \defgroup DRAW In Game draw procedures
 * taking care of moving the game
 * @{
 */
/**
* @brief Draws palyers stats, lives, score
*
*/
void draw_infoBar() {
    //condition to write only if lives or score changed
    if ( statChange ) {
        EsploraTFT.stroke( 255, 255, 255 );
        EsploraTFT.setTextSize( 1 );
        EsploraTFT.text( "Score: ", 5, 5 );

        EsploraTFT.stroke( 0, 0, 0 );
        EsploraTFT.text( stringScore, 40, 5 );

        EsploraTFT.stroke( 255, 255, 255 );
        itoa( score, stringScore, 10 );
        EsploraTFT.text( stringScore, 40, 5 );

        EsploraTFT.text( "Lives: ", 70, 5 );
        for ( int i = 0; i < 3; i++ ) {
            EsploraTFT.stroke( 0, 0, 0 );
            EsploraTFT.text( "o", 110 + i * 7, 5 );
        }
        for ( int i = 0; i < lives; i++ ) {
            EsploraTFT.stroke( 255, 255, 255 );
            EsploraTFT.text( "o", 110 + i * 7, 5 );
        }
        statChange = false;
    }

}

/**
 * @brief Erases all bricks
 *
 */
void erase_bricks() {

    EsploraTFT.stroke( 0, 0, 0 );
    EsploraTFT.fill( 0, 0, 0 );

    bry = brickOffsetTop;
    brx = brickOffsetLeft;
    for ( int i = 0; i < brickRowCount; ++i ) {

        for ( int j = 0; j < brickColumnCount; ++j ) {

            EsploraTFT.rect( brx, bry, brickWidth, brickHeight );
            brx += brickWidth + brickPadding;
        }
        brx = brickOffsetLeft;
        bry += brickPadding + brickHeight;
    }


}

/**
 * @brief Draws all undestroyed bricks
 *
 */
void draw_bricks() {

    //erase_bricks();//

    bry = brickOffsetTop;
    brx = brickOffsetLeft;

    for ( int i = 0; i < brickRowCount; ++i ) {

        EsploraTFT.fill( brickRowColor[i][0], brickRowColor[i][1], brickRowColor[i][2] );

        for ( int j = 0; j < brickColumnCount; ++j ) {
//            EsploraTFT.stroke( brickRowColor[i][0], brickRowColor[i][1], brickRowColor[i][2] );
            EsploraTFT.stroke( 0, 0, 0 ); // solves dents/cutOfCorners in bricks
            EsploraTFT.fill( brickRowColor[i][0], brickRowColor[i][1], brickRowColor[i][2] );
            if ( !brickDestroyed[i][j] ) {
                EsploraTFT.rect( brx, bry, brickWidth, brickHeight );
            } else {
                EsploraTFT.stroke( 0, 0, 0 );
                EsploraTFT.fill( 0, 0, 0 );
                EsploraTFT.rect( brx, bry, brickWidth, brickHeight );
            }
            brx += brickWidth + brickPadding;
        }
        brx = brickOffsetLeft;
        bry += brickPadding + brickHeight;
    }


}

/**
 * @brief Erases ball
 *
 */
void erase_ball() {
    EsploraTFT.stroke( 0, 0, 0 );
    EsploraTFT.fill( 0, 0, 0 );
    EsploraTFT.circle( bxl, byl, ballRadius );
}

/**
 * @brief Draws ball
 *
 */
void draw_ball() {
    EsploraTFT.stroke( 255, 255, 255 );
    EsploraTFT.fill( 255, 255, 255 );
    EsploraTFT.circle( bx, by, ballRadius );
}

/**
 * @brief Moves ball
 *  deletes last ball position and draws it at the new position
 */
void move_ball() {

    bx += dx;
    by += dy;

    erase_ball();
    draw_ball();
    if(score<50)
        delay( 60 );
    else if(score < 100)
        delay( 45 );
    else
        delay( 30 );
    bxl = bx;
    byl = by;

}

/**
 * @brief Erases paddle
 *
 */
void erase_paddle() {
    EsploraTFT.stroke( 0, 0, 0 );
    EsploraTFT.fill( 0, 0, 0 );
    EsploraTFT.rect( pxl, pyl, paddleWidth, paddleHeight );
}

/**
 * @brief Draws paddle
 *
 */
void draw_paddle() {
//    EsploraTFT.stroke( 255, 255, 255 );
    EsploraTFT.stroke( 0, 0, 0 ); //solves dents in paddle
    EsploraTFT.fill( 255, 255, 255 );
    EsploraTFT.rect( px, py, paddleWidth, paddleHeight );
}

/**
 * @brief Moves paddle
 *  deletes last paddle position and draws it at the new position
 */
void move_paddle() {
    px = map( Esplora.readJoystickX(), -512, 512, TFT_WIDTH - paddleWidth, 0 );

    if ( px != pxl ) {
        erase_paddle();
        draw_paddle();
    }

    pxl = px;
    pyl = py;
}
//-------------------------------------------------
/**
 * @brief Draws a debuging grid
 *  used for debuging
 */
void drawGrid() {

    EsploraTFT.stroke( 0, 0, 200 );
    EsploraTFT.setTextSize( 1 );
    EsploraTFT.line( 0, 2, TFT_WIDTH, 2 );//first visible line

    EsploraTFT.line( 0, TFT_HEIGHT, TFT_WIDTH, TFT_HEIGHT );
    EsploraTFT.line( 0, TFT_HEIGHT - 1, TFT_WIDTH, TFT_HEIGHT - 1 ); //last visible lines


    EsploraTFT.line( 0, 13, TFT_WIDTH, 13 ); //max ball pos
    EsploraTFT.line( 0, 123, TFT_WIDTH, 123 ); //min ball pos
    //-----------------------------------------------------------------------------------------------------------------

    EsploraTFT.stroke( 255, 255, 255 );
    EsploraTFT.setTextSize( 1 );

    String str;
    str = "-";
    char charBuf[50];
    str.toCharArray( charBuf, 50 );

    for ( int i = 0; TFT_BAR_OFFSET + i < py; i += 10 ) {
        str = "+";
        str += i + TFT_BAR_OFFSET;
        str.toCharArray( charBuf, 50 );
//        EsploraTFT.line(25, TFT_BAR_OFFSET+i, TFT_WIDTH-25, TFT_BAR_OFFSET+i);
//        EsploraTFT.text(charBuf,0, TFT_BAR_OFFSET+i);
        EsploraTFT.line( 0, TFT_BAR_OFFSET + i, TFT_WIDTH, TFT_BAR_OFFSET + i );
    }
}

/**
 * @brief Game engine
 *  moves all objects, detects collision etc.
 */
void draw_game() {
    draw_infoBar();
//    move_paddle();
    move_ball();
    move_paddle();
    detect_collision();
    if ( brickChange ) {
//        erase_bricks();//slows down game too much
        draw_bricks();
    }
    brickChange = false;
//    drawGrid();
    if ( lives == 0 || brickCount == 0 ) {
        NEXT_STATE = END_GAME;
        return;
    }
}
/**@}*/
//-----------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Reset Game
 * resets player and game stats to default values
 * sets ball and paddle init pos and creates a creates bricks
 */
void reset_game() {
    lives = 3;
    score = 0;
    statChange = true;
    brickChange = true;
    for ( int i = 0; i < brickRowCount; ++i ) {
        for ( int j = 0; j < brickColumnCount; ++j ) {
            brickDestroyed[i][j] = false;
        }
    }

    bx = BALL_INIT_POS_X;
    by = BALL_INIT_POS_Y;
    dx = 5;
    dy = 5;
    brickCount = brickRowCount*brickColumnCount;

}

/**
 * @brief Reset Level sets ball pos prepares game to start
 *
 */
void reset_level() {

    bx = BALL_INIT_POS_X;
    by = BALL_INIT_POS_Y;
    pxl = px;
    px = TFT_WIDTH / 2 - paddleWidth / 2;    ///< x-axis pos


    draw_game();
}

/**
 * @brief Waits for palyer to start the game
 */
void wait_for_player() {
    while ( Esplora.readJoystickY() < 50 && Esplora.readJoystickY() > -50 ) {
        move_paddle();
        delay( 1 );
    }
}
//-----------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Moves cursor between menu buttons
 */
void change_position() {
    EsploraTFT.stroke( 0, 0, 0 );
    EsploraTFT.setTextSize( 1 );
    if ( STATE == HOME_SEL_BEGIN_GAME ) {
        EsploraTFT.text( ">", 10, 25 );
    } else if ( STATE == HOME_SEL_SHOW_BEST_SCORE ) {
        EsploraTFT.text( ">", 10, 35 );
    } else if ( STATE == END_SEL_ENTER_NAME ) {
        EsploraTFT.text( ">", 95, 110 );
    } else if ( STATE == END_SEL_HOME_MENU ) {
        EsploraTFT.text( ">", 5, 110 );
    }


    EsploraTFT.stroke( 255, 255, 255 );
    EsploraTFT.setTextSize( 1 );
    if ( NEXT_STATE == HOME_SEL_BEGIN_GAME ) {
        EsploraTFT.text( ">", 10, 25 );
    } else if ( NEXT_STATE == HOME_SEL_SHOW_BEST_SCORE ) {
        EsploraTFT.text( ">", 10, 35 );
    } else if ( NEXT_STATE == END_SEL_ENTER_NAME ) {
        EsploraTFT.text( ">", 95, 110 );
    } else if ( NEXT_STATE == END_SEL_HOME_MENU ) {
        EsploraTFT.text( ">", 5, 110 );
    }
}

/**
 * @brief Draws home menu
 */
void display_home_menu() {
    EsploraTFT.background( 0, 0, 0 );

    EsploraTFT.stroke( 255, 255, 255 );
    EsploraTFT.setTextSize( 1 );
    EsploraTFT.text( "BreakOut\n ", 5, 5 );
    EsploraTFT.setTextSize( 1 );
    EsploraTFT.text( "Begin Game\n ", 20, 25 );
    EsploraTFT.text( "Show Score\n ", 20, 35 );
//    EsploraTFT.text( "Show Controls\n ", 20, 45 );
}

/**
 * @brief Draws end menu
 */
void display_end_menu() {
    EsploraTFT.setTextSize( 1 );
    EsploraTFT.stroke( 255, 255, 255 );
    EsploraTFT.fill( 255, 0, 0 );//useless
    EsploraTFT.text( "PLAY AGAIN\n", 10, 110 );
    EsploraTFT.text( "ENTER NAME\n", 100, 110 );
}

/**
 * @brief Detects button push
 */
bool buttonEvent( int button ) {
    switch (button) {
        case UP:
            if ( Esplora.readButton( UP ) == LOW ) {
                buttonFlag |= 1;
            } else if ( buttonFlag & 1 ) {
                buttonFlag ^= 1;
                return true;
            }
            break;

        case DOWN:
            if ( Esplora.readButton( DOWN ) == LOW ) {
                buttonFlag |= 2;
            } else if ( buttonFlag & 2 ) {
                buttonFlag ^= 2;
                return true;
            }
            break;

        case BACK:
            if ( Esplora.readButton( BACK ) == LOW ) {
                buttonFlag |= 4;
            } else if ( buttonFlag & 4 ) {
                buttonFlag ^= 4;
                return true;
            }
            break;

        case ENTER:
            if ( Esplora.readButton( ENTER ) == LOW ) {
                buttonFlag |= 8;
            } else if ( buttonFlag & 8 ) {
                buttonFlag ^= 8;
                return true;
            }
    }
    return false;
}

/**
 * @brief Draws top  3 players and their score
 */
void draw_bestScore() {
    EsploraTFT.background( 0, 0, 0 );
    EsploraTFT.stroke( 255, 255, 255 );
    EsploraTFT.setTextSize( 2 );
    EsploraTFT.text( "Best Scores: \n ", 5, 5 );
    EsploraTFT.setTextSize( 1 );
    for ( int i = 0; i < 3; ++i ) {
        itoa( i, stringPos, 10 );

        EsploraTFT.text( stringPos, 5, 30 + i * 10 );
        EsploraTFT.text( ".", 10, 30 + i * 10 );

        itoa( bestScore[i], stringBestScore[i], 10 );
        EsploraTFT.text( stringBestScore[i], 90, 30 + i * 10 );

        for ( int j = 0; j < 3; ++j ) {
            EsploraTFT.text( bestPlayers[i][j], 30 + j * 6, 30 + i * 10 );
        }

    }

}

/**
 * @brief reads palyer name
 */
void read_name() {
    EsploraTFT.background( 0, 0, 0 );
    EsploraTFT.fill( 255, 0, 0 );//useless

    EsploraTFT.stroke( 255, 255, 255 );
    EsploraTFT.setTextSize( 2 );

    EsploraTFT.text( "ENTER NAME: \n ", 15, 5 );


    for ( int i = 0; i < 3; ++i ) {
        EsploraTFT.text( p_name[i], 55 + i * 20, 50 );
    }
    EsploraTFT.text( "-", 55 + char_pos * 20, 62 );
}

/**
 * @brief saves player name and overwrites top players if palyer is better
 */
void save_name() {
    if ( score > bestScore[0] ) {
        overwrite_score( bestPlayers[2], bestPlayers[1], &bestScore[2], bestScore[1] );
        overwrite_score( bestPlayers[1], bestPlayers[0], &bestScore[1], bestScore[0] );
        overwrite_score( bestPlayers[0], p_name, &bestScore[0], score );

    } else if ( score > bestScore[1] ) {
        overwrite_score( bestPlayers[2], bestPlayers[1], &bestScore[2], bestScore[1] );
        overwrite_score( bestPlayers[1], p_name, &bestScore[1], score );

    } else if ( score > bestScore[2] ) {
        overwrite_score( bestPlayers[2], p_name, &bestScore[2], score );
    }
}

/**
 * @brief overwrite one player and score by anothers
 */
void overwrite_score( char p1_name[3][2], char p2_name[3][2], int *p1_score, int p2_score ) {
    *p1_score = p2_score;
    p1_name[0][0] = p2_name[0][0];
    p1_name[1][0] = p2_name[1][0];
    p1_name[2][0] = p2_name[2][0];
}

//-----------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initialize Arduino
 *
 */
void setup() {
    Serial.begin( 9600 );
    EsploraTFT.begin();
    EsploraTFT.background( 0, 0, 0 );

    setBrickColors();
    reset_game();

    STATE = HOME_SEL_BEGIN_GAME;
    NEXT_STATE = HOME_SEL_BEGIN_GAME;

    display_home_menu();
    change_position();


}

/**
 * @brief Reppeating code
 *
 */
void loop() {
    switch (STATE) {
        case HOME_SEL_BEGIN_GAME:

            if ( buttonEvent( ENTER )) {
                EsploraTFT.background( 0, 0, 0 );
                reset_game();
                reset_level();
                wait_for_player();
                NEXT_STATE = IN_GAME;
            } else if ( buttonEvent( DOWN )) {
                NEXT_STATE = HOME_SEL_SHOW_BEST_SCORE;
                change_position();
            }
            break;

        case HOME_SEL_SHOW_BEST_SCORE:

            if ( buttonEvent( UP )) {
                NEXT_STATE = HOME_SEL_BEGIN_GAME;
                change_position();
            } else if ( buttonEvent( ENTER )) {
                NEXT_STATE = SHOW_BEST_SCORE;
                draw_bestScore();
            }
            break;

        case IN_GAME:

            draw_game();
            break;

        case SHOW_BEST_SCORE:
            if ( buttonEvent( BACK ) || buttonEvent( ENTER ) || buttonEvent( UP ) || buttonEvent( DOWN )) {
                NEXT_STATE = HOME_SEL_SHOW_BEST_SCORE;
                EsploraTFT.background( 0, 0, 0 );
                display_home_menu();
                change_position();
            }
            break;
        case END_GAME:

            EsploraTFT.background( 0, 0, 0 );
            EsploraTFT.stroke( 255, 255, 255 );
            EsploraTFT.setTextSize( 3 );
            EsploraTFT.text( "GAME OVER\n ", 1, 15 );
            EsploraTFT.setTextSize( 2 );
            EsploraTFT.text( "YOUR SCORE:\n", 20, 45 );
            score += lives * 50; //bonus for leftoverlives
            itoa( score, stringScore, 10 );
            EsploraTFT.text( stringScore, 75, 70 );
            //----------------------------------------------------------
            NEXT_STATE = END_SEL_HOME_MENU;
            display_end_menu();
            change_position();
            break;

        case END_SEL_ENTER_NAME:
            if ( buttonEvent( ENTER ) || buttonEvent( BACK )) {
                NEXT_STATE = END_SEL_HOME_MENU;
                change_position();
            } else if ( buttonEvent( UP ) || buttonEvent( DOWN )) {
                EsploraTFT.background( 0, 0, 0 );
                char_pos = 0;
                read_name();
                NEXT_STATE = ENTER_NAME;
            }
            break;
        case END_SEL_HOME_MENU:
            if ( buttonEvent( ENTER ) || buttonEvent( BACK )) {
                NEXT_STATE = END_SEL_ENTER_NAME;
                change_position();
            } else if ( buttonEvent( UP ) || buttonEvent( DOWN )) {
                NEXT_STATE = HOME_SEL_BEGIN_GAME;
                EsploraTFT.background( 0, 0, 0 );
                display_home_menu();
                change_position();
            }
            break;
        case ENTER_NAME:
            if ( buttonEvent( ENTER )) {
                char_pos++;
                if ( char_pos > 2 ) {
                    save_name();

                    NEXT_STATE = HOME_SEL_BEGIN_GAME;
                    EsploraTFT.background( 0, 0, 0 );
                    display_home_menu();
                    change_position();
                } else {
                    read_name();
                }
            } else if ( buttonEvent( BACK )) {
                char_pos--;
                if ( char_pos < 0 )
                    char_pos = 0;
                read_name();
            } else if ( buttonEvent( UP )) {
                p_name[char_pos][0] = (65 + ((-65 + p_name[char_pos][0] + 1) % 26));
                read_name();
            } else if ( buttonEvent( DOWN )) {
                p_name[char_pos][0] = (p_name[char_pos][0] == 'A') ? 'Z' : 65 + abs((-65 + p_name[char_pos][0] - 1) % 26 );
                read_name();
            }
            break;

    }
    STATE = NEXT_STATE;
}


