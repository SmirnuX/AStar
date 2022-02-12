#include "add_math.h"

//=== Trigonometry ===
double degtorad(double angle)  //Convert degrees to radians
{
    return angle * M_PI / 180.0;
}

double radtodeg(double angle)  //Convert radians to degrees
{
    return angle * 180 / M_PI;
}

double direction_to_point(double from_x, double from_y, double to_x, double to_y) //Calculate driection from (from_x, from_y) to (to_x, to_y)
{
    double dir = safe_acos( (to_x - from_x) / sqrt(distance2(from_x, from_y, to_x, to_y)));
    if (to_y < from_y)
        dir = 2* M_PI - dir;
    return dir;
}

double anglediff(double a, double b)    //Angle difference in range from -180 to 180
{
    if (a < 0)
        a += 360;
    if (a >= 360)
        a -= 360;
    if (b < 0)
        b += 360;
    if (b >= 360)
        b -= 360;
    double da = a - b;
    if (da < 0)
        da += 360;
    if (da >= 360)
        da -= 360;
    if (da > 180)
        da = da - 360;
    return da;
}


//=== Safe trigonometry functions ===
double safe_acos(double ang)    //If safe_acos is called with cos > 1 or < -1, it converts it to nearest right value
{
    if (ang > 1)
        ang = 0.999;
    if (ang < -1)
        ang = -0.999;
    double res = acos(ang);
    if (res < 0)
        res += 2 * M_PI;
    if (res > 2*M_PI)
        res -= 2*M_PI;
    return res;
}

double safe_asin(double ang)
{
    if (ang > 1)
        ang = 0.999;
    if (ang < -1)
        ang = -0.999;
    double res = asin(ang);
    if (res < 0)
        res += 2 * M_PI;
    if (res > 2*M_PI)
        res -= 2*M_PI;
    return res;
}


//=== Distance ==
double distance2(double ax, double ay, double bx, double by)  //Square distance between points
{
    return (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
}

double distance2(Point a, Point b)
{

    return (b.GetX()-a.GetX())*(b.GetX()-a.GetX()) + (b.GetY()-a.GetY())*(b.GetY()-a.GetY());
}

double distance(double ax, double ay, double bx, double by)  //Distance between points
{
    return sqrt(distance2(ax, ay, bx, by));
}

double distance(Point a, Point b)
{

    return sqrt(distance2(a, b));
}


//=== Simple helper functions ===
double sign(double a)   //1 if a >=0, -1 if a<0
{
    if (a >= 0)
        return 1;
    else
        return -1;
}

bool almostEq(double a, double b, double eps)  //Are delta between a and b lesser then eps
{
    if (a > b - eps && a < b + eps)
        return true;
    else
        return false;
}

bool intersect(double a1, double a2, double b1, double b2) //Does (a1,a2) and (b1, b2) intersect.
{
    double min_a, max_a, min_b, max_b;
    if (a1 < a2)
    {
        min_a = a1;
        max_a = a2;
    }
    else
    {
        min_a = a2;
        max_a = a1;
    }

    if (b1 < b2)
    {
        min_b = b1;
        max_b = b2;
    }
    else
    {
        min_b = b2;
        max_b = b1;
    }

    double max_of_min = (min_a > min_b) ? min_a : min_b;
    double min_of_max = (max_a < max_b) ? max_a : max_b;

    return max_of_min < min_of_max;
}

Point* intersect2d(double a1, double b1, double c1, double a2, double b2, double c2) //Point where lines intersect (or nullptr if there isn't one or several. Additional check is to compare c1 and c2)
{
    //Solution of system of linear equations : X = A^(-1)xB
    Matrix A(2, 2);
    A.SetElem(a1, 0, 0);
    A.SetElem(b1, 0, 1);
    A.SetElem(a2, 1, 0);
    A.SetElem(b2, 1, 1);
    //Checking determinant
    double _Det = A.det();
    if (almostEq(_Det, 0))
    {
        return nullptr;
    }
    Matrix C(2, 1);
    C.SetElem(-c1, 0, 0);
    C.SetElem(-c2, 1, 0);
    Matrix A_inv = A.inverse();
    Matrix res = A_inv * C;
    double res_x = res.GetElem(0, 0);
    double res_y = res.GetElem(0, 1);
    return new Point(res_x, res_y);
}


void DrawTurnedRect(QPainter* painter, double x, double y, double angle, double width, double height)   //Draw turned rectangle
{
    QPoint points[4];
    double diag_angle = atan(height/width);
    double diag = sqrt(width*width + height*height)/2;
    double sinus = sin(diag_angle + degtorad(angle));
    double cosinus = cos(diag_angle + degtorad(angle));
    double r_sinus = sin(M_PI - diag_angle + degtorad(angle));
    double r_cosinus = cos(M_PI - diag_angle + degtorad(angle));
    points[0] = QPoint(x + cosinus * diag, y - sinus * diag);
    points[1] = QPoint(x + r_cosinus * diag, y - r_sinus * diag);
    points[2] = QPoint(x - cosinus * diag, y + sinus * diag);
    points[3] = QPoint(x - r_cosinus * diag, y + r_sinus * diag);
    painter->drawConvexPolygon(points, 4);
}


//=== Angle class realization ===
Angle::Angle()  //Uninitialized angle
{

}

Angle::Angle(double rad)
{
    angle = rad;
    CorrectAngle();
}

void Angle::CorrectAngle()   //Correct angle to be in range from 0 to 2PI
{
    while (angle < 0 || angle >= 2*M_PI)
    {
        if (angle < 0)
            angle += 2*M_PI;
        else if (angle >= 2*M_PI)
            angle -= 2*M_PI;
    }
}

double Angle::GetR() const  //Get angle in radians
{
    return angle;
}

double Angle::GetD() const  //Get angle in degrees
{
    return radtodeg(angle);
}

Angle Angle::normalL() const   //Get left normal
{
    Angle a = Angle(GetR() + M_PI/4);
    return a;
}

Angle Angle::normalR() const  //Get right normal
{
    Angle a = Angle(GetR() - M_PI/4);
    return a;
}

Angle& Angle::operator+=(Angle& a)
{
    angle += a.GetR();
    CorrectAngle();
    return *this;
}

Angle& Angle::operator+=(double a)
{
    angle += a;
    CorrectAngle();
    return *this;
}

Angle& Angle::operator-=(Angle& rad)
{
    *this += -rad.GetR();
    return *this;
}

Angle& Angle::operator-=(double a)
{
    *this += -a;
    return *this;
}

Angle& Angle::operator=(Angle& right)
{
    angle = right.GetR();
    CorrectAngle();
    return *this;
}

Angle& Angle::operator=(double rad)
{
    angle = rad;
    CorrectAngle();
    return *this;
}

Angle operator+(Angle& left, Angle& right)
{
    Angle temp = left.GetR();
    temp += right;
    return temp;
}

Angle operator-(Angle& left, Angle& right)
{
    Angle temp = left.GetR();
    temp -= right;
    return temp;
}

bool operator==(Angle& left, Angle& right)
{
    return almostEq(left.GetR(), right.GetR());
}

bool operator!=(Angle& left, Angle& right)
{
    return !(left.GetR() == right.GetR());
}

bool operator<(Angle& left, Angle& right)    //Checks, if smaller angle between left and right is negative
{
    return ((left-right).GetR() > M_PI);
}

bool operator>(Angle& left, Angle& right)
{
    return ((left-right).GetR() < M_PI);
}


//=== Matrix class realization ===

Matrix::Matrix(int _h, int _w, double** data)    //Creating matrix, if h or w = -1 -> matrix is uninitialized
{
    if (_h >= 0 && _w >= 0)
    {
        h = _h;
        w = _w;
        matrix = new double* [h];
        for (int i = 0; i < h; i++)
        {
            matrix[i] = new double [w];
            if (data == nullptr)
            {
                continue;
            }
            for (int j = 0; j < w; j++)
            {
                matrix[i][j] = data[i][j];
            }
        }
    }
    else
    {
        h = -1;
        w = -1;
        matrix = nullptr;
    }
}

Matrix::~Matrix()
{
    clear();
}

double Matrix::det()   //Determinant of matrix
{
    if (h < 1 || w < 1)
    {
        throw std::runtime_error("Tried to calculate determinant of uninitialized matrix.");
    }
    else if (h != w)
    {
        throw std::runtime_error("Tried to calculate determinant of not square matrix.");
    }
    if (h == 1) //For 1x1
    {
        return matrix[0][0]; //Return single element
    }
    if (h == 2) //For 2x2
    {
        return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
    }
    //For bigger matrices
    double res = 0;
    double sign = 1;  //Sign of minor
    for (int i = 0; i < w; i++)
    {
        res += sign*matrix[0][i] * getAddition(0, i).det();
        sign = -sign;
    }
    return res;
}

Matrix Matrix::getAddition(int _i, int _j)   //Get new matrix by deleting i row and j column
{
    Matrix result = Matrix(h-1, w-1);
    for (int i = 0; i < h; i++)
    {
        if (i == _i)
        {
            continue;
        }
        for (int j = 0; j < w; j++)
        {
            if (j == _j)
            {
                continue;
            }
            int new_i = (i > _i) ? i - 1 : i;
            int new_j = (j > _j) ? j - 1 : j;
            result.SetElem(matrix[i][j], new_i, new_j);
        }
    }
    return result;
}

Matrix Matrix::inverse()  //Inverse matrix
{
    double _det = det();
    if (_det == 0)
    {
        throw std::runtime_error("Tried to calculate inverse matrix for matrix with zero determinant.");
    }
    Matrix result = Matrix(h, w);
    double sign = 1;  //Sign of minor
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            double new_elem = sign * getAddition(0, i).det();
            new_elem /= _det;
            result.SetElem(new_elem, i, j);
            sign = -sign;
        }
    }
    return result;

}

double Matrix::GetElem(int i, int j)   //Get element from i row and j column
{
    if (i < 0 || i >= h || j < 0 || j >= w)
    {
        throw std::runtime_error("Out of matrix bounds.");
    }
    return matrix[i][j];
}

int Matrix::GetH()
{
    return h;
}

int Matrix::GetW()
{
    return w;
}

void Matrix::SetElem(double x, int i, int j)
{
    if (i < 0 || i >= h || j < 0 || j >= w)
    {
        throw std::runtime_error("Out of matrix bounds.");
    }
    matrix[i][j] = x;
}

Matrix& Matrix::operator=(Matrix& mx)
{
    clear();
    h = mx.h;
    w = mx.w;
    if (h >= 0 && w >= 0)
    {
        matrix = new double* [h];
        for (int i = 0; i < h; i++)
        {
            matrix[i] = new double [w];
            for (int j = 0; j < w; j++)
            {
                matrix[i][j] = mx.GetElem(i, j);
            }
        }
    }
    else
    {
        h = -1;
        w = -1;
        matrix = nullptr;
    }
    return *this;
}

void Matrix::clear()
{
    if (h >= 0 && w >= 0 && matrix != nullptr)
    {
        for (int i = 0; i < h; i++)
        {
            delete[] matrix;
        }
        delete[] matrix;
        w = -1;
        h = -1;
    }
}

Matrix& Matrix::operator+=(Matrix& mx)
{
    if (h != mx.GetH() || w != mx.GetW())
    {
        throw std::runtime_error("Tried to sum matrices with different sizes");
    }
    if (h < 1 || w < 1)
    {
        return *this;
    }
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            matrix[i][j] += mx.GetElem(i, j);
        }
    }
    return *this;
}

Matrix& Matrix::operator-=(Matrix& mx)
{
    if (h != mx.GetH() || w != mx.GetW())
    {
        throw std::runtime_error("Tried to sub matrices with different sizes");
    }
    if (h < 1 || w < 1)
    {
        return *this;
    }
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            matrix[i][j] -= mx.GetElem(i, j);
        }
    }
    return *this;
}

Matrix& Matrix::operator*=(double coef)
{
    if (h < 1 || w < 1)
    {
        return *this;
    }
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            matrix[i][j] *= coef;
        }
    }
    return *this;
}

bool operator==(Matrix& left, Matrix& right)
{
    if(left.GetW() != right.GetW() || left.GetH() != right.GetH())
    {
        return false;
    }
    if (left.GetW() < 1 && left.GetH() < 1)
    {
        return true;
    }
    for (int i = 0; i < left.GetH(); i++)
    {
        for (int j = 0; j < left.GetW(); j++)
        {
            if (left.GetElem(i, j) != right.GetElem(i, j))
            {
                return false;
            }
        }
    }
    return true;
}


Matrix operator+(Matrix& left, Matrix& right)
{
    if (left.GetH() != right.GetH() || left.GetW() != right.GetW())
    {
        throw std::runtime_error("Tried to sum matrices with different sizes");
    }
    Matrix res(left.GetH(), left.GetW());
    for (int i = 0; i < left.GetH(); i++)
    {
        for (int j = 0; j < left.GetW(); j++)
        {
            res.SetElem(left.GetElem(i, j) + right.GetElem(i, j), i, j);
        }
    }
    return res;
}

Matrix operator-(Matrix& left, Matrix& right)
{
    if (left.GetH() != right.GetH() || left.GetW() != right.GetW())
    {
        throw std::runtime_error("Tried to sum matrices with different sizes");
    }
    Matrix res(left.GetH(), left.GetW());
    for (int i = 0; i < left.GetH(); i++)
    {
        for (int j = 0; j < left.GetW(); j++)
        {
            res.SetElem(left.GetElem(i, j) - right.GetElem(i, j), i, j);
        }
    }
    return res;
}


Matrix operator*(Matrix& left, Matrix& right)
{
    if (left.GetH() < 1 || left.GetW() < 1 || right.GetH() < 1 || right.GetW() < 1)
    {
        throw std::runtime_error("Tried to multiply empty matrix");
    }
    if (left.GetW() != right.GetH())
    {
        throw std::runtime_error("Tried to mul matrices, with different width (of left) and height (of right)");
    }
    Matrix res(left.GetH(), right.GetW());
    for (int i = 0; i < left.GetH(); i++)
    {
        for (int j = 0; j < right.GetW(); j++)
        {
            double sum = 0;
            for (int k = 0; k < right.GetH(); k++)
            {
                sum+=left.GetElem(i, k) * right.GetElem(k, j);
            }
            res.SetElem(sum, i, j);
        }
    }
    return res;
}

Matrix operator*(Matrix& left, double right)
{
    Matrix res(left.GetH(), left.GetW());
    for (int i = 0; i < left.GetH(); i++)
    {
        for (int j = 0; j < left.GetW(); j++)
        {
            res.SetElem(left.GetElem(i, j) * right, i, j);
        }
    }
    return res;
}
