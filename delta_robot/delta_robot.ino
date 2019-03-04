/*--------------------------------------------------------------------------------------------------------------------------------------------------------
 * 
 * CHECK THE CODE FOR "TODO:" AND EDIT APPROPRIATELY 
 * 
 * The code is developed for a Delta robot. The robot is controlled by an Arduino Nano.
 * 
 * Delta Robot STL files: 
 * 
 * Project video: 
 * 
 * All measurements are in SI units unless otherwise specified.
 * 
 * The arm's coordinate frame: Servo1 is alligned with the X-axis. Z-axis is vertically up.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE
 * 
 * Code written by isaac879
 *
 *--------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include "deltaRobot.h"
#include <Iibrary.h> //TODO: Add my custom library or remove dependant functions. Available at: https://github.com/isaac879/Iibrary

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

extern Coordinate end_effector;//Stores the end effector coordinates (declared in deltaRobot.cpp)

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

void setup(){
    Serial.begin(57600); //TODO: Baud rate for the HC-05 module is 9600 by default so needs to be set to 57600 via AT commands to work.
    attach_servos();
    inverse_kinematics(HOME_POSITION);
    gripper_servo(0);
    move_servos();
}

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

void loop(){    
    while(Serial.available() < 1){ //Wait for serial data to be available
        if(get_battery_level_voltage() < 9.5){//9.5V is used as the cut off to allow for inaccuracies and be on the safe side.
            delay(200);
            if(get_battery_level_voltage() < 9.5){//Check voltage is still low and the first wasn't a miscellaneous reading
                printi("Battery low! Turn the power off.");
                detach_servos();
                while(1){}//loop and do nothing
            }
        }
    } 
    
    byte instruction = Serial.read(); 
    int count = 0;
    
    switch(instruction){
        case COMMAND_CARTESIAN:{//Moves relitive to the current position
            while(Serial.available() != 6){//Wait for six bytes to be available. Breaks after ~200ms if bytes are not received.
                delayMicroseconds(200); 
                count++;
                if(count > 1000){
                    serialFlush();//Clear the serial buffer
                    break;   
                }
            }
            int xTarget = (Serial.read() << 8) + Serial.read();  
            int yTarget = (Serial.read() << 8) + Serial.read(); 
            int zTarget = (Serial.read() << 8) + Serial.read(); 
         
            inverse_kinematics(end_effector.x + xTarget, end_effector.y + yTarget, end_effector.z + zTarget);//Calculates servo positions 
            move_servos();//Moves the robot's servos
        }
        break;
        case COMMAND_ABSOLUTE_CARTESIAN:{//Moves to the specified position
            while(Serial.available() != 6){//Wait for six bytes to be available. Breaks after ~200ms if bytes are not received.
                delayMicroseconds(200); 
                count++;
                if(count > 1000){
                    serialFlush();//Clear the serial buffer
                    break;   
                }
            }
            int xTarget = (Serial.read() << 8) + Serial.read();  
            int yTarget = (Serial.read() << 8) + Serial.read(); 
            int zTarget = (Serial.read() << 8) + Serial.read(); 
         
            inverse_kinematics(xTarget, yTarget, zTarget);//Calculates servo positions 
            move_servos();//Moves the robot's servos
        }
        break;
        case COMMAND_STATUS:{
            report_status();
        }
        break;
        case COMMAND_REPORT_COMMANDS:{
            report_commands();
        }
        break;
        case COMMAND_GRIPPER:{
            while(Serial.available() < 1){
                delay(1);
                count++;
                if(count > 1000){ //Breaks after ~1 second if bytes are not received.
                    serialFlush(); //Clear the serial buffer
                    break;   
                }
            }
            char rotation = Serial.read();
            gripper_servo(rotation);
            move_servos();
        }break;
        case COMMAND_JOG_X:{
            inverse_kinematics(end_effector.x + get_serial_float(), end_effector.y, end_effector.z);//Calculates servo positions 
            move_servos();//Moves the robot's servos
        }
        break;
        case COMMAND_JOG_Y:{
            inverse_kinematics(end_effector.x, end_effector.y + get_serial_float(), end_effector.z);//Calculates servo positions 
            move_servos();//Moves the robot's servos
        }
        break;
        case COMMAND_JOG_Z:{
            inverse_kinematics(end_effector.x, end_effector.y, end_effector.z + get_serial_float());//Calculates servo positions 
            move_servos();//Moves the robot's servos
        }
        break;
        case COMMAND_GRIPPER_ASCII:{
            gripper_servo(get_serial_int());
            move_servos();
        }
        break;
        case COMMAND_ADD_POSITION:{
            add_position();
        }
        break;
        case COMMAND_CLEAR_ARRAY:{
            clear_array();
        }
        break;
        case COMMAND_SET_STEP_DELAY:{           
             set_step_delay(get_serial_int());
        }
        break;
        case COMMAND_SET_STEP_INCREMENT:{
            set_step_increment(get_serial_float());
        }
        break;
        case COMMAND_EXECUTE:{
            execute_moves(get_serial_int());
        }
        break;
        case COMMAND_EXECUTE_JOINT:{
            execute_moves_joint(get_serial_int());
        }
        break;
        case COMMAND_SET_US_INCREMENT:{
            set_step_pulses(get_serial_int());
        }
        break;
    }
}

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/
