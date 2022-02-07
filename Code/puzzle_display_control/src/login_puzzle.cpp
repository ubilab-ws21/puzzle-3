#include "encoder_handle.h"
#include "display_stuff.h"
#include "login_puzzle.h"


#include "Fonts/FreeSerifBoldItalic9pt7b.h"

// important defines
#define LETTER_OFFSET 10

#define STEP_SIZE 25

#define NUM_SECTIONS NUM_SECTIONS_X*NUM_SECTIONS_Y

#define SCREEN_SIZE_X 800
#define SCREEN_SIZE_Y 480

// letters array and their positions
char letters[18];
int orig_valid_sections[18] = {0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1};
int valid_sections[18] = {};

// possible sections for touch events
enum{
    NONE_SECTION = 0,
    SHOW_SECTION,
    RESET_SECTION, 
    LETTER_SECTION
};

// array for letters in the placeholder area (the area where to enter the letters for the password)
// at first no letter is in this area, indicated by "-----"
char placeholder_letters[7] = "------";
// the 6 placeholder rectangles that are permanently displayed to indicate where to enter to password
rect letter_place[6];

// The two different rectangles that can be moved on the screen
rect game_rect;
rect cursor_rect;

// whether reset button was pressed.
bool reset_pressed = false;

bool removed = true;

// default frequency is 1332 MHz. This value will be changed as soon as the game starts:
// Then, these values will be replaced by the saved frequency from the previous game.
static char freq_pre_comma[5] = "133";
static char freq_post_comma[2] = "2";

/**********************************************************************************************************
* This function initializes the rectangle login game. It displays an introduction to the game, draws the 
* permanently given information on top of the screen, starts random number generation and displays the 
* letters on the screen.
**********************************************************************************************************/ 
void init_rect()
{
    static Adafruit_RA8875 local_tft = gettft();

    local_tft.fillScreen(BACKGROUND_COLOR);

    // first game rectangle position is in section 0, which is the top left section.
    game_rect.tr_corner.x = 0;
    game_rect.tr_corner.y =  REC_MIN_Y;
    game_rect.section = 0;
    game_rect.color = RA8875_BLUE;

    // first cursor rectangle position is in section 18, which is the bottom left section (placeholder area)
    cursor_rect.section = 18;
    section_to_xy(cursor_rect.section, &cursor_rect.tr_corner.x, &cursor_rect.tr_corner.y);
    cursor_rect.color = RA8875_RED;

    // initialize letter placeholder area: Here the password "manual" must be placed in order to solve the puzzle.
    int x, y;
    int i;
    // display "PASSWD" in the placeholder area to indicate that here the password has to be entered.
    char password[7] = "PASSWD";

    // blink PASSWD 3 times. 
    for(int j = 0; j<3; j++)
    {
        fill_display(RA8875_BLACK);
        delay(500);
        for(i = 0; i<6; i++)
        {
            section_to_xy(18+i, &x, &y);
            Serial.print("x value: " + String(x));
            Serial.print("y value: " + String(y));
            letter_place[i].tr_corner.x = x;
            letter_place[i].tr_corner.y =  y;
            letter_place[i].section = 18+i;
            letter_place[i].color = RA8875_BLUE;
            local_tft.drawChar(letter_place[i].tr_corner.x+40, letter_place[i].tr_corner.y+40, password[i], RA8875_RED, RA8875_BLACK, 8);
        }
        delay(500);
    }

    delay(250);

    // now initialize the proper display. 
    fill_display(BACKGROUND_COLOR);

    // #TODO: duplicaate?
    for(i = 0; i<6; i++)
    {
        local_tft.fillRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, RA8875_YELLOW);
        local_tft.drawRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, letter_place[i].color);
        local_tft.drawChar(letter_place[i].tr_corner.x+40, letter_place[i].tr_corner.y+40, password[i], RA8875_RED, RA8875_YELLOW, 8);
    }

    // print information on top of the screen. 
    local_tft.drawFastHLine(0, REC_MIN_Y-1, SCREEN_SIZE_X, RA8875_BLUE);
    local_tft.textMode();
    local_tft.textTransparent(RA8875_RED);
    local_tft.textEnlarge(1);
    local_tft.textSetCursor(280, 10);
    local_tft.textWrite("Ferdi's");
    local_tft.textSetCursor(420, 10);
    local_tft.textWrite("Login");
    local_tft.textSetCursor(350, 40);
    local_tft.textWrite("Space");

    // print the current radio broadcast frequency on the top left of the screen
    char string1[5];
    int freq = get_solved_frequency();
    itoa(freq, string1, 10);
    strcpy(freq_pre_comma, string1);
    if(freq >= 1000)
    {
        strcpy(freq_post_comma, string1+3);
        freq_pre_comma[3] = '\0';
    }
    else{
        strcpy(freq_post_comma, string1+2);
        freq_pre_comma[2] = '\0';
    }

    local_tft.textSetCursor(25, 10);
    local_tft.textTransparent(RA8875_BLUE);
    local_tft.textEnlarge(0);

    local_tft.textWrite("Choosen Frequency:");

    local_tft.textTransparent(RA8875_BLACK);
    local_tft.textEnlarge(1);
    local_tft.textSetCursor(50, 25);
    local_tft.textWrite(freq_pre_comma);

    local_tft.textSetCursor(100, 25);
    local_tft.textWrite(",");
    local_tft.textSetCursor(120, 25);
    local_tft.textWrite(freq_post_comma);

    local_tft.textSetCursor(160, 25);
    local_tft.textWrite("MHz");

    local_tft.textEnlarge(0);
    local_tft.textSetCursor(25, 60);
    local_tft.textWrite("Private Radio Broadcast");

    local_tft.graphicsMode();
    delay(1000);

    // draw all 6 letter placeholders (password area)
    draw_placeholders();

    // draw the reset button on the top right of the screen.
    draw_reset_button();

    // draw the game rectangle on the first section.
    local_tft.fillRect(game_rect.tr_corner.x, game_rect.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, game_rect.color);

    for(i = 0; i<18; i++)
    {
        valid_sections[i] = orig_valid_sections[i];
    }

    random_letter_generation(true);
    reset_pressed = false;

    // show all letters
    blink_section_letters(false); 
    removed = false;

    local_tft.drawRect(cursor_rect.tr_corner.x+5, cursor_rect.tr_corner.y+5, REC_SIZE_X-10, REC_SIZE_Y-10, cursor_rect.color);

    char repl[] = {"-"};
    for(int i= 0; i<6; i++)
    {
        placeholder_letters[i] = repl[0];
    }
  }

/******************************************************************************************************
* This function draws the six placeholders where the password needs to be entered.
******************************************************************************************************/
void draw_placeholders()
{
    static Adafruit_RA8875 local_tft = gettft();

    for(int i = 0; i<6; i++)
    {
        local_tft.fillRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, RA8875_YELLOW);
        local_tft.drawRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, letter_place[i].color);

        local_tft.drawRect(cursor_rect.tr_corner.x+5, cursor_rect.tr_corner.y+5, REC_SIZE_X-10, REC_SIZE_Y-10, cursor_rect.color);
    }
}

/******************************************************************************************************
* This function draws the showbutton. Not used now
******************************************************************************************************/
void draw_showbutton(int counter)
{
    static Adafruit_RA8875 local_tft = gettft();

    char string_buffer[3];;

    itoa(counter, string_buffer, 10);

    local_tft.fillRect(0, 0, REC_SIZE_X*2, REC_MIN_Y, RA8875_GREEN);
    local_tft.drawRect(0, 0, REC_SIZE_X*2, REC_MIN_Y, 0x07D0);

    local_tft.fillRect(REC_SIZE_X*4, 0, REC_SIZE_X*2, REC_MIN_Y, RA8875_GREEN);
    local_tft.drawRect(REC_SIZE_X*4, 0, REC_SIZE_X*2, REC_MIN_Y, 0x07D0);

    local_tft.textMode();
    local_tft.textTransparent(RA8875_RED);
    local_tft.textEnlarge(2);
    local_tft.textSetCursor(80, 20);
    local_tft.textWrite(string_buffer);
    local_tft.textSetCursor(120, 20);
    local_tft.textWrite("/ 4");
    local_tft.textSetCursor(600, 20);
    local_tft.textWrite("RESET");
    local_tft.graphicsMode();
}

/******************************************************************************************************
* This function draws the reset button.
******************************************************************************************************/
void draw_reset_button()
{
    static Adafruit_RA8875 local_tft = gettft();

    local_tft.fillRect(REC_SIZE_X*4, 0, REC_SIZE_X*2, REC_MIN_Y, RA8875_GREEN);
    local_tft.textMode();

    local_tft.textSetCursor(600, 20);
    local_tft.textTransparent(RA8875_RED);
    local_tft.textEnlarge(2);
    local_tft.textWrite("RESET");
    local_tft.graphicsMode();
}
/******************************************************************************************************
* This function is the main function for the login game. The user is required to place the letters
* of the password ("manual") into the password space on the bottom.
******************************************************************************************************/
bool login_game(tsPoint_t touch_raw)
{
    static Adafruit_RA8875 local_tft = gettft();
    static tsMatrix_t local_matrix = gettsMatrix();

    // if last action was an encoder action, i.e. either of the two first encoders was used:
    int last_action = get_last_action();
    // the last action matches the current encode number. 
    int cur_encoder_num = last_action;

    int x;
    int y;
    int rec_section;
    int color;
    switch (last_action)
    {
    case last_action_encoder1:
    case last_action_encoder2:
    case last_action_encoder3:

        if(!removed)
        {
            // remove letters if they have not been removed yet
            Serial.print("Calling from enc 123");
            remove_section_letters();
            removed = true; 
        }
        
        // check direction of encoder movement.
        int sign; 
        if(encoder_get_value(cur_encoder_num) < 0)
            sign = -1;
        else
            sign = 1;

        if(last_action == last_action_encoder1 || last_action == last_action_encoder2)
        {
            x = game_rect.tr_corner.x;
            y = game_rect.tr_corner.y;
            rec_section = game_rect.section;
            color = game_rect.color;
            // remove current position of the selected rectangle
            local_tft.fillRect(x, y, REC_SIZE_X, REC_SIZE_Y, BACKGROUND_COLOR);
            if(rec_section >= 12)
            {
                draw_placeholders();
            }
            // depending on the rotation direction, the rectangle is moved up 
            // or down (in case encoder 2 was rotated), or left and right (in 
            // case encoder 1 was rotated)
            if(sign == 1)
            {
                Serial.println((rec_section+1) % NUM_SECTIONS_X);
                Serial.println(rec_section+1);
                if(cur_encoder_num == 1)
                {
                    if(((rec_section+1) % NUM_SECTIONS_X))
                    {
                        Serial.println("one to the rightbefore: ");
                        Serial.println(x);
                        x += REC_SIZE_X;
                        rec_section += 1;
                        game_rect.tr_corner.x = x;
                        game_rect.section = rec_section;
                    }
                }
                else if(cur_encoder_num == 2)
                {
                    if((y + 2 * REC_SIZE_Y <= SCREEN_SIZE_Y-REC_SIZE_Y))
                    {
                        Serial.println("down");
                        y+= REC_SIZE_Y;
                        rec_section += 6;
                        game_rect.tr_corner.y = y;
                        game_rect.section = rec_section;
                    }
                }
            }
            else if (sign == -1)
            {
                if(cur_encoder_num == 1)
                    {
                        if(( x - REC_SIZE_X >= 0))
                        {
                            x -= REC_SIZE_X;
                            rec_section -= 1;
                            game_rect.tr_corner.x = x;
                            game_rect.section = rec_section;
                        }
                    }
                    else if(cur_encoder_num == 2)
                    {
                        if(( y - REC_SIZE_Y >= 0))
                        {
                            y -= REC_SIZE_Y;
                            rec_section -= 6;
                            game_rect.tr_corner.y = y;
                            game_rect.tr_corner.x = x;
                            game_rect.section = rec_section;                        
                        }
                    }
            }
            // print new rectangle
            local_tft.fillRect(x, y, REC_SIZE_X, REC_SIZE_Y, color);

            if(rec_section >= 6)
            {
                // make sure that the placeholders have not been removed.
                draw_placeholder_letters();
            }
        }
        else
        {
            // encoder no 3
            x = cursor_rect.tr_corner.x;
            y = cursor_rect.tr_corner.y;
            rec_section = cursor_rect.section;
            color = cursor_rect.color;

            // check encoder movement direction.
            if(sign == 1)
            {
                Serial.println((rec_section+1) % NUM_SECTIONS_X);
                Serial.println(rec_section+1);

                if(((rec_section+1) % NUM_SECTIONS_X))
                {
                    int next_emtpy_rec_section = find_next_free_placeholder(true, rec_section);
                    if(next_emtpy_rec_section != -1)
                    {
                        section_to_xy(next_emtpy_rec_section, &x, &y);
                        rec_section = next_emtpy_rec_section;
                        cursor_rect.tr_corner.x = x;
                        cursor_rect.section = rec_section;
                    }
                }
            }
            else if(sign == -1)
            {
                
                if(( x - REC_SIZE_X >= 0))
                {
                    Serial.println(" turning left");
                    int next_emtpy_rec_section = find_next_free_placeholder(false, rec_section);
                    if(next_emtpy_rec_section != -1)
                    {
                        
                        section_to_xy(next_emtpy_rec_section, &x, &y);
                        rec_section = next_emtpy_rec_section;
                        
                        Serial.print("afterwards");
                        Serial.print(x);
                        Serial.print("rec section");
                        Serial.println(rec_section);
                        cursor_rect.tr_corner.x = x;
                        cursor_rect.section = rec_section;
                    }
                }
            }

            // draw placeholders (password area) and letters in there
            draw_placeholders();
            draw_placeholder_letters();
            local_tft.drawRect(cursor_rect.tr_corner.x+5, cursor_rect.tr_corner.y+5, REC_SIZE_X-10, REC_SIZE_Y-10, cursor_rect.color);
        }

        // reset encoder value to 0
        encoder_set_value(cur_encoder_num, 0);
        last_action = last_action_none;

        break;
    case last_action_touch:
        // a touch event was triggered.

        tsPoint_t calibrated;
        //Calcuate the real X/Y position based on the calibration matrix 
        calibrateTSPoint(&calibrated, &touch_raw, &local_matrix);

        int touch_space = what_was_touched(calibrated.x, calibrated.y);
        switch(touch_space)
        {
        case NONE_SECTION:
            break;
        case RESET_SECTION:
            // this will result in resetting the game
            reset_pressed = true;
            break;
        case LETTER_SECTION:
            // a letter section has been touched. 
            x = game_rect.tr_corner.x;
            y = game_rect.tr_corner.y;
            color = game_rect.color;
            int section = game_rect.section;

            int placeholder_x, placeholder_y;
            section_to_xy(cursor_rect.section, &placeholder_x, &placeholder_y);                

            local_tft.fillRect(game_rect.tr_corner.x, game_rect.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, game_rect.color);
            
            
            if(is_section_with_letter(section))
            {
                // if section is a section containing a letter, shift this letter to the current cursor position in the password space
                char mychar = get_letter(section);
                shift_letter(section, cursor_rect.section);
                Serial.println(mychar);
                local_tft.drawChar(placeholder_x + 50, placeholder_y + 50, mychar, RA8875_BLACK, RA8875_YELLOW , 5);
                rec_section = cursor_rect.section;

                // try to find next free placeholder for cursor rectangle in the password area
                rec_section = find_next_free_placeholder(true, rec_section);
                if(rec_section != -1)
                {
                    // place cursor in this next free placeholder.
                    local_tft.drawRect(cursor_rect.tr_corner.x+5, cursor_rect.tr_corner.y+5, REC_SIZE_X-10, REC_SIZE_Y-10, RA8875_YELLOW);
                    cursor_rect.section = rec_section;
                    section_to_xy(rec_section, &cursor_rect.tr_corner.x, &cursor_rect.tr_corner.y);
                    local_tft.drawRect(cursor_rect.tr_corner.x+5, cursor_rect.tr_corner.y+5, REC_SIZE_X-10, REC_SIZE_Y-10, cursor_rect.color);
                }         
            }
            else
            {
                local_tft.fillRect(game_rect.tr_corner.x, game_rect.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, RA8875_RED);
                delay(200);
                local_tft.fillRect(game_rect.tr_corner.x, game_rect.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, game_rect.color);
            }

            break;
        }
        last_action = last_action_none;
        break;
    }
    
    // Do we need to check whether the game was successfully solved? This is the case when there is no free placeholder anymore in the password area.
    if(find_next_free_placeholder(true, 18) == -1 || reset_pressed)
    {
        if(is_rect_puzzle_solved() && !reset_pressed)
        {   
            // password entered correctly, puzzle solved.
            return 1;
        }
        else
        {   
            // worng password entered or if reset button was pressed.
            // Re-initialize the game.
            init_rect();
        }
    }    
    //puzzle not solved
    return 0;
}

/***********************************************************************
* This function checks which section was touched
***********************************************************************/
int what_was_touched(int x, int y)
{
    if(y < REC_MIN_Y)
    {
        if(x < 2*REC_SIZE_X)
            return SHOW_SECTION;
        else if(x > 4*REC_SIZE_X)
            return RESET_SECTION;
        else 
            return NONE_SECTION;
    }
    else
    {
        return LETTER_SECTION;
    }
}

/***********************************************************************
* This function converts x and y coordinates into the corresponding section 
***********************************************************************/
int get_section(int x, int y)
{
    int section = ((y-REC_MIN_Y)/REC_SIZE_Y)*NUM_SECTIONS_X + (x/REC_SIZE_X);
    return section;
}

/***********************************************************************
* This function checks whether the choosen section contains a letter.
* This is the case when it is a valid section where it has not been 
* shifted yet
***********************************************************************/
bool is_section_with_letter(int section)
{
    if(valid_sections[section] == true)
        return true;
    return false;
}

/***********************************************************************
* This function randomly creates letters for the Letter section and saves 
* them in the letter[] array which has 18 elements. 
***********************************************************************/
bool random_letter_generation(bool init)
{
    static int random_start;
    char manual[7] = "MANUAL";
    
    random_start = rand() % 6;
    Serial.println("random start: " + String(random_start));

    for(int i = 0; i<18; i++)
    {
        letters[i] = 'A' + (rand() % 26);
    }
    int counter = 0;
    for(int i = 0; i<18; i+=2)
    {
        while(!is_section_with_letter(i))
        {
            i++;
        }
        Serial.println("now: " + String((random_start + counter + 6) % 6));
        letters[i] = manual[(random_start + counter + 6) % 6];
        counter++;
        if(counter == 6)
            break;
    }

    for(int i = 0; i<18; i++)
    {
        Serial.print(letters[i]);
    }
    
    return true;
}

/***********************************************************************
* returns the letter in this section 
***********************************************************************/
char get_letter(int section)
{
    return letters[section];
}

/***********************************************************************
* This function makes all letters in the letter section invisible. Happens
* as soon as an encoder was triggered
***********************************************************************/
void remove_section_letters()
{
    static Adafruit_RA8875 local_tft = gettft();
    int section; 
    bool valid_section;
    int letter_bg_color;
    int char_x, char_y;

    Serial.print("REMOVE");
    for(section=0; section<18; section++)
    {   
        valid_section = is_section_with_letter(section);
        if(valid_section)
        {
            char mychar = get_letter(section);
            int letter_color; 
            if (!is_rect_here(section))
            {
                letter_color = BACKGROUND_COLOR;
                letter_bg_color = BACKGROUND_COLOR;
            }
            else
            {
                letter_color = game_rect.color;
                letter_bg_color = game_rect.color;
            }
            
            section_to_xy(section, &char_x, &char_y);
            local_tft.drawChar(char_x + 50, char_y + 50, mychar, letter_color, letter_bg_color, 5);
        }
    }
}

/***********************************************************************
* This function makes all letters in the letter section visible. 
* Happens in the beginning of a game
***********************************************************************/
void blink_section_letters(bool blink)
{
    static Adafruit_RA8875 local_tft = gettft();
    int section; 
    int letter_bg_color;
    int char_x, char_y;
    bool valid_section;
    Serial.println("BLINK");
    for(section=0; section<18; section++)
    {
        valid_section = is_section_with_letter(section);
        if(valid_section)
        {
            char mychar = get_letter(section);
            if (!is_rect_here(section))
            {
                letter_bg_color = BACKGROUND_COLOR;
            }
            else
            {
                letter_bg_color = game_rect.color;
            }
        section_to_xy(section, &char_x, &char_y);
        local_tft.drawChar(char_x+50, char_y+50, mychar, RA8875_BLACK, letter_bg_color , 5);
        }
    }
    removed = false;
    }/*
    if(blink || was_visible)
    {
        if(blink)
            delay(1500);

        Serial.print("blinking");
        for(section=0; section<18; section++)
        {   
            valid_section = is_section_with_letter(section);
            if(valid_section)
            {
                char mychar = get_letter(section);
                int letter_color; 
                if (!is_rect_here(section))
                {
                    letter_color = BACKGROUND_COLOR;
                    letter_bg_color = BACKGROUND_COLOR;
                }
                else
                {
                    letter_color = game_rect.color;
                    letter_bg_color = game_rect.color;
                }
                
                section_to_xy(section, &char_x, &char_y);
                local_tft.drawChar(char_x + 50, char_y + 50, mychar, letter_color, letter_bg_color, 5);
            }
        }
        removed = true;
        letters_visible = false;
    }
}*/

/***********************************************************************
* This function draws all currently active placeholder letters (letters in the
* password area)
***********************************************************************/
void draw_placeholder_letters()
{
    static Adafruit_RA8875 local_tft = gettft();
    int i;
    char repl[] = {"-"};
    for(i=0; i<6; i++)
    {  
        if(!(placeholder_letters[i] == repl[0]))
        {
            int placeholder_x, placeholder_y;
            section_to_xy(i+18, &placeholder_x, &placeholder_y);
            int bg_color;
            if(is_rect_here(i+18))
                bg_color = game_rect.color;
            else 
                bg_color = RA8875_YELLOW;
            local_tft.drawChar(placeholder_x + 50, placeholder_y +50, placeholder_letters[i], RA8875_BLACK, bg_color, 5);
        }
    }
}

/***********************************************************************
* checks if there is a rectangle in this section 
***********************************************************************/
bool is_rect_here(int section)
{
    if(game_rect.section == section)
    {
        return true; 
    }
    return false;
}

/***********************************************************************
* This function shifts the letter from the current rectangle section 
* to the current cursor section 
***********************************************************************/
void shift_letter(int cur_section, int cur_cursor_section)
{
    if(is_section_with_letter(cur_section))
    {
        char letter = get_letter(cur_section);
        valid_sections[cur_section] = 0;
        placeholder_letters[cur_cursor_section-18] = letter;
    }
}

/***********************************************************************
* This function finds the next free placeholder (i.e. not containing a letter
* yet), where the cursore can then be placed. It can be specified whether to 
* look for a free placeholder to the right or to the left of the current section 
***********************************************************************/
int find_next_free_placeholder(bool right, int rec_section)
{
    int skip_recs = 0;
    char repl[] = {"-"}; 
    bool free_place_found = false;
    if(right)
    {
        
        for(int iterate_rec_section=rec_section-18+1; iterate_rec_section<NUM_SECTIONS_X; iterate_rec_section++)
        {
            if(placeholder_letters[iterate_rec_section] == repl[0])
            {
                free_place_found = true;
                break;
            }
            skip_recs++;
        }
        if(free_place_found)
            return (rec_section + (1+ skip_recs));
        else
        {
            skip_recs = 0;
            for(int iterate_rec_section=18; iterate_rec_section<rec_section; iterate_rec_section++)
            {
                if(placeholder_letters[iterate_rec_section] == repl[0])
                {
                    free_place_found = true;
                    break;
                }
                skip_recs++;
            }
        }
        if(free_place_found)
            return (18+skip_recs);
    }
    else
    {    
        for(int iterate_rec_section=rec_section-18-1; iterate_rec_section>=0; iterate_rec_section--)
        {
            if(placeholder_letters[iterate_rec_section] == repl[0])
            {
                Serial.print("placeholder letters[ ");
                Serial.print(iterate_rec_section);
                Serial.print("] = ");
                Serial.println(placeholder_letters[iterate_rec_section]);
                free_place_found = true;
                break;
            }
            skip_recs++;
            Serial.print("skip recs ");
            Serial.println(skip_recs);
        }
        if(free_place_found)
            return (rec_section - (1+ skip_recs));
    }
    return -1;

}

/***********************************************************************
* This function checks whether the game was solved, i.e. if "MANUAL" was 
* entered to the password space
***********************************************************************/
bool is_rect_puzzle_solved()
{
    int i;
    char password[7] = "MANUAL";
    for(i=0; i<6; i++)
    {
        if(placeholder_letters[i] != password[i])
        {
            return false;
        }
    }
    return true;
}
