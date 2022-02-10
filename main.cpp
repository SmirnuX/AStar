#include "game.h"

//-------Глобальные переменные-------
QPixmap* picture;   //Буфер. Изначально вся графика отрисовывается на него, а потом уже сам буфер отрисовывается в окне для предотвращния мерцания
EntityStack* stack;
bool SHOW_COLLIDERS;    //Показывать границы столкновений. Переключение на F1

int main(int argc, char *argv[])
{
    SHOW_COLLIDERS = false;
    int width=1900, height=1000; //Размеры окна
    QApplication a(argc, argv);
    QPixmap pic(width, height);
    pic.fill(QColor(255,255,255)); //Закраска фона
    picture=&pic;
    game window(width,height);
    window.show();
    return a.exec();
}
