#include "display_stuff.h"

#include "encoder_handle.h"


#include "Fonts/FreeSerifBoldItalic9pt7b.h"

#define LETTER_OFFSET 10

#define MAX_X_VAL 700
#define MIN_X_VAL 100
#define MEAN_X_VAL 400

#define MAX_Y_VAL_1 450
#define MIN_Y_VAL_1 400

#define MAX_Y_VAL_2 380
#define MIN_Y_VAL_2 330

#define MAX_Y_VAL_3 310
#define MIN_Y_VAL_3 260

#define STEP_SIZE 25

#define X_Y_RATIO double(MAX_Y_VAL_1-MIN_Y_VAL_1)/double(MAX_X_VAL-MIN_X_VAL)

#define REC_MIN_Y 60
#define REC_SIZE_X 133
#define REC_SIZE_Y 105

#define NUM_SECTIONS_X 6
#define NUM_SECTIONS_Y 4

#define NUM_SECTIONS NUM_SECTIONS_X*NUM_SECTIONS_Y

#define SCREEN_SIZE_X 800
#define SCREEN_SIZE_Y 480

unsigned long last_touch_time;


unsigned long random_number_0_6;

char letters[18];

int orig_valid_sections[18] = {1,0,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1};
int valid_sections[18] = {};
 //{0, 2, 3, 5, 7, 8, 9, 11, 12, 13, 15, 17}; 


enum
{
    last_action_none = 0,
    last_action_encoder1 = 1,
    last_action_encoder2,
    last_action_encoder3,
    last_action_touch
};

int last_action = last_action_none;

int free_placeholer_num = 0;
char placeholder_letters[6] = {};

tsPoint_t letter;
tsPoint_t calibrated;
tsPoint_t midpoint;

static int initialized_game =0;
//static int init_rect = 0;

int num_rect_visible;
int rect_selected_num;

rect selected_rect;

rect rectangles[6];

rect letter_place[6];

bool section0free = 1;

void init_rect()
{

    static tsMatrix_t local_matrix = gettsMatrix();
    static Adafruit_RA8875 local_tft = gettft();

    local_tft.fillScreen(BACKGROUND_COLOR);

    int i;
    for(i = 0; i<6; i++)
    {
        rectangles[i].tr_corner.x = 0;
        rectangles[i].tr_corner.y =  REC_MIN_Y;
        rectangles[i].section = 0;
        rectangles[i].color = RA8875_BLUE;
    }

    calibrateTSPoint(&rectangles[0].tr_corner_cal, &rectangles[0].tr_corner, &local_matrix);
    Serial.println(rectangles[0].tr_corner_cal.x);
    Serial.println(rectangles[0].tr_corner_cal.y);

    local_tft.fillRect(rectangles[0].tr_corner.x,rectangles[0].tr_corner.y, REC_SIZE_X, REC_SIZE_Y, rectangles[0].color);

    num_rect_visible = 1;
    rect_selected_num = 0;

    selected_rect = rectangles[0];

    // initialize letter placeholder rectangles: Here the Letter rectangles must be placed in order to solve the puzzle.
    int x, y;
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

    delay(1500);

    for(i = 0; i<6; i++)
    {
        /*
        local_tft.drawRect(letter_place[i].tr_corner.x+10, letter_place[i].tr_corner.y, REC_SIZE_X-15, REC_SIZE_Y-30, letter_place[i].color);
        */
        local_tft.fillRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, RA8875_YELLOW);
        local_tft.drawRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, letter_place[i].color);
    }

    local_tft.fillRect(0, 0, REC_SIZE_X*2, REC_MIN_Y, RA8875_GREEN);
    local_tft.drawRect(0, 0, REC_SIZE_X*2, REC_MIN_Y, 0x07D0);
    local_tft.textMode();
    local_tft.textTransparent(RA8875_RED);
    local_tft.textEnlarge(1);
    local_tft.textSetCursor(80, 10);
    local_tft.textWrite("SHOW");
    local_tft.graphicsMode();

    local_tft.fillRect(rectangles[0].tr_corner.x, rectangles[0].tr_corner.y, REC_SIZE_X, REC_SIZE_Y, rectangles[0].color);


    /*
    local_tft.drawFastHLine(letter_place[0].tr_corner.x+10, letter_place[0].tr_corner.y, 6*REC_SIZE_X-15, RA8875_BLUE);
    local_tft.drawFastHLine(letter_place[0].tr_corner.x+10, letter_place[0].tr_corner.y+REC_SIZE_Y-30, 6*REC_SIZE_X-15, RA8875_BLUE);
    local_tft.textMode();
    local_tft.textTransparent(RA8875_RED);
    local_tft.textSetCursor(letter_place[2].tr_corner.x+25, letter_place[0].tr_corner.y+REC_SIZE_Y-35);
    local_tft.textEnlarge(1);
    local_tft.textWrite("Password");
    local_tft.graphicsMode();*/
    
    free_placeholer_num = 0;

    for(i = 0; i<18; i++)
    {
        valid_sections[i] = orig_valid_sections[i];
    }

    random_letter_generation(true);   

}

void draw_placeholders()
{
    static Adafruit_RA8875 local_tft = gettft();

    char password[7] = "PASSWD";

    for(int i = 0; i<6; i++)
    {
        local_tft.fillRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, RA8875_YELLOW);
        local_tft.drawRect(letter_place[i].tr_corner.x, letter_place[i].tr_corner.y, REC_SIZE_X, REC_SIZE_Y-1, letter_place[i].color);
        //local_tft.drawChar(letter_place[i].tr_corner.x+40, letter_place[i].tr_corner.y+40, password[i], RA8875_RED, RA8875_YELLOW, 8);
        //draw_placeholder_letters();
    }
    /*

    for(int i = 0; i<6; i++)
    {
        local_tft.drawRect(letter_place[i].tr_corner.x+10, letter_place[i].tr_corner.y+10, REC_SIZE_X-15, REC_SIZE_Y-30, letter_place[i].color);
    }
    local_tft.drawFastHLine(letter_place[0].tr_corner.x+10, letter_place[0].tr_corner.y, 6*REC_SIZE_X-15, RA8875_BLUE);
    local_tft.drawFastHLine(letter_place[0].tr_corner.x+10, letter_place[0].tr_corner.y+REC_SIZE_Y-30, 6*REC_SIZE_X-15, RA8875_BLUE);
    local_tft.textMode();
    local_tft.textTransparent(RA8875_RED);
    local_tft.textSetCursor(letter_place[2].tr_corner.x+25, letter_place[0].tr_corner.y+REC_SIZE_Y-35);
    local_tft.textEnlarge(1);
    local_tft.textWrite("Password");
    local_tft.graphicsMode();*/
}

bool rectangles_game(tsPoint_t touch_raw)
{
    static Adafruit_RA8875 local_tft = gettft();
    static tsMatrix_t local_matrix = gettsMatrix();

    int x = rectangles[rect_selected_num].tr_corner.x;
    int y = rectangles[rect_selected_num].tr_corner.y;
    int rec_section = rectangles[rect_selected_num].section;
    int color = rectangles[rect_selected_num].color;

    // if last action was an encoder action, i.e. either of the two first encoders was used:
    if(last_action >= 1 && last_action <= 2)
    {
        Serial.print("last action = ");
        Serial.print(last_action);
        // the last action matches the current encode number. 
        int cur_encoder_num = last_action;
        /*if(color == RA8875_RED)
        {
            encoder_set_value(1, 0);
            last_action = last_action_none;
            return 0;
        }*/
        /*Serial.print("selected rect: ");
        Serial.println(rect_selected_num);
        Serial.println(x);
        Serial.println(y);
        Serial.println(rec_section);*/

        int sign; 
        if(encoder_get_value(cur_encoder_num) < 0)
            sign = 1;
        else
            sign = -1;

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
                if(((rec_section+1) % NUM_SECTIONS_X) && (rec_section+1 < NUM_SECTIONS) &&
                (!position_occupied(rec_section + 1)))
                {
 
                    Serial.println("one to the rightbefore: ");
                    Serial.println(x);
                    x += REC_SIZE_X;
                    rec_section += 1;
                    rectangles[rect_selected_num].tr_corner.x = x;
                    rectangles[rect_selected_num].section = rec_section;
                    if(rec_section > 0)
                    {
                        section0free = true;
                    }
                }
                else if(!(rec_section+1) % NUM_SECTIONS_X)
                {
                    // no nothing. rectangle should stay where it is
                }
                else if(isNewRecToCome(rect_selected_num))
                {

                    //for now: puzzle is solved now!
                    Serial.print("puzzle solved");
                    //return 1;

                    
                    Serial.print("num rect visinle");
                    Serial.print(num_rect_visible);
                    if(num_rect_visible < 6)
                    {
                        local_tft.fillRect(x, y, REC_SIZE_X, REC_SIZE_Y, color);
                        num_rect_visible++;
                        Serial.print(num_rect_visible);
                        //save_rect_data(rect_selected_num, selected_rect.tr_corner, selected_rect.color);
                        rect_selected_num = num_rect_visible-1;
                    }
                }
                else {
                    Serial.println("else loop");
                }
            }
            else if(cur_encoder_num == 2)
            {
                if((y + 2 * REC_SIZE_Y <= SCREEN_SIZE_Y) && (rec_section+6 < NUM_SECTIONS) &&
                (!position_occupied(rec_section+6)))
                {
                    Serial.println("down");
                    y+= REC_SIZE_Y;
                    //x = 0;
                    rec_section += 6;
                    if(rec_section > 0)
                    {
                        section0free = true;
                    }
                    rectangles[rect_selected_num].tr_corner.y = y;
                    //rectangles[rect_selected_num].tr_corner.x = x;
                    rectangles[rect_selected_num].section = rec_section;
                }
                else if(!(rec_section+6 < NUM_SECTIONS))
                {
                    // no nothing. Rectangle should stay where it is
                }
                else {
                    Serial.println("else loop");
                }
            }
            /*
            else if((y + 2 * REC_SIZE_Y <= SCREEN_SIZE_Y) &&
            (!position_occupied(rec_section+1)))
            {
                Serial.println("down");
                y+= REC_SIZE_Y;
                x = 0;
                rec_section += 1;
                rectangles[rect_selected_num].tr_corner.y = y;
                rectangles[rect_selected_num].tr_corner.x = x;
                rectangles[rect_selected_num].section = rec_section;
            }*/
        }
        else if (sign == -1)
        {
             if(cur_encoder_num == 1)
                {
                    if(( x - REC_SIZE_X >= 0) && (!position_occupied(rec_section - 1)))
                    {
                        x -= REC_SIZE_X;
                        rec_section -= 1;
                        rectangles[rect_selected_num].tr_corner.x = x;
                        rectangles[rect_selected_num].section = rec_section;
                        if(rec_section == 0)
                        {
                            section0free = false;
                        }
                    }/*
                    else if(( y - REC_SIZE_Y >= 0) && (!position_occupied(rec_section - 1)))
                    {
                        y -= REC_SIZE_Y;
                        x = SCREEN_SIZE_X - REC_SIZE_X;
                        rec_section -= 1;
                        rectangles[rect_selected_num].tr_corner.y = y;
                        rectangles[rect_selected_num].tr_corner.x = x;
                        rectangles[rect_selected_num].section = rec_section;
                    }*/
                    else
                    {

                    }
                }
                else if(cur_encoder_num == 2)
                {
                    if(( y - REC_SIZE_Y >= 0) && (!position_occupied(rec_section - 6)))
                    {
                        y -= REC_SIZE_Y;
                        //x = SCREEN_SIZE_X - REC_SIZE_X;
                        rec_section -= 6;
                        rectangles[rect_selected_num].tr_corner.y = y;
                        rectangles[rect_selected_num].tr_corner.x = x;
                        rectangles[rect_selected_num].section = rec_section;                        
                    }
                }
        }

        encoder_set_value(cur_encoder_num, 0);
        //encoder_set_value(2, 0);

        local_tft.fillRect(rectangles[rect_selected_num].tr_corner.x, rectangles[rect_selected_num].tr_corner.y, REC_SIZE_X, REC_SIZE_Y, rectangles[rect_selected_num].color);
        if(rectangles[rect_selected_num].section == 0)
            section0free = false;

        if(rectangles[rect_selected_num].section >= 5)
        {
            draw_placeholder_letters();
        }
        last_action = last_action_none;
    }
    else if(last_action == last_action_touch)
    {
        tsPoint_t calibrated;
        //Calcuate the real X/Y position based on the calibration matrix 
        calibrateTSPoint(&calibrated, &touch_raw, &local_matrix);

        if(calibrated.y < REC_MIN_Y && calibrated.x < 2*REC_SIZE_X)
        {
            //show letters. 
            random_letter_generation(false);
            int section; 
            int letter_bg_color;
            int char_x, char_y;
            bool valid_section;
            for(section=0; section<18; section++)
            {
                valid_section = is_section_with_letter(section);
                if(valid_section)
                {
                    char mychar = get_letter(section);
                    int rect_num = who_is_here(section);
                    if (rect_num == -1)
                    {
                        letter_bg_color = BACKGROUND_COLOR;
                    }
                    else
                    {
                        letter_bg_color = rectangles[rect_num].color;
                    }
                section_to_xy(section, &char_x, &char_y);
                local_tft.drawChar(char_x+50, char_y+50, mychar, RA8875_BLACK, letter_bg_color , 5);
                }
            }
            delay(1000);
            for(section=0; section<18; section++)
            {   
                valid_section = is_section_with_letter(section);
                if(valid_section)
                {
                    char mychar = get_letter(section);
                    int rect_num = who_is_here(section);
                    int letter_color; 
                    if (rect_num == -1)
                    {
                        letter_color = BACKGROUND_COLOR;
                        letter_bg_color = BACKGROUND_COLOR;
                    }
                    else
                    {
                        letter_color = rectangles[rect_num].color;
                        letter_bg_color = rectangles[rect_num].color;
                    }
                    
                    section_to_xy(section, &char_x, &char_y);
                    local_tft.drawChar(char_x + 50, char_y + 50, mychar, letter_color, letter_bg_color, 5);
                }
            }

        }
        else
        {
            int section = get_section(calibrated.x, calibrated.y);

            int i; 
            for(i = 0; i< num_rect_visible; i++)
            {
                if(rectangles[i].section == section)
                {
                    Serial.println("rectancle was touched! no: ");
                    Serial.println(i);
                    Serial.println("section");
                    Serial.println(section);
                    rect_selected_num = i;
                    x = rectangles[rect_selected_num].tr_corner.x;
                    y = rectangles[rect_selected_num].tr_corner.y;
                    color = rectangles[rect_selected_num].color;
                    section = rectangles[rect_selected_num].section;

                    if(section < 18)
                    {
                        int placeholder_x, placeholder_y;
                        section_to_xy(free_placeholer_num+18, &placeholder_x, &placeholder_y);
                        Serial.println("fill rect");

                        local_tft.fillRect(rectangles[rect_selected_num].tr_corner.x, rectangles[rect_selected_num].tr_corner.y, REC_SIZE_X, REC_SIZE_Y, BACKGROUND_COLOR);

                        rectangles[rect_selected_num].tr_corner.x = placeholder_x;
                        rectangles[rect_selected_num].tr_corner.y = placeholder_y;
                        rectangles[rect_selected_num].color = color;
                        rectangles[rect_selected_num].section = free_placeholer_num+18;

                        local_tft.fillRect(rectangles[rect_selected_num].tr_corner.x, rectangles[rect_selected_num].tr_corner.y, REC_SIZE_X, REC_SIZE_Y, rectangles[rect_selected_num].color);
                        
                        if(is_section_with_letter(section)) //&& color == RA8875_RED)
                        {
                            char mychar = get_letter(section);
                            shift_letter(section);
                            Serial.println(mychar);
                            local_tft.drawChar(placeholder_x + 50, placeholder_y + 50, mychar, RA8875_BLACK, rectangles[rect_selected_num].color , 5);
                        }
                    }

                }
            }
        }
        last_action = last_action_none;
    }
    if(free_placeholer_num == 6)
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

bool isNewRecToCome(int rect_selected_num)
{
    int section; 
    section = get_section(rectangles[rect_selected_num].tr_corner.x, rectangles[rect_selected_num].tr_corner.y);

        Serial.print("section of rect");
        Serial.print(section);
    if(section == 23)
        return true;

    bool found = false;
    int i; 
    for(i = section+1; i<NUM_SECTIONS; i++)
    {
        Serial.print("section");
        Serial.println(i);
        found = false;
        Serial.println(num_rect_visible);
        for(int j = 0; j<num_rect_visible; j++)
        {
            Serial.print("section of rect no: ");
            Serial.println(j);
            Serial.print(get_section(rectangles[j].tr_corner.x, rectangles[j].tr_corner.y));
            if(get_section(rectangles[j].tr_corner.x, rectangles[j].tr_corner.y) == i)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            return false; 
        }
    }
    return true;
}

int get_section(int x, int y)
{
    int section = ((y-REC_MIN_Y)/REC_SIZE_Y)*NUM_SECTIONS_X + (x/REC_SIZE_X);
    return section;
}

void section_to_xy(int section, int* x, int* y)
{
    int section_y = section / NUM_SECTIONS_X;
    *y = section_y * REC_SIZE_Y + REC_MIN_Y;


    *x = (section - (section_y * NUM_SECTIONS_X)) * REC_SIZE_X;
}

bool is_section_with_letter(int section)
{
    if(valid_sections[section] == true)
        return true;
    return false;
}

void random_letter_generation(bool init)
{
    static int function_called_count = 0;

    if(init)
    {
        function_called_count=0;
    }

    Serial.println("counter = " +  String(function_called_count));
    static int random_start;
    char manual[7] = "MANUAL";
    if(!(function_called_count%4))
    {
        function_called_count = 0;
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
    function_called_count++;
}

char get_letter(int section)
{
    return letters[section];
}

void set_letter(char letter){

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
        int rect_num = who_is_here(i+18);
        if(rect_num >= 0)
            bg_color = rectangles[rect_num].color;
        else 
            bg_color = RA8875_YELLOW;
        local_tft.drawChar(placeholder_x + 50, placeholder_y +50, placeholder_letters[i], RA8875_BLACK, bg_color, 5);
    }
}

int who_is_here(int section)
{
    int i;
    for(i = 0; i<num_rect_visible; i++)
    {
        if(rectangles[i].section == section)
            {
                Serial.println("rectange " + String(i) + " is in section " + String(section));
                return i; 
            }
    }
    return -1;
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
    int i; 
    Serial.println(rect_selected_num);
    for(i = 0; i<num_rect_visible; i++)
    {
        if(i != rect_selected_num)
        {
            if(rectangles[i].section == section)
            {
                Serial.print("next position occuppied. section: ");
                return 1; 
            }
                
        }
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
/*
void dissapearing_letters()
{
    static tsMatrix_t local_matrix = gettsMatrix();
    static Adafruit_RA8875 local_tft = gettft();

    if(!initialized_game)
    {
        Serial.println("Initializing game");
        local_tft.setFont(&FreeSerifBoldItalic9pt7b);

        letter.x = 400;
        letter.y = 240;
        calibrateTSPoint(&calibrated, &letter, &local_matrix );
        Serial.println(calibrated.x);
        Serial.println(calibrated.y);
        char mychar = 'A';
        local_tft.drawChar(letter.x,letter.y, mychar, RA8875_BLUE, BACKGROUND_COLOR, 5);

        midpoint.x = letter.x + LETTER_OFFSET;
        midpoint.y = letter.y + LETTER_OFFSET;
        initialized_game = 1;
    }    

    tsPoint_t raw;
    tsPoint_t calibrated;

    int hit = 0;
    while(hit == 0)
    {
        // Wait around for a touch event 
        waitForTouchEvent(&raw);

        //Calcuate the real X/Y position based on the calibration matrix 
        calibrateTSPoint(&calibrated, &raw, &local_matrix );

        Serial.println("Calibrated letter: ");
        Serial.println(letter.x);
        Serial.println(letter.y);    

        if(abs(calibrated.x - midpoint.x) < 40 && abs(calibrated.y - midpoint.y) < 40)
        {
            Serial.println("Hit button");
            local_tft.drawChar(letter.x,letter.y, 'A', BACKGROUND_COLOR, RA8875_WHITE, 5);
            initialized_game = 0;
            hit = 1;
        }
        else
        { 
            Serial.println("Failed: ");
            local_tft.drawChar(letter.x,letter.y, 'A', RA8875_RED, RA8875_WHITE,5);
            delay(300);
            local_tft.drawChar(letter.x,letter.y, 'A', RA8875_BLUE, RA8875_WHITE,5);
        }
        Serial.print("touchscreen sensed at: ");
        Serial.println(calibrated.x);
        Serial.println(calibrated.y);
        // Draw a single pixel at the calibrated point 
        local_tft.fillCircle(calibrated.x, calibrated.y, 3, RA8875_BLACK);
    }
}
*/
bool sliding_bars(int encoder_num)
{
    static Adafruit_RA8875 local_tft = gettft();
    int32_t color;
    int max_Y_val = 0;
    int min_Y_val = 0;
    if(encoder_num == 1)
    {
        color = RA8875_CYAN;
        max_Y_val = MAX_Y_VAL_1;
        min_Y_val = MIN_Y_VAL_1;
    }
    else if(encoder_num == 2)
    {
        color = RA8875_GREEN;
        max_Y_val = MAX_Y_VAL_2;
        min_Y_val = MIN_Y_VAL_2;
    }
    else if(encoder_num == 3)
    {
        color = RA8875_MAGENTA;
        max_Y_val = MAX_Y_VAL_3;
        min_Y_val = MIN_Y_VAL_3;
    }

    local_tft.textMode();
    local_tft.textSetCursor(250, 100);
    

    /* Render some text! */
    char string[16] = "Ferdi's Radio! ";
    local_tft.textTransparent(RA8875_BLUE);
    local_tft.textEnlarge(2.5);
    local_tft.textWrite(string);

    local_tft.graphicsMode();
    local_tft.fillTriangle(MIN_X_VAL, max_Y_val, MAX_X_VAL, max_Y_val, MAX_X_VAL, min_Y_val, BACKGROUND_COLOR);
    local_tft.drawTriangle(MIN_X_VAL, max_Y_val, MAX_X_VAL, max_Y_val, MAX_X_VAL, min_Y_val, color);

    local_tft.textMode();

    char string_buffer[4];
    for(int i = 0; i<=24; i++)
    {
        itoa(i-12, string_buffer, 10);
        draw_numbers(string_buffer, MIN_X_VAL + ((MAX_X_VAL - MIN_X_VAL) / 24)*i, max_Y_val);
    }

    int encoder_value = encoder_get_value(encoder_num);

    int bar_fill_value_x = convert_encoder2display_x(encoder_value);
    int bar_fill_value_y = convert_encoder2display_y(bar_fill_value_x, max_Y_val);

    Serial.print("value of encoder num ");
    Serial.print(encoder_num);
    Serial.print(": ");
    Serial.print(encoder_value);

    local_tft.graphicsMode();

    local_tft.fillTriangle(MIN_X_VAL, max_Y_val, bar_fill_value_x, max_Y_val, bar_fill_value_x, bar_fill_value_y, color);

    int solved_values[3] = {10,-4,-1};

    int i;
    for(i = 1; i<=NUM_ENCODERS_DEFINED; i++)
    {
        Serial.print(solved_values[i-1]);
        Serial.print(encoder_get_value(i));
        if(encoder_get_value(i) != solved_values[i-1])
        {
            return 0;
        }      
    }
    //solved
    return 1;
}

void draw_numbers(const char* string, int cursor_x, int cursor_y)
{

    static Adafruit_RA8875 local_tft = gettft();

    local_tft.textSetCursor(cursor_x-8, cursor_y+5);

    local_tft.textTransparent(RA8875_BLUE);
    local_tft.textEnlarge(0);
    local_tft.textWrite(string);
}
int convert_encoder2display_x(int encoder_value)
{
    int return_val = MEAN_X_VAL;

    return_val += 25 * encoder_value;

    return return_val;
}

int convert_encoder2display_y(int x_val, int max_y_val)
{
    int x_length = x_val-MIN_X_VAL;

    int return_val  = max_y_val - X_Y_RATIO * x_length;

    return return_val;
}



/**************************************************************************/
/*!
    @brief  Waits for a touch or encoder event
*/
/**************************************************************************/
int waitForTouchorEncoderEvent(tsPoint_t * point)
{

    static Adafruit_RA8875 local_tft = gettft();

    /* Clear the touch data object and placeholder variables */
    memset(point, 0, sizeof(tsPoint_t));

    /* Clear any previous interrupts to avoid false buffered reads */
    uint16_t x, y;
    local_tft.touchRead(&x, &y);
    delay(1);

    /* Wait around for a new touch event (INT pin goes low) */
    while (digitalRead(RA8875_INT))
    {
        int enc_val = check_encoder();
        switch(enc_val)
        {
            case 1:
                last_action = last_action_encoder1;
                return enc_val;
                break;
            case 2:
                last_action = last_action_encoder2;
                return enc_val;
                break;
            case 3:
                last_action = last_action_encoder3;
                return enc_val;
                break;
            default:
                last_action = last_action_none;
                break;
        }
    }

    /* Make sure this is really a touch event */
    if (local_tft.touched() && millis() - last_touch_time > 300)
    {
    local_tft.touchRead(&x, &y);
    point->x = x;
    point->y = y;
    Serial.print("Touch: ");
    Serial.print(point->x); Serial.print(", "); Serial.println(point->y);

    last_touch_time = millis();
    last_action = last_action_touch;
    }
    else
    {
    point->x = 0;
    point->y = 0;
    }
    return 0;
}
void first_screen()
{
    static Adafruit_RA8875 local_tft =  gettft();
    local_tft.graphicsMode();
    fill_display(RA8875_BLACK);
 
    uint16_t lcd_buffer[1000];
    for(int i = 0; i<1000; i++)
    {
        lcd_buffer[i] = RA8875_WHITE;
    }

    for(int i = 0; i<480; i+=4)
    {
        local_tft.drawPixels(lcd_buffer, 800, 300, i);
    }
}

void final_screen()
{
    static Adafruit_RA8875 local_tft = gettft();
    local_tft.fillScreen(RA8875_BLUE);
}