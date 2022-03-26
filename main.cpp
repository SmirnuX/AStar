#include "game.h"

//-------Global variables-------
QPixmap* picture;   //Image buffer
EntityStack* stack;
QFile* path_log;
bool SHOW_COLLIDERS;    //Show collision masks

int main(int argc, char *argv[])
{
    SHOW_COLLIDERS = false;
    int width=1900, height=1000; //Dimensions of window
    QApplication a(argc, argv);
    QPixmap pic(width, height);
    pic.fill(QColor(255,255,255)); //Filling background
    picture=&pic;
    game window(width,height);
    window.show();
    return a.exec();
}
