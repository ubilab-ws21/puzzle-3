#include "frequency_puzzle.h"

#include "encoder_handle.h"
#include "display_stuff.h"

// important defines
#define REC_MIN_Y 80

#define MAX_X_VAL 700
#define MIN_X_VAL 100
#define MEAN_X_VAL 400

#define MAX_Y_VAL_1 450
#define MIN_Y_VAL_1 400

#define MAX_Y_VAL_2 380
#define MIN_Y_VAL_2 330

#define MAX_Y_VAL_3 310
#define MIN_Y_VAL_3 260

#define X_Y_RATIO double(MAX_Y_VAL_1-MIN_Y_VAL_1)/double(MAX_X_VAL-MIN_X_VAL)

#define SCREEN_SIZE_X 800
#define SCREEN_SIZE_Y 480

#define SELECT_FIELD_X 600
#define SELECT_FIELD_Y 100

// default frequency is 1332 MHz. This value will be changed as soon as the game starts or the encoders are turned.
static int cur_frequency = 1332; 
static char freq_pre_comma[5] = "133";
static char freq_post_comma[2] = "2";

// correct encoder values
int solved_values[3] = {10,-4,-1};

void init_sliding_bars(void)
{
    static Adafruit_RA8875 local_tft = gettft();
    local_tft.displayOn(true);

    //--------------------------------------------------------------------------
    // this is just some text that is permanently printed on top of the screen. 
    // No update during the game is needed. 
    local_tft.fillScreen(BACKGROUND_COLOR);
    local_tft.fillRect(0,0,SCREEN_SIZE_X, 60, RA8875_BLUE);
    local_tft.fillRoundRect(600, 100, 160, 70, 20, RA8875_BLUE);
    local_tft.drawRoundRect(600, 100, 160, 70, 20, RA8875_WHITE);

    local_tft.textMode();

    local_tft.textSetCursor(20, 20);
    local_tft.textEnlarge(0);

    local_tft.textTransparent(RA8875_CYAN);

    local_tft.textWrite("-------------- +++++ NEWS: Unexpectedly High Enviromental Temperature +++++ --------------------                                 Ferdi's Radio - Frequency Select Menu");

    // Display Frequency. Frequency is saved in the variables freq_pre_comma and freq_post_comma. Will be updated during the game.
    local_tft.textSetCursor(80, REC_MIN_Y+20);
    local_tft.textTransparent(RA8875_BLUE);
    local_tft.textEnlarge(2);
    local_tft.textWrite("Choose Frequency:");

    local_tft.textTransparent(RA8875_BLACK);
    local_tft.textEnlarge(2);
    local_tft.textSetCursor(250, REC_MIN_Y+70);

    local_tft.textWrite(freq_pre_comma);
    local_tft.textSetCursor(320, REC_MIN_Y+70);
    local_tft.textWrite(",");
    local_tft.textSetCursor(340, REC_MIN_Y+70);
    local_tft.textWrite(freq_post_comma);

    local_tft.textSetCursor(390, REC_MIN_Y+70);
    local_tft.textWrite("MHz");

    // Text for Select button to confirm currently selected Frequency. Will lead to next puzzle if frequency is correct.
    local_tft.textSetCursor(630, 120);
    local_tft.textTransparent(RA8875_WHITE);
    local_tft.textEnlarge(1);
    local_tft.textWrite("Select");
     //--------------------------------------------------------------------------

    // init all three sliding bars. Don't reset the currently set encoder values.
    // This way we can call this init function without resetting the sliding bars.
    tsPoint_t raw;
    sliding_bars(1, raw, 1);
    sliding_bars(2, raw, 1);
    sliding_bars(3, raw, 1);
}

/************************************************************************************************
* This Function is the main function for the frequency puzzle. The three bars are updated (filled) 
* according to how the three game encoders are turned, which leads to a change of the chosen frequency.
* If the Select Button is pressed and the choosen frequency is correct, the puzzle is solved and this 
* function will return 1.
*************************************************************************************************/ 
bool sliding_bars(int encoder_num,  tsPoint_t touch_raw, int init)
{
    static Adafruit_RA8875 local_tft = gettft();
    int32_t color;
    int max_Y_val = 0;
    int min_Y_val = 0;

    // Find out which sliding bar is to be updated, i.e. which encoder was triggered/with which encoder this function was called.
    // Set the coordinated of the bars accordingly.
    if(encoder_num == 1)
    {
        color = 0xFDE0; //dark yellow
        max_Y_val = MAX_Y_VAL_1;
        min_Y_val = MIN_Y_VAL_1;
    }
    else if(encoder_num == 2)
    {
        color = 0x03E0; //dark green
        max_Y_val = MAX_Y_VAL_2;
        min_Y_val = MIN_Y_VAL_2;
    }
    else if(encoder_num == 3)
    {
        color = RA8875_RED; 
        max_Y_val = MAX_Y_VAL_3;
        min_Y_val = MIN_Y_VAL_3;
    }

    // update top half of the screen: current frequency
    update_top_half();
    

    local_tft.graphicsMode();
    int last_action = get_last_action();
    // only react on encoder triggers from encoders 1 to 3, i.e. the game encoders, or on touch events. 
    if(((last_action >= last_action_encoder1) && (last_action <= last_action_encoder3)) || init)
    {
        local_tft.fillTriangle(MIN_X_VAL, max_Y_val, MAX_X_VAL, max_Y_val, MAX_X_VAL, min_Y_val, BACKGROUND_COLOR);
        local_tft.drawTriangle(MIN_X_VAL, max_Y_val, MAX_X_VAL, max_Y_val, MAX_X_VAL, min_Y_val, color);

        // under the sliding bars, draw the numbers -12 to 12, which correspond to the time differences from the GMT.
        local_tft.textMode();
        char string_buffer[4];
        for(int i = 0; i<=24; i++)
        {
            itoa(i-12, string_buffer, 10);
            draw_numbers(string_buffer, MIN_X_VAL + ((MAX_X_VAL - MIN_X_VAL) / 24)*i, max_Y_val);
        }

        // get value of the current encoder to be able to fill the corresponding bar up to this number
        int encoder_value = encoder_get_value(encoder_num);
        // convert number to bar fill value
        int bar_fill_value_x = convert_encoder2display_x(encoder_value);
        int bar_fill_value_y = convert_encoder2display_y(bar_fill_value_x, max_Y_val);

        // fill the triangle up to calculated number
        local_tft.graphicsMode();
        local_tft.fillTriangle(MIN_X_VAL, max_Y_val, bar_fill_value_x, max_Y_val, bar_fill_value_x, bar_fill_value_y, color);
        last_action = last_action_none;
    }
    else if(last_action == last_action_touch)
    {
        // if the last action was touched, check if the select button was touched. 
        // in this case, we need to check if the current frequency is the correct one. 
        // Otherwise, ignore touch event. 
        last_action = last_action_none;
        if(select_btn_pressed(touch_raw))
        {
            int i;
            for(i = 1; i<=NUM_ENCODERS_DEFINED; i++)
            {
                if(encoder_get_value(i) != solved_values[i-1])
                {
                    // an encoder value has found which does not match the desired one. 
                    // print "unknown frequency" on screen for 1,5 seconds.
                    local_tft.fillRoundRect(160, SELECT_FIELD_Y, 400, 100, 10, RA8875_RED);
                    local_tft.drawRoundRect(160, SELECT_FIELD_Y, 400, 100, 10, RA8875_BLACK);

                    local_tft.textMode();
                    local_tft.textEnlarge(1);
                    local_tft.textSetCursor(220, 130);
                    local_tft.textTransparent(RA8875_BLUE);
                    local_tft.textWrite("Unknown Frequency!");
                    delay(1500);
                    local_tft.fillRoundRect(160, SELECT_FIELD_Y, 400, 100, 10, BACKGROUND_COLOR);

                    // print everything on screen again so that player can try again.
                    init_sliding_bars();

                    // return 0, puzzle has not been solved yet.
                    return 0;
                }      
            }
            // puzzle has been solved!
            // save the current frequency for next game. Then return 1 to indicate that puzzle was solved.
            set_solved_frequency(calc_cur_frequency());
            return 1;
        }
    }
    // puzzle not solved
    return 0;
}

/********************************************************************************
* This function handles a touch event by checking if the select button was pressed. 
* Returns 1 if this was the case.
**********************************************************************************/
bool select_btn_pressed(tsPoint_t touch_raw)
{
    tsPoint_t calibrated;
    static tsMatrix_t local_matrix = gettsMatrix();
    //Calcuate the real X/Y position based on the calibration matrix 
    calibrateTSPoint(&calibrated, &touch_raw, &local_matrix);

    if(calibrated.x >= SELECT_FIELD_X && calibrated.x <= SELECT_FIELD_X + 180)
    {
        if(calibrated.y >= SELECT_FIELD_Y && calibrated.y <= SELECT_FIELD_Y + 80)
        {
            // select button was pressed.
            Serial.println("select pressed!");
            return true;
        }
    }
    // user pressed somewhere else.
    return false; 
}

/*******************************************************************************
* Update top half of the screen, particularly update the currently chosen frequency
*******************************************************************************/
void update_top_half()
{
    int new_freq = calc_cur_frequency();
    // update frequency if new one was found.
    if(new_freq != cur_frequency)
    {
        cur_frequency = new_freq;

        static Adafruit_RA8875 local_tft = gettft();

        local_tft.textMode();

        local_tft.textColor(BACKGROUND_COLOR, BACKGROUND_COLOR);
        local_tft.textEnlarge(2);
        local_tft.textSetCursor(250, REC_MIN_Y+70);

        local_tft.textWrite(freq_pre_comma);
        local_tft.textSetCursor(320, REC_MIN_Y+70);
        local_tft.textWrite(",");
        local_tft.textSetCursor(340, REC_MIN_Y+70);
        local_tft.textWrite(freq_post_comma);

        char string1[5];
        itoa(cur_frequency, string1, 10);
        strcpy(freq_pre_comma, string1);
        if(cur_frequency >= 1000)
        {
            strcpy(freq_post_comma, string1+3);
            freq_pre_comma[3] = '\0';
        }
        else{
            strcpy(freq_post_comma, string1+2);
            freq_pre_comma[2] = '\0';
        }

        local_tft.textSetCursor(80, REC_MIN_Y+20);
        local_tft.textTransparent(RA8875_BLUE);
        local_tft.textEnlarge(2);

        local_tft.textWrite("Choose Frequency:");

        local_tft.textTransparent(RA8875_BLACK);
        local_tft.textEnlarge(2);
        local_tft.textSetCursor(250, REC_MIN_Y+70);
        local_tft.textWrite(freq_pre_comma);

        local_tft.textSetCursor(320, REC_MIN_Y+70);
        local_tft.textWrite(",");
        local_tft.textSetCursor(340, REC_MIN_Y+70);
        local_tft.textWrite(freq_post_comma);

        local_tft.textSetCursor(390, REC_MIN_Y+70);
        local_tft.textWrite("MHz");


        local_tft.graphicsMode();

    }

}

/*******************************************************************************
* This function calculated the current frequency according to the current game
* encoder values, and returns the calculation.
*******************************************************************************/
int calc_cur_frequency()
{
    int summed_values = 1332;
    int i;

    summed_values = 1332; // + encoder_get_value(1);
    for(i = 1; i<=NUM_ENCODERS_DEFINED; i++)
    {
        Serial.println(encoder_get_value(i));
        summed_values += encoder_get_value(i)*pow10(i-1);
    }
   return summed_values;
    
}

/**************************************************************************
* This function draws a certain string (always a number, in this case),
* onto the screen at certain coordinates
***************************************************************************/
void draw_numbers(const char* string, int cursor_x, int cursor_y)
{

    static Adafruit_RA8875 local_tft = gettft();

    local_tft.textSetCursor(cursor_x-8, cursor_y+5);

    local_tft.textTransparent(RA8875_BLUE);
    local_tft.textEnlarge(0);
    local_tft.textWrite(string);
}

/*******************************************************************************
* This function calculats the x value of the filling of the triangle to indicate
* the current encoder value. 
*******************************************************************************/
int convert_encoder2display_x(int encoder_value)
{
    int return_val = MEAN_X_VAL;

    return_val += 25 * encoder_value;

    return return_val;
}

/*******************************************************************************
* This function calculats the y value of the filling of the triangle to indicate
* the current encoder value. 
*******************************************************************************/
int convert_encoder2display_y(int x_val, int max_y_val)
{
    int x_length = x_val-MIN_X_VAL;

    int return_val  = max_y_val - X_Y_RATIO * x_length;

    return return_val;
}