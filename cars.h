#ifndef CARS_H
#define CARS_H

#include "game.h"
#include "objects.h"

class graph;

class BaseCar: public MovingEntity //Base car class
{
protected:
    //Properties
    double FOV_distance; //Radius of FOV
    Angle FOV_angle;    //Angle of FOV
    LineCollider** FOV_collider;
    RadarPoint* FOV_points;
    double acc;         //Acceleration
    double dec;         //Decceleration
    Angle rot_speed;    //Angular speed

    int ray_count;      //Number of radars
    double maneuver_dist;

    //Size properties
    double radius;
    double base_length, base_width;
    double wheels_x_delta, wheels_y_delta;
    double wheels_length, wheels_width;
    double lights_x_delta, lights_y_delta, rear_lights_y_delta;
    double lights_length, lights_width;

    //Flags and timers
    bool is_acc;    //You shouldn't be able accelerate twice in one frame, so this flag is coming in help
    bool is_dec;
    bool is_turn;
    Angle wheels_angle;   //Last angle of wheels


public:

    //Debug info
    Angle target_ui_angle;

    BaseCar(double _x, double _y);
    void Show();    //Drawing
    virtual void ShowRadar() = 0;
    virtual void ShowWheels() = 0;  //Drawing wheels
    virtual void ShowBase() = 0;    //Drawing base
    virtual void ShowLights() = 0;  //Drawing lights
    void ResetFlags();  //Reset all flags (should be called on start of a new frame)

    void ShowFOV(QPainter* pic_pntr, double _x, double _y); //Draw FOV

    void Accelerate();   //Full acceleration
    void Accelerate(double _acc);    //Acceleration by _acc amount (or full, if _acc > acc)

    void Deccelerate(); //Full decceleration
    void Deccelerate(double _dec);  //Acceleration by _dec amout (or full, if _dec > dec)

    void Rotate(Angle delta_angle);

    void RotateTo(Angle target_angle, DIRECTION dir = NODIRECTION);   //Rotating to specified angle and specified direction

    void RotateL();
    void RotateL(const Angle &delta_angle);
    void RotateR();
    void RotateR(const Angle& delta_angle);

    void updateProperties(int ray_count, double ray_dist, Angle fov_angle, double d);

};

class Path;

class Car: public BaseCar
{
public:
    Car(double _x, double _y);
    ~Car();

    Path* path;

    ObstacleMap map;
    int map_timer;

    void OnStep();

    void ShowRadar();
    void ShowWheels();
    void ShowBase();
    void ShowLights();

    void graph_to_path(graph* gr, uint target = 1);  //Building path from graph
    void ShowPath();    //Drawing path
    void FollowPath();  //Following path
    double GetPathMaxSpeed();

    QString GetName();

    //[DEPRECATED functions]
    void RideTo(Box* obstacle); //[TEST] Driving over box
    void BuildPath(double tx, double ty);  //Creating simple path
    void BuildPath(double tx, double ty, Box* obstacle);    //[ТЕСТ] Driving over box
};

#endif // CARS_H
