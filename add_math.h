#ifndef ADD_MATH_H
#define ADD_MATH_H

constexpr double EPSILON = 0.001;

#include <math.h>
#include "objects.h"
#include "collision.h"

//Trigonometry
double degtorad(double angle);  //Convert degrees to radians
double radtodeg(double angle);  //Convert radians to degrees

double direction_to_point(double from_x, double from_y, double to_x, double to_y);  //Calculate driection from (from_x, from_y) to (to_x, to_y)
double anglediff(double a, double b);    //Angle difference in range from -180 to 180

double safe_acos(double cos);   //If safe_acos is called with cos > 1 or < -1, it converts it to nearest right value
double safe_asin(double sin);


double distance2(double ax, double ay, double bx, double by);  //Square distacnce between points
double distance2(Point a, Point b);

double distance(double ax, double ay, double bx, double by);  //Distance between points
double distance(Point a, Point b);


double sign(double a);  //1 if a >=0, -1 if a<0
bool almostEq(double a, double b, double eps = EPSILON);  //Are delta between a and b lesser then eps
bool intersect(double a1, double a2, double b1, double b2); //Does (a1,a2) and (b1, b2) intersect.
Point* intersect2d(double a1, double b1, double c1, double a2, double b2, double c2); //Point where lines intersect (or nullptr if there isn't one or several. Additional check is to compare c1 and c2)

void DrawTurnedRect(QPainter* painter, double x, double y, double angle, double width, double height);

//Angle class
class Angle
{
private:
    double angle;   //Angle in radians between 0 and 2PI
public:

    Angle();            //Uninitialized angle
    Angle(double rad);

    void CorrectAngle();    //Correct angle to be in range from 0 to 2PI

    double GetR() const;    //Get angle in radians
    double GetD() const ;   //Get angle in degrees

    Angle normalL() const;   //Get left normal
    Angle normalR() const;   //Get right normal

    Angle& operator+=(Angle& a);
    Angle& operator+=(double rad);

    Angle& operator-=(Angle& a);
    Angle& operator-=(double rad);

    Angle& operator=(Angle& right);
    Angle& operator=(double rad);
};

Angle operator+(Angle& left, Angle& right);
Angle operator-(Angle& left, Angle& right);

bool operator==(Angle& left, Angle& right);
bool operator!=(Angle& left, Angle& right);
bool operator<(Angle& left, Angle& right);  //Checks, if smaller angle between left and right is negative
bool operator>(Angle& left, Angle& right);


//Matrix class
class Matrix
{
private:
    double** matrix;
    int h;
    int w;
public:
    Matrix(int h = -1, int w = -1, double** data = nullptr);    //Creating matrix, if h or w = -1 -> matrix is uninitialized
    ~Matrix();
    double det();   //Determinant of matrix
    Matrix getAddition(int _i, int _j);   //Get new matrix by deleting i row and j column
    Matrix inverse();   //Inverse matrix
    double GetElem(int i, int j);   //Get element from i row and j column
    int GetH();
    int GetW();
    void SetElem(double x, int i, int j);
    void clear();

    Matrix& operator=(Matrix& mx);
    Matrix& operator+=(Matrix& mx);
    Matrix& operator-=(Matrix& mx);
    Matrix& operator*=(double coef);
};

bool operator==(Matrix& left, Matrix& right);
Matrix operator+(Matrix& left, Matrix& right);
Matrix operator-(Matrix& left, Matrix& right);
Matrix operator*(Matrix& left, Matrix& right);
Matrix operator*(Matrix& left, double right);

#endif // ADD_MATH_H
