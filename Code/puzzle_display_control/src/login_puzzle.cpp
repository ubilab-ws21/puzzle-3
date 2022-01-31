#include "encoder_handle.h"
#include "display_stuff.h"
#include "login_puzzle.h"


#include "Fonts/FreeSerifBoldItalic9pt7b.h"

#define LETTER_OFFSET 10



#define STEP_SIZE 25





#define NUM_SECTIONS NUM_SECTIONS_X*NUM_SECTIONS_Y

#define SCREEN_SIZE_X 800
#define SCREEN_SIZE_Y 480


char letters[18];

int orig_valid_sections[18] = {1,0,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1};
int valid_sections[18] = {};



enum{
    NONE_SECTION = 0,
    SHOW_SECTION,
    RESET_SECTION, 
    LETTER_SECTION
};


int free_placeholer_num = 0;
char placeholder_letters[6] = {};

tsPoint_t letter;
tsPoint_t calibrated;
tsPoint_t midpoint;

int num_rect_visible;
int rect_selected_num;

rect rectangles;

rect letter_place[6];



bool letters_visible = false;

static int function_called_count = 0;

bool removed = false;



void init_rect()
{
    static tsMatrix_t local_matrix = gettsMatrix();
    static Adafruit_RA8875 local_tft = gettft();

    local_tft.fillScreen(BACKGROUND_COLOR);

    rectangles.tr_corner.x = 0;
    rectangles.tr_corner.y =  REC_MIN_Y;
    rectangles.section = 0;
    rectangles.color = RA8875_BLUE;

    calibrateTSPoint(&rectangles.tr_corner_cal, &rectangles.tr_corner, &local_matrix);
    Serial.println(rectangles.tr_corner_cal.x);
    Serial.println(rectangles.tr_corner_cal.y);

    local_tft.fillRect(rectangles.tr_corner.x,rectangles.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, rectangles.color);

    num_rect_visible = 1;
    rect_selected_num = 0;

    // initialize letter placeholder rectangles: Here the Letter rectangles must be placed in order to solve the puzzle.
    int x, y;

    int i;
    
    char password[7] = "PASSWD";

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

    delay(500);
    fill_display(BACKGROUND_COLOR);

    for(i = 0; i<6; i++)
    {
        /*
        local_tft.drawRect(letter_place[i].tr_corner.x+10, letter_place[i].tr_corner.y, REC_SIZE_X-15, REC_SIZE_Y-30, letter_place[i].color);
        */
        local_tft.fillRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, RA8875_YELLOW);
        local_tft.drawRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, letter_place[i].color);
        local_tft.drawChar(letter_place[i].tr_corner.x+40, letter_place[i].tr_corner.y+40, password[i], RA8875_RED, RA8875_YELLOW, 8);
        placeholder_letters[i] = {};
    }

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
    local_tft.graphicsMode();
    delay(1000);

    for(i = 0; i<6; i++)
    {
        /*
        local_tft.drawRect(letter_place[i].tr_corner.x+10, letter_place[i].tr_corner.y, REC_SIZE_X-15, REC_SIZE_Y-30, letter_place[i].color);
        */
        local_tft.fillRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, RA8875_YELLOW);
        local_tft.drawRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, letter_place[i].color);
    }

    //draw_showbutton(0);
    draw_reset_button();

    local_tft.fillRect(rectangles.tr_corner.x, rectangles.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, rectangles.color);
    
    free_placeholer_num = 0;

    for(i = 0; i<18; i++)
    {
        valid_sections[i] = orig_valid_sections[i];
    }

    random_letter_generation(true);

    // show all letters
    blink_section_letters(false); 
    removed = false;
}

void draw_placeholders()
{
    static Adafruit_RA8875 local_tft = gettft();

    for(int i = 0; i<6; i++)
    {
        local_tft.fillRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, RA8875_YELLOW);
        local_tft.drawRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, letter_place[i].color);
    }
}

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

bool rectangles_game(tsPoint_t touch_raw)
{
    static Adafruit_RA8875 local_tft = gettft();
    static tsMatrix_t local_matrix = gettsMatrix();

    int x = rectangles.tr_corner.x;
    int y = rectangles.tr_corner.y;
    int rec_section = rectangles.section;
    int color = rectangles.color;

    // if last action was an encoder action, i.e. either of the two first encoders was used:
    int last_action = get_last_action();
    if(last_action >= 1 && last_action <= 2)
    {   
        if(!removed)
        {
            remove_section_letters();
            removed = true; 
        }
        
        Serial.print("last action = ");
        Serial.println(last_action);
        // the last action matches the current encode number. 
        int cur_encoder_num = last_action;

        int sign; 
        if(encoder_get_value(cur_encoder_num) < 0)
            sign = -1;
        else
            sign = 1;

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
                if(((rec_section+1) % NUM_SECTIONS_X) && (rec_section+1 < NUM_SECTIONS))
                {
                    Serial.println("one to the rightbefore: ");
                    Serial.println(x);
                    x += REC_SIZE_X;
                    rec_section += 1;
                    rectangles.tr_corner.x = x;
                    rectangles.section = rec_section;
                }
                else if(!(rec_section+1) % NUM_SECTIONS_X)
                {
                    // no nothing. rectangle should stay where it is
                }
                else {
                    Serial.println("else loop");
                }
            }
            else if(cur_encoder_num == 2)
            {
                if((y + 2 * REC_SIZE_Y <= SCREEN_SIZE_Y) && (rec_section+6 < NUM_SECTIONS))
                {
                    Serial.println("down");
                    y+= REC_SIZE_Y;
                    rec_section += 6;
                    rectangles.tr_corner.y = y;
                    rectangles.section = rec_section;
                }
                else if(!(rec_section+6 < NUM_SECTIONS))
                {
                    // no nothing. Rectangle should stay where it is
                }
                else {
                    Serial.println("else loop");
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
                        rectangles.tr_corner.x = x;
                        rectangles.section = rec_section;
                    }
                    else
                    {

                    }
                }
                else if(cur_encoder_num == 2)
                {
                    if(( y - REC_SIZE_Y >= 0))
                    {
                        y -= REC_SIZE_Y;
                        rec_section -= 6;
                        rectangles.tr_corner.y = y;
                        rectangles.tr_corner.x = x;
                        rectangles.section = rec_section;                        
                    }
                }
        }

        encoder_set_value(cur_encoder_num, 0);

        local_tft.fillRect(rectangles.tr_corner.x, rectangles.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, rectangles.color);

        if(rectangles.section >= 5)
        {
            draw_placeholder_letters();
        }
        last_action = last_action_none;
    }
    else if(last_action == last_action_touch)
    {
        if(!removed)
        {
            remove_section_letters();
            removed = true; 
        }

        tsPoint_t calibrated;
        //Calcuate the real X/Y position based on the calibration matrix 
        calibrateTSPoint(&calibrated, &touch_raw, &local_matrix);

        int touch_space = what_was_touched(calibrated.x, calibrated.y);
        switch(touch_space)
        {
        case NONE_SECTION:
            break;
        /*case SHOW_SECTION:
            if(random_letter_generation(false))
            {
                //show letters. 
                blink_section_letters();
            }
            break;
        */
        case RESET_SECTION:
            // this will result in resetting the game
            function_called_count = 5;
            break;
        case LETTER_SECTION:
            int section = get_section(calibrated.x, calibrated.y);

            if(rectangles.section == section)
            {
                Serial.println("section");
                Serial.println(section);
                x = rectangles.tr_corner.x;
                y = rectangles.tr_corner.y;
                color = rectangles.color;
                section = rectangles.section;

                if(section < 18)
                {
                    int placeholder_x, placeholder_y;
                    section_to_xy(free_placeholer_num+18, &placeholder_x, &placeholder_y);

                    //local_tft.fillRect(rectangles.tr_corner.x, rectangles.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, BACKGROUND_COLOR);
                    /*
                    rectangles.tr_corner.x = placeholder_x;
                    rectangles.tr_corner.y = placeholder_y;
                    rectangles.color = color;
                    rectangles.section = free_placeholer_num+18;*/
                    

                    local_tft.fillRect(rectangles.tr_corner.x, rectangles.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, rectangles.color);
                    
                    if(is_section_with_letter(section))
                    {
                        char mychar = get_letter(section);
                        shift_letter(section);
                        Serial.println(mychar);
                        local_tft.drawChar(placeholder_x + 50, placeholder_y + 50, mychar, RA8875_BLACK, RA8875_YELLOW , 5);
                    }
                }

            }
            break;
        }
        last_action = last_action_none;
    }
    if(free_placeholer_num == 6 || function_called_count > 4)
    {
        if(is_rect_puzzle_solved())
        {
            return 1;
        }
        else
        {
            init_rect();
        }
    }
    //puzzle not solved
    return 0;
}

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

int get_section(int x, int y)
{
    int section = ((y-REC_MIN_Y)/REC_SIZE_Y)*NUM_SECTIONS_X + (x/REC_SIZE_X);
    return section;
}


bool is_section_with_letter(int section)
{
    if(valid_sections[section] == true)
        return true;
    return false;
}

bool random_letter_generation(bool init)
{
    if(init)
    {
        function_called_count=0;    
    }
    else
    {
        function_called_count++;
        draw_showbutton(function_called_count);
        if(function_called_count > 4)
        {
            return false; 
        }
    }

    Serial.println("counter = " +  String(function_called_count));
    static int random_start;
    char manual[7] = "MANUAL";
    if(!(function_called_count))
    {

        //function_called_count = 0;
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
    }

    for(int i = 0; i<18; i++)
    {
        Serial.print(letters[i]);
    }
    
    return true;
}

char get_letter(int section)
{
    return letters[section];
}

void remove_section_letters()
{
    static Adafruit_RA8875 local_tft = gettft();
    int section; 
    bool valid_section;
    int letter_bg_color;
    int char_x, char_y;
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
                    letter_color = rectangles.color;
                    letter_bg_color = rectangles.color;
                }
                
                section_to_xy(section, &char_x, &char_y);
                local_tft.drawChar(char_x + 50, char_y + 50, mychar, letter_color, letter_bg_color, 5);
            }
        }
        letters_visible = false;
}

void blink_section_letters(bool blink)
{
    static Adafruit_RA8875 local_tft = gettft();
    int section; 
    int letter_bg_color;
    int char_x, char_y;
    bool valid_section;
    bool was_visible = letters_visible;
    if(!letters_visible)
    {
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
                    letter_bg_color = rectangles.color;
                }
            section_to_xy(section, &char_x, &char_y);
            local_tft.drawChar(char_x+50, char_y+50, mychar, RA8875_BLACK, letter_bg_color , 5);
            }
        }
        letters_visible = true;
    }
    if(blink || was_visible)
    {
        if(blink)
            delay(1500);
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
                    letter_color = rectangles.color;
                    letter_bg_color = rectangles.color;
                }
                
                section_to_xy(section, &char_x, &char_y);
                local_tft.drawChar(char_x + 50, char_y + 50, mychar, letter_color, letter_bg_color, 5);
            }
        }
        letters_visible = false;
    }
}

void draw_placeholder_letters()
{
    static Adafruit_RA8875 local_tft = gettft();
    int i;
    for(i=0; i<free_placeholer_num; i++)
    {
        int placeholder_x, placeholder_y;
        section_to_xy(i+18, &placeholder_x, &placeholder_y);
        int bg_color;
        if(is_rect_here(i+18))
            bg_color = rectangles.color;
        else 
            bg_color = RA8875_YELLOW;
        local_tft.drawChar(placeholder_x + 50, placeholder_y +50, placeholder_letters[i], RA8875_BLACK, bg_color, 5);
    }
}

bool is_rect_here(int section)
{
    if(rectangles.section == section)
    {
        return true; 
    }
    return false;
}

void shift_letter(int cur_section)
{
    if(is_section_with_letter(cur_section))
    {
        char letter = get_letter(cur_section);
        valid_sections[cur_section] = 0;
        placeholder_letters[free_placeholer_num] = letter;
        free_placeholer_num++;
    }
}

int position_occupied(int section)
{
    Serial.println(rect_selected_num);

    if(rectangles.section == section)
    {
        Serial.print("next position occuppied. section: ");
        return 1; 
    }
                
    Serial.println("next position not occupied");
    return 0;
}

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
