/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       1697A, David Crespi                                       */
/*    Created:      Sat Nov 18 2023                                           */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include "vex.h"

using namespace vex;

/*---------------------------------------------------------------------------*/
/*                               Configurations                              */
/*                               max watts: 88                               */
/*                              total watts: 55                              */
/*---------------------------------------------------------------------------*/

//general
brain Brain;
competition Competition;
controller Ctrllr = controller();

//drive motors
motor left_front = motor(PORT1, false); //11w
motor left_back = motor(PORT6, false); //11w
motor right_front = motor(PORT5, true); //11w
motor right_back = motor(PORT7, true); //11w

//mandible motors
motor left_mandible = motor(PORT8, false); //5.5w
motor right_mandible = motor(PORT9, true); //5.5w

//catapult motor
motor catapult = motor(PORT10, false); //11w

//pneumatics
pneumatics Pneumatics = pneumatics(Brain.ThreeWirePort.A);

//motor groups
motor_group left_drive = motor_group(left_front, left_back);
motor_group right_drive = motor_group(right_front, right_back);
motor_group full_drive = motor_group(left_front,left_back,right_front, right_back);
motor_group mandibles = motor_group(left_mandible, right_mandible);

/* global variables */
const motor motors[] = {left_front,left_back,right_front, right_back};

const double DEADZONE = 15.1;
int driveType = 0;
/* end */

/*---------------------------------------------------------------------------*/
/*                              Brain Graphics                               */
/*---------------------------------------------------------------------------*/

// 480x240 pixels
void choose_drive_brain() {
  Brain.Screen.drawRectangle(0,0,160,240,red);
  Brain.Screen.drawRectangle(160,0,320,240,green);
  Brain.Screen.drawRectangle(320,0,480,240,blue);

  waitUntil(Brain.Screen.pressing());
  int x = Brain.Screen.xPosition(); //last x position

  if(x < 160)
    driveType = 0;
  else if(x < 320)
    driveType = 1;
  else if(x < 480)
    driveType = 2;
}
void brain_display_ports() {
  Brain.Screen.clearScreen();
  Brain.Screen.setCursor(0,0);
  Brain.Screen.print("Mandibles:"); Brain.Screen.newLine();
  Brain.Screen.print("\tLeft: 8 -- Right: 9"); Brain.Screen.newLine();
  Brain.Screen.print("Drivetrain Motors:"); Brain.Screen.newLine();
  Brain.Screen.print("\tLeft Front: 1 -- Right Front: 5"); Brain.Screen.newLine();
  Brain.Screen.print("\tLeft Back: 6 -- Right Back: 7"); Brain.Screen.newLine();
  Brain.Screen.print("Other:"); Brain.Screen.newLine();
  Brain.Screen.print("\tCatapult: 10"); Brain.Screen.newLine();
  //Brain.Screen.print("\tPneumatics: 3WPA"); Brain.Screen.newLine();
}
void brain_smiley() {
  Brain.Screen.clearScreen();
  Brain.Screen.setPenWidth(0);

  Brain.Screen.drawCircle(240,120,100,yellow);//head

  Brain.Screen.drawCircle(240,104,70,white); //mouth
  Brain.Screen.drawRectangle(160,90,160,100,yellow);

  Brain.Screen.drawRectangle(220, 84, 40, 80, orange);//nose

  Brain.Screen.drawCircle(300,144,20,white);//eye 1
  Brain.Screen.drawCircle(290,144,10,black);//pupil 1

  Brain.Screen.drawCircle(180,144,20,white);//eye 2
  Brain.Screen.drawCircle(190,144,10,black);//pupil 2
}
void image_1() {
  Brain.Screen.drawImageFromFile("wonky-lucas.png",0,0);
}
/*---------------------------------------------------------------------------*/
/*                              Ctrllr Graphics                              */
/*---------------------------------------------------------------------------*/
void Ychosen(){
  driveType = 0;
}
void Xchosen() {
  driveType = 1;
}
void Achosen(){
  driveType = 2;
}
void choose_drive_controller() {
  Ctrllr.Screen.setCursor(1,1);
  Ctrllr.Screen.print("Y: Dynamic Drive"); Ctrllr.Screen.newLine();
  Ctrllr.Screen.print("X: Tank Drive"); Ctrllr.Screen.newLine();
  Ctrllr.Screen.print("A: Simple Drive");

  waitUntil(Ctrllr.ButtonY.pressing() || Ctrllr.ButtonX.pressing() || Ctrllr.ButtonA.pressing());
  Ctrllr.ButtonY.released(Ychosen);
  Ctrllr.ButtonX.released(Xchosen);
  Ctrllr.ButtonA.released(Achosen);
}
void Ctrllr_graphics_temps() {
  Ctrllr.Screen.setCursor(2,1);
  Ctrllr.Screen.print("LF F: %d, RF F: %d", (int) left_front.temperature(pct), (int) right_front.temperature(pct));
  Ctrllr.Screen.newLine();
  Ctrllr.Screen.print("LB F: %d, RB F: %d", (int) left_back.temperature(pct), (int) right_back.temperature(pct));
}

/*---------------------------------------------------------------------------*/
/*                          Pre-Autonomous Functions                         */
/*---------------------------------------------------------------------------*/

void vexcodeInit(void) {
}

void pre_auton(void) {
  vexcodeInit(); // DO NOT REMOVE

  catapult.setPosition(0,degrees); //sets default catauplt position

  for(motor Motor: motors) //sets the break mode for each motor
    Motor.setBrake(coast);
  
  brain_display_ports(); //adds graphics to the brain
  //provides useful information to the driver
}


/*---------------------------------------------------------------------------*/
/*                                Drive Controls                             */
/*---------------------------------------------------------------------------*/

int i = 1;
void reverse_drive() {
  i *= -1;
}

double x, y, rx, ry, t;
//axi: 4, 3, 1, 2
void dynamic_drive() {
  Ctrllr.Screen.print("Dynamic Drive");
  /*
    verticle movement on the left joystick is forwards and backwards
    horizontal movement on the right joystick is rotation left and right
    the porportion between the left drive and right drive change dynamically based on the position of the joystick
  */
  y = i*Ctrllr.Axis3.position();
  x = i*Ctrllr.Axis1.position();
  //rx = i*Ctrllr.Axis1.position();

  if(fabs(x) < DEADZONE && fabs(y) < DEADZONE)
    full_drive.stop();
  else {
    t = 2*100/fmax(fabs(x) + fabs(y),100);
    left_drive.spin(fwd, (y - x)*t, pct);
    right_drive.spin(fwd, (y + x)*t, pct);
  }
}
void tank_drive() {
  Ctrllr.Screen.print("Tank Drive");
  /*
    left joystick controls the speed of the left drive
    right joystick controls the speed of the right drive
  */
  y = i*Ctrllr.Axis3.position();
  ry = i*Ctrllr.Axis2.position();

  if(fabs(y) < DEADZONE && fabs(ry) < DEADZONE)
    full_drive.stop(brake);
  else {
    left_drive.spin(fwd, y, pct);
    right_drive.spin(fwd, ry, pct);
  }
}

void simple_drive() {
  Ctrllr.Screen.print("Simple Drive");
  /*
    verticle movement on the joystick is forwards and backwards
    horizontal movement on the joystick is rotation left and right
    the speed is always 100%
  */
  y = i*Ctrllr.Axis3.position();
  x = i*Ctrllr.Axis4.position();

  if(fabs(y) < DEADZONE && fabs(x) < DEADZONE)
    full_drive.stop();
  else if(fabs(y) >= fabs(x)) {
    left_drive.spin(fwd, y*10, pct);
    right_drive.spin(fwd, y*10, pct);
  }
  else {
    left_drive.spin(fwd, -x*10, pct);
    right_drive.spin(fwd, x*10, pct);
  }
}

void drive() {
  //chooses the drive based on the driveType selected
  switch(driveType){
    case 0:
      dynamic_drive(); break;
    case 1:
      tank_drive(); break;
    case 2:
      simple_drive(); break;
    default:
      dynamic_drive(); break;
  }
}

/*---------------------------------------------------------------------------*/
/*                                 Functions                                 */
/*---------------------------------------------------------------------------*/

void run_mandibles(){
  if(Ctrllr.ButtonR1.pressing() == Ctrllr.ButtonR2.pressing())
    //if both or no mandibles are being run they stop
    mandibles.stop(brakeType::hold);
  else if(Ctrllr.ButtonR1.pressing()) { // hold R1 to outtake
    mandibles.spin(directionType::fwd,100,percentUnits::pct);
    Ctrllr.rumble("long");
  }
  else if(Ctrllr.ButtonR2.pressing()) { // hold L1 to intake
    mandibles.spin(directionType::rev,100,percentUnits::pct);
    Ctrllr.rumble("long");
  }
}

void run_catapult() { // needs testing
  catapult.stop(coast);
  wait(800,msec);
  Ctrllr.rumble("short");
  catapult.spinTo(0, degrees, 100, velocityUnits::pct);
}
void continous_catapult() {//goes inside the loop
  //runs the catapult while L2 is pressed
  if(Ctrllr.ButtonL2.pressing()) {
    run_catapult();
    Ctrllr.rumble("long");
  }
}

void pneumatics_open() {
  Ctrllr.rumble("short");
  Pneumatics.open();
}
void pneumatics_close() {
  Pneumatics.close();
}

/*---------------------------------------------------------------------------*/
/*                              User Control Task                            */
/*---------------------------------------------------------------------------*/
/*
Button Listings: 
  UP: Displays ports on the brain
  RIGHT: Allows user to choose the drive on the controller with Y, X & A
  DOWN: Draws smiley face on brain
  LEFT: Allows the user to choose the drive on the brain
  A: Opens the pneumatics
  B: Closes the pneumatics
  L1: Hold to run the catapult continously 
  L2: Reverses the drive
  R1: Ejects Tetraball
  R2: Intakes Tetraball
Joysticks depend on the drive: 
  Dynamic: Left Y for forward and backwards, Right X for rotation
  Tank: Left Y for left drive speed, Right Y for right drive speed
  Simple: Left Joystick for movement and rotation
  
*/
void usercontrol() {
  //Ctrllr Events
  Ctrllr.ButtonUp.pressed(brain_display_ports);
  //Ctrllr.ButtonRight.pressed(choose_drive_controller);
  Ctrllr.ButtonDown.pressed(brain_smiley);
  //Ctrllr.ButtonLeft.pressed(choose_drive_brain);

  //Ctrllr.ButtonA.pressed(pneumatics_open);
  //Ctrllr.ButtonB.pressed(pneumatics_close);
  
  Ctrllr.ButtonL2.pressed(reverse_drive);

  //Continous Events
  while (1) { 
    Ctrllr.Screen.clearScreen();//resets controller screen
    Ctrllr.Screen.setCursor(1,1);

    dynamic_drive(); //rewrite as "drive();" for swapping capabilities 
    run_mandibles(); //hold
    //continous_catapult(); //hold

    //Ctrllr_graphics_temps(); //update
  }
}

/*---------------------------------------------------------------------------*/
/*                            Autonomous Functions                           */
/*---------------------------------------------------------------------------*/

void move(directionType dir, double velocity, int t_msec){
  //direction, speed, milliseconds it runs for
  left_drive.spin(dir,velocity,pct);
  right_drive.spin(dir,velocity,pct);
  wait(t_msec,msec);
  full_drive.stop(brake);
}
void rotate(directionType dir, double velocity, int t_msec) {
  //forward: right and reverse: left, speed, milliseconds it runs for
  left_drive.spin(dir, velocity, pct);
  left_drive.spin(dir, -velocity, pct);
  wait(t_msec, msec);
  full_drive.stop(brake);
}
void curve(directionType dir, double left_velocity, double right_velocity, int t_msec) { 
  //direction, left drive power, right drive power, milliseconds it runs for
  left_drive.spin(dir,left_velocity,pct);
  right_drive.spin(dir,right_velocity,pct);
  wait(t_msec, msec);
  full_drive.stop(brake);
}

void outtake(int t_msec) {
  mandibles.spin(forward,100, pct);
  wait(t_msec, msec);
  mandibles.stop(brakeType::hold);
}
void intake(int t_msec) {
  mandibles.spin(reverse,100,pct);
  wait(t_msec,msec);
  mandibles.stop(brakeType::hold);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              Autonomous Task                              */
/*                                                                           */
/*  1  Removed the Tetraball from the Alliance’sMatch Load Zone that         */
/*     coincides with their Starting Tiles.                                  */
//*                                                                           */
/*  2  Scored at least one Alliance Tetraball in the Alliance’s own Goal.    */
/*                                                                           */
/*  3  Ended the Autonomous Period with at least one Robot contacting their  */
/*     own Elevation Bar.                                                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/* 
Estimated time for rotating 90* with 70 pct
  - 
Estimated time for moving one block with 70 pct
  - 
*/

void auton_right_23() {
  //placed on the right facing the goal
  //first step is to place the alliance triball inside the goal
  //move(forward,80,1200); //forwards
  move(forward,80,1400);
  outtake(2000); //outtake alliance Tetraball for 1 second
  move(reverse, 70, 800); //backwards
  rotate(reverse,60,1250); //90* left (testing needed)
  curve(fwd,50,80,3000); //left:80, right:60
}
void auton_right_123() {
  move(forward,80,1200);
  outtake(2000);
  move(forward,50,500);
  move(reverse, 50, 850);
  rotate(forward,70,1250);
  move(forward,70,1000);
  intake(2000);
  move(reverse,70,3000);
  curve(reverse,100,-50,5000);
}

void auton_left_23() { //basically the reverse of auton right
  move(forward,80,1400);
  outtake(2000); //outtake alliance Tetraball for 1 second
  move(reverse, 70, 900);
  rotate(forward,60,1250);
  curve(fwd,80,50,3000);
}

/*---------------------------------------------------------------------------*/
/*                                    Main                                   */
/*---------------------------------------------------------------------------*/

int main() {
  Competition.autonomous(auton_left_23);
  Competition.drivercontrol(usercontrol);
  pre_auton();

  //while (1)  wait(100, msec);
}