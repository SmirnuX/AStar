#include "graph_struct.h"
#include "game.h"

//=== ObstacleMap class realization ===

ObstacleMap::ObstacleMap()
{
    //Empty body
}

ObstacleMap::~ObstacleMap()
{
    //Empty body
}

void ObstacleMap::AddPoint(const Point& pt, int d)
{
    Line ln;
    double min_dist = d;
    int min_i = -1;
    int min_j = -1;


    const double VISION_EPSILON = 8;

    for (int i = 0; i < obstacles.size(); i++)
    {
        for (int j = 0; j < obstacles[i].size(); j++)
        {
            if (    distance(pt.GetX(), pt.GetY(), obstacles[i][j].GetX(),
                         obstacles[i][j].GetY()) < VISION_EPSILON)
            {
                return; //Skipping points, that are too close to already existing
            }
        }
    }

    //Check distance to existing lines
    for (int i = 0; i < obstacles.size(); i++)
    {
        for (int j = 1; j < obstacles[i].size(); j++)
        {
            ln.Set(obstacles[i][j-1].GetX(), obstacles[i][j-1].GetY(),
                    obstacles[i][j].GetX(), obstacles[i][j].GetY());
            double dist = ln.DistTo(pt);
            if (dist < VISION_EPSILON) //If point on same line
            {
                if (pt.GetX() > ln.GetMinX() - EPSILON && pt.GetX() < ln.GetMaxX() + EPSILON &&
                        min(ln.GetMinY(), ln.GetMaxY()) - EPSILON < pt.GetY() && max(ln.GetMinY(), ln.GetMaxY()) + EPSILON > pt.GetY())
                    return; //Excessive point - skip
                if (distance(pt.GetX(), pt.GetY(), obstacles[i][j-1].GetX(), obstacles[i][j-1].GetY()) < d) //Extend line
                {
                    if (distance2(pt.GetX(), pt.GetY(), obstacles[i][j].GetX(), obstacles[i][j].GetY()) >
                         distance2(obstacles[i][j-1].GetX(), obstacles[i][j-1].GetY(), obstacles[i][j].GetX(), obstacles[i][j].GetY()))
                    {
                        obstacles[i][j-1].MoveTo(pt.GetX(), pt.GetY());
                        ToAlpha(i, d);  //TOALPHA
                        return;
                    }
                }
                if (distance(pt.GetX(), pt.GetY(), obstacles[i][j].GetX(), obstacles[i][j].GetY()) < d) //Extend in opposite direction
                {
                    if (distance2(pt.GetX(), pt.GetY(), obstacles[i][j-1].GetX(), obstacles[i][j-1].GetY()) >
                         distance2(obstacles[i][j-1].GetX(), obstacles[i][j-1].GetY(), obstacles[i][j].GetX(), obstacles[i][j].GetY()))
                    {
                        obstacles[i][j].MoveTo(pt.GetX(), pt.GetY());
                        ToAlpha(i, d);  //TOALPHA
                        return;
                    }
                }
            }
            else if (dist < d)
            {
                if (dist < min_dist &&
                        (distance(pt.GetX(), pt.GetY(), obstacles[i][j].GetX(), obstacles[i][j].GetY()) < d ||
                         distance(pt.GetX(), pt.GetY(), obstacles[i][j-1].GetX(), obstacles[i][j-1].GetY()) < d))
                {
                    min_dist = dist;
                    min_i = i;
                    min_j = j;
                }
            }
        }
    }

    if (min_i != -1 && min_j != -1)
    {
        obstacles[min_i].insert(obstacles[min_i].begin() + min_j - 1, pt);
        ToAlpha(min_i, d);  //TOALPHA
        return;
    }

    //Check distance to end points of obstacles
    for (int i = 0; i < obstacles.size(); i++)
    {
        if (distance(pt.GetX(), pt.GetY(), obstacles[i][0].GetX(),
                     obstacles[i][0].GetY()) < min_dist)
        {
            min_dist = distance(pt.GetX(), pt.GetY(), obstacles[i][0].GetX(),
                    obstacles[i][0].GetY());
            min_i = i;
            min_j = 0;
        }
        if (distance(pt.GetX(), pt.GetY(), obstacles[i][obstacles.size()-1].GetX(),
                     obstacles[i][obstacles.size()-1].GetY()) < min_dist)
        {
            min_dist = distance(pt.GetX(), pt.GetY(), obstacles[i][obstacles.size()-1].GetX(),
                    obstacles[i][obstacles.size()-1].GetY());
            min_i = i;
            min_j = obstacles[i].size();
        }
    }

    if (min_i != -1 && min_j != -1)
    {
        if (min_j == 0)
            obstacles[min_i].insert(obstacles[min_i].begin(), pt);
        else
            obstacles[min_i].push_back(pt);
        ToAlpha(min_i, d);  //TOALPHA
        return;
    }
    //Create new obstacle
    obstacles.push_back(std::vector<Point> {pt});
}

void ObstacleMap::DeletePoint(Point& pt)
{

}

int ObstacleMap::Size()
{
    return obstacles.size();
}

obstacle ObstacleMap::GetObstacle(int i, double r)
{
    PolygonCollider poly(obstacles[i]);
    return poly.GetOutline(r);
}

void ObstacleMap::Show()
{
    QPainter pntr(picture);
    QPen penn;
    penn.setWidth(2);


    for (int i = 0; i < obstacles.size(); i++)
    {
        QColor col;
        col.setHsl(50*i%255, 255, 120);
        penn.setColor(col);
        pntr.setPen(penn);
        pntr.setBrush(QBrush(col));
        for (int j = 0; j < obstacles[i].size(); j++)
        {
            pntr.drawEllipse(obstacles[i][j].GetX() - 3, obstacles[i][j].GetY() - 3, 6, 6);
            if (j < obstacles[i].size()-1)
                pntr.drawLine(obstacles[i][j].GetX(), obstacles[i][j].GetY(),
                              obstacles[i][j+1].GetX(), obstacles[i][j+1].GetY());
            else
                pntr.drawLine(obstacles[i][j].GetX(), obstacles[i][j].GetY(),
                              obstacles[i][0].GetX(), obstacles[i][0].GetY());

        }
    }
}

std::vector<int> GetConvexHull(std::vector<Point>&pts, int beg, int end)
{
    std::vector<int> hull;  //Graham algorithm
    //Looking for point with min y
    int min_pt = beg;
    for (int i = beg+1; i <= end; i++)
    {
        if (pts[i].GetY() < pts[min_pt].GetY())
            min_pt = i;
        else if (pts[i].GetY() < pts[min_pt].GetY())    //Small probability but
        {
            if (pts[i].GetX() < pts[min_pt].GetX())
                min_pt = i;
        }
    }

    std::vector<int> sorted;
    for (int i = beg; i <= end; i++)
    {
        if (i != min_pt)
            sorted.push_back(i);
    }
    std::sort(sorted.begin(), sorted.end(), [pts, min_pt](int a, int b) {
        return direction_to_point(pts[min_pt].GetX(), pts[min_pt].GetY(), pts[a].GetX(), pts[a].GetY()) <
                direction_to_point(pts[min_pt].GetX(), pts[min_pt].GetY(), pts[b].GetX(), pts[b].GetY());
    });

    hull.push_back(min_pt);
    hull.push_back(sorted[0]);
    for (int i = 1; i < sorted.size(); i++)
    {
        while(true)
        {
            Point A = pts[hull[hull.size()-2]];
            Point B = pts[hull[hull.size()-1]];
            Point C = pts[sorted[i]];
            double mul = (B.GetX() - A.GetX()) * (C.GetY() - B.GetY()) - (B.GetY() - A.GetY()) * (C.GetX() - B.GetX());
            if (mul >= 0)
                break;
            hull.pop_back();
        }
        hull.push_back(sorted[i]);
    }
    return hull;
}

void ObstacleMap::ToAlpha(int i, int d)
{
    std::vector<Point> result = obstacles[i];
    if (result.size() < 3)
        return;
//    std::sort(result.begin(), result.end(), [](Point a, Point b) {return a.GetX() < b.GetX();});    //Sort by x-value

//    auto res = Merge(result, 0, result.size()-1);
    auto res = GetConvexHull(result, 0, result.size()-1);
    std::vector<Point> opt;
    for (int i = 0; i < res.size(); i++)
    {
        opt.push_back(result[res[i]]);
    }
    obstacles[i] = opt;
}

