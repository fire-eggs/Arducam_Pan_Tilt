#include "PCA9685_servo_driver.h"
#include <termios.h>
#include <pthread.h>
#include <signal.h>

double ServoUpDegree = 90;
double ServoDownDegree = 90;
pthread_t manualFocusPthreadID;

int get_key_board_from_termios()
{
    int key_value;
    struct termios new_config;
    struct termios old_config;

    tcgetattr(0, &old_config);
    new_config = old_config;
    new_config.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new_config);
    key_value = getchar();
    tcsetattr(0, TCSANOW, &old_config);
    return key_value;
}

#ifdef STEP
#undef STEP
#define STEP 0.25
#endif

//#define SLEEP 100
#define SLEEP 80

/* Microsecond time tracking
 */
typedef struct
{
    struct timeval startTimeVal;
} TIMER_usecCtx_t;

/* Helper : initialize a start time for microsecond tracking.
 * ctx : the time to be initialized
 */
void TIMER_usecStart(TIMER_usecCtx_t* ctx)
{
    gettimeofday(&ctx->startTimeVal, NULL);
}

/* Helper : provide microseconds elapsed from a given start time.
 * ctx : the start time
 */
unsigned int TIMER_usecElapsedUs(TIMER_usecCtx_t* ctx)
{
    unsigned int rv;

    /* get current time */
    struct timeval nowTimeVal;
    gettimeofday(&nowTimeVal, NULL);

    /* compute diff */
    rv = 1000000 * (nowTimeVal.tv_sec - ctx->startTimeVal.tv_sec) + nowTimeVal.tv_usec - ctx->startTimeVal.tv_usec;

    return rv;
}

/* Step the servo from a start position, by a given amount, in 10 increments.
 * Delay by given microseconds per increment.
 */
void delayGTD(int delayMicroseconds, double val, double delta, int n)
{
    // I don't understand this. To get something approaching
    // accurate clock timing, a fudge factor needs to be applied.
    // A fudge factor of 7 works for 10 and 20 second scans.
    delayMicroseconds *= 7;

    // Start our timer
    TIMER_usecCtx_t timer;
    TIMER_usecStart(&timer);

    int i =0;
    while (1)
    {
        // Loop, doing nothing, until the desired microseconds has passed
        if (TIMER_usecElapsedUs(&timer) > delayMicroseconds)
        {
            // increment the original position over 1/10 the desired amount
            val += delta * .1;
            setServoDegree(n, val);
            
            // count 10 steps
            i++;
            if (i == 10)
                return;
                
            TIMER_usecStart(&timer);
        }
    }
}

/* Make the servo 'scan' : step from a start position by a delta, with a given delay.
 * 
 * n : the servo channel
 * degree : the start position 
 * delta  : the amount to scan (positive: go up; negative: go down)
 * delay  : how much to delay for the scan, in microseconds
 */
void scan(int n, int degree, int delta, int delay)
{
    // delayGTD will step by 10 parts, determine the
    // delay to use for those 10 parts.
    int slowdelay = (int)(delay / 10.0 + 0.5);
    delayGTD(slowdelay, degree, delta, n);
}

/* Servo 'scan' test for the lower servo.
 * Scan from 90 to 0, then from 0 to 180.
 * Step the servo so the full scan takes the specified number of seconds.
 */
void loopTest(int seconds)
{

    // Initialize to known state
    setServoDegree(SERVO_UP_CH, 90);
    setServoDegree(SERVO_DOWN_CH,90);

    printf("Looptest - down 90 degrees, %g seconds\n", (seconds / 2.0));

    // 90 degrees : calc the # of microseconds delay required per degree
    int delay = (int)(seconds * 100000.0 / (90.0 * 2.0) + 0.5);

    // Scan down 90 degrees
    for (int i = 90; i >=0 ; i--)
        scan(SERVO_DOWN_CH, i, -1, delay);

    printf("Looptest - up 180 degrees, %d seconds\n", seconds);
        
    // 180 degrees : calc the # of microseconds delay required per degree
    delay = (int)(seconds * 100000.0 / 180.0 + 0.5);

    // Scan up 180 degrees
    for (int i = 0; i <= 180 ; i++)
        scan(SERVO_DOWN_CH, i, +1, delay);

    printf("End looptest\n");
}

void processKeyboardEvent(void){
    int keyVal = 0;
    while(1){
        usleep(SLEEP);
        tcflush(0, TCIOFLUSH);
        keyVal= get_key_board_from_termios();
        if (keyVal == 'L' || keyVal == 'l')
        {
            //time_t start = time(NULL);
            loopTest(10);
            //time_t end   = time(NULL);
            // Debugging: determine the actual time spent in seconds
            //printf("%d\n", (int)(end - start));
        }
        else if (keyVal == 'M' || keyVal == 'm')
        {
            //time_t start = time(NULL);
            loopTest(20);
            //time_t end   = time(NULL);
            // Debugging: determine the actual time spent in seconds
            //printf("%d\n", (int)(end - start));
        }
        else
        if(keyVal == 27){
            keyVal= get_key_board_from_termios();
            if(keyVal == 91){
              keyVal= get_key_board_from_termios();
               switch (keyVal)
                {
                    case 65/* up */:   
                        ServoDegreeIncrease(SERVO_UP_CH, STEP);
                        break;
                    case 66/* down */:
                        ServoDegreeDecrease(SERVO_UP_CH, STEP);
                        break;
                    case 67/* right */: 
                        ServoDegreeIncrease(SERVO_DOWN_CH, STEP);
                      break;
                    case 68/* left */: 
                        ServoDegreeDecrease(SERVO_DOWN_CH, STEP);
                        break;   
                    default :
                      break;
                }
            }
          }
      }
}

int main(int argc, char *argv[]){
        
    printf("Arrow keys: move servos\n");
    printf("L : loop test - lower servo scan, 10 seconds\n");
    printf("M : loop test - lower servo scan, 20 seconds\n");
    
    PCA9685_init(I2C_ADDR);
    //PCA9685_setPWMFreq(30);  // Analog servos run at ~30 Hz updates
    PCA9685_setPWMFreq(60);  // Analog servos run at ~60 Hz updates
    setServoDegree(SERVO_UP_CH, ServoUpDegree);
    setServoDegree(SERVO_DOWN_CH, ServoDownDegree);
    delay_ms(1000);
    processKeyboardEvent();
}
