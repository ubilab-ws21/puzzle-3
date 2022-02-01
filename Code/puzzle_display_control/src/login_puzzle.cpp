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

int orig_valid_sections[18] = {0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1};
int valid_sections[18] = {};

enum{
    NONE_SECTION = 0,
    SHOW_SECTION,
    RESET_SECTION, 
    LETTER_SECTION
};


int free_placeholer_num = 0;
char placeholder_letters[7] = "------";

tsPoint_t letter;
tsPoint_t calibrated;
tsPoint_t midpoint;

int num_rect_visible;
int rect_selected_num;

rect game_rect;
rect cursor_rect;

rect letter_place[6];



bool letters_visible = false;

static int function_called_count = 0;

bool removed = true;



void init_rect()
{
    static tsMatrix_t local_matrix = gettsMatrix();
    static Adafruit_RA8875 local_tft = gettft();

    local_tft.fillScreen(BACKGROUND_COLOR);

    game_rect.tr_corner.x = 0;
    game_rect.tr_corner.y =  REC_MIN_Y;
    game_rect.section = 0;
    game_rect.color = RA8875_BLUE;

    cursor_rect.section = 18;
    section_to_xy(cursor_rect.section, &cursor_rect.tr_corner.x, &cursor_rect.tr_corner.y);
    cursor_rect.color = RA8875_RED;

    calibrateTSPoint(&game_rect.tr_corner_cal, &game_rect.tr_corner, &local_matrix);
    Serial.println(game_rect.tr_corner_cal.x);
    Serial.println(game_rect.tr_corner_cal.y);

    local_tft.fillRect(game_rect.tr_corner.x,game_rect.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, game_rect.color);

    num_rect_visible = 1;
    rect_selected_num = 0;

    // initialize letter placeholder game_rect: Here the Letter game_rect must be placed in order to solve the puzzle.
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

    local_tft.fillRect(game_rect.tr_corner.x, game_rect.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, game_rect.color);
    
    free_placeholer_num = 0;

    for(i = 0; i<18; i++)
    {
        valid_sections[i] = orig_valid_sections[i];
    }

    random_letter_generation(true);

    // show all letters
    blink_section_letters(false); 
    removed = false;

    local_tft.drawRect(cursor_rect.tr_corner.x+5, cursor_rect.tr_corner.y+5, REC_SIZE_X-10, REC_SIZE_Y-10, cursor_rect.color);
  }

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
            Serial.print("Calling from enc 123");
            remove_section_letters();
            removed = true; 
            letters_visible = false;
        }
        
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

            local_tft.fillRect(game_rect.tr_corner.x, game_rect.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, game_rect.color);

            if(rec_section >= 6)
            {
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
            draw_placeholders();
            draw_placeholder_letters();
            local_tft.drawRect(cursor_rect.tr_corner.x+5, cursor_rect.tr_corner.y+5, REC_SIZE_X-10, REC_SIZE_Y-10, cursor_rect.color);
        }
        encoder_set_value(cur_encoder_num, 0);
        last_action = last_action_none;

        break;
    case last_action_touch:
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

            if(game_rect.section == section)
            {
                Serial.println("section");
                Serial.println(section);
                x = game_rect.tr_corner.x;
                y = game_rect.tr_corner.y;
                color = game_rect.color;
                section = game_rect.section;

                int placeholder_x, placeholder_y;
                section_to_xy(cursor_rect.section, &placeholder_x, &placeholder_y);

                //local_tft.fillRect(game_rect.tr_corner.x, game_rect.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, BACKGROUND_COLOR);
                /*
                game_rect.tr_corner.x = placeholder_x;
                game_rect.tr_corner.y = placeholder_y;
                game_rect.color = color;
                game_rect.section = free_placeholer_num+18;*/
                

                local_tft.fillRect(game_rect.tr_corner.x, game_rect.tr_corner.y, REC_SIZE_X, REC_SIZE_Y, game_rect.color);
                
                if(is_section_with_letter(section))
                {
                    char mychar = get_letter(section);
                    shift_letter(section, cursor_rect.section);
                    Serial.println(mychar);
                    local_tft.drawChar(placeholder_x + 50, placeholder_y + 50, mychar, RA8875_BLACK, RA8875_YELLOW , 5);
                    rec_section = cursor_rect.section;
                    rec_section = find_next_free_placeholder(true, rec_section);
                    if(rec_section != -1)
                    {
                        local_tft.drawRect(cursor_rect.tr_corner.x+5, cursor_rect.tr_corner.y+5, REC_SIZE_X-10, REC_SIZE_Y-10, RA8875_YELLOW);
                        cursor_rect.section = rec_section;
                        section_to_xy(rec_section, &cursor_rect.tr_corner.x, &cursor_rect.tr_corner.y);
                        local_tft.drawRect(cursor_rect.tr_corner.x+5, cursor_rect.tr_corner.y+5, REC_SIZE_X-10, REC_SIZE_Y-10, cursor_rect.color);
                    }         
                }
            }
            break;
        }
        last_action = last_action_none;
        break;
    }
    
    if(find_next_free_placeholder(true, 18) == -1 || function_called_count > 4)
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
    letters_visible = true;
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

bool is_rect_here(int section)
{
    if(game_rect.section == section)
    {
        return true; 
    }
    return false;
}

void shift_letter(int cur_section, int cur_cursor_section)
{
    if(is_section_with_letter(cur_section))
    {
        char letter = get_letter(cur_section);
        valid_sections[cur_section] = 0;
        placeholder_letters[cur_cursor_section-18] = letter;
        free_placeholer_num++;
    }
}

int position_occupied(int section)
{
    Serial.println(rect_selected_num);

    if(game_rect.section == section)
    {
        Serial.print("next position occuppied. section: ");
        return 1; 
    }
                
    Serial.println("next position not occupied");
    return 0;
}

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
