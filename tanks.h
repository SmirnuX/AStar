#ifndef TANKS_H
#define TANKS_H

//Tanks and projectiles

#include "game.h"
#include "objects.h"

class graph;

class BaseTank: public MovingEntity //Base tank class
{
protected:
    //Variables
    Angle cannon_angle; //Rotation of tank head
    //Properties
    double FOV_distance; //Radius of FOV
    Angle FOV_angle;    //Angle of FOV
    double tank_speed;  //Max tank speed
    double acc;         //Acceleration of tank
    double dec;         //Decceleration of tank
    double friction;    //Friction on tank
    Angle rot_speed;   //Speed of rotating tank head
    int rel_time;       //Reload timeout
    //Base properties
    int base_width;     //Width
    int base_length;    //length
    int cannon_width;   //Cannon width
    int cannon_length;  //Cannon length
public:
    Collider* FOV_collider;
    double reload_timeout;   //Timeout before next shot
    BaseTank(double _x, double _y);
    void Show();    //Drawing tank
    virtual void ShowTracks() = 0;  //Drawing tracks
    virtual void ShowBase() = 0;    //Drawing base
    virtual void ShowCannon() = 0;  //Drawing of head

    void ShowFOV(QPainter* pic_pntr, double _x, double _y); //Draw FOV

    virtual void Shoot() = 0;
    void SetCannonAngle(Angle _angle);  //Setting angle of cannon

    void Accelerate();   //Full acceleration
    void Accelerate(double _acc);    //Acceleration by _acc amount

    void Deccelerate();
    void Deccelerate(double _dec);

    void Rotate(Angle delta_angle);   //Rotating
    void RotateL();
    void RotateR();
};

class Path;

class Tank: public BaseTank
{
public:
    Tank(double _x, double _y);
    ~Tank();

    Path* path;

    void Shoot();
    void OnStep();

    void ShowTracks();
    void ShowBase();
    void ShowCannon();

    void RideTo(Box* obstacle); //[TEST] Driving over box

    void BuildPath(double tx, double ty);  //Creating simple path
    void BuildPath(double tx, double ty, Box* obstacle);    //[ТЕСТ] Driving over box
    void graph_to_path(graph* gr, uint target = 1);  //Building path from graph
    void ShowPath();    //Drawing path
    void FollowPath();  //Following path

    QString GetName();
};

class EnemyTank: public BaseTank
{
public:
    EnemyTank(double _x, double _y);
    ~EnemyTank();
    void Shoot();
    void OnStep();
    void ShowTracks();
    void ShowBase();
    void ShowCannon();

    QString GetName();
};

class BaseBullet: public MovingEntity
{
protected:
    Entity* parent;
public:
    BaseBullet(float _x, float _y, Entity* _parent = NULL);
};

class Bullet: public BaseBullet
{
public:
    Bullet(float _x, float _y, Angle _angle, Entity* _parent = NULL);
    void Show();
    void OnStep();


    QString GetName();
};

#endif // TANKS_H
