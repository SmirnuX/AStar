#include "game.h"
extern QPixmap* picture;
extern EntityStack* stack;

//-------Реализация функций класса Game-------

game::game(int w, int h, QWidget *parent)   //Создание игрового окна
    : QMainWindow(parent)
{
    target_x = 0;
    target_y = 0;
    SHOW_COLLIDERS = true;
    SHOW_PATH = true;
    UI_ACTIVE = false;
    Menu = new Ui_DebugMenu(this);
    Menu->setupUi();
    connect(Menu->Search, SIGNAL(clicked()), this, SLOT(player_set_path()));
    Menu->hide();
    PAUSE = false;
    width = w;
    height = h;
    resize(width, height);
    for(int i=0;i<6;i++)    //Инициализируем буфер ввода
        key[i]=false;
    //Создание танка
    player = new Tank(500, 500);
    //EnemyTank* enemy = new EnemyTank(200, 200);
    box = new Box(1000, 500);
    Box* box1 = new Box(1200, 700);
    Box* box2 = new Box(500, 200);
    //Создание стека сущностей
    stack = new EntityStack();
    stack->Add(player);
    stack->Add((Entity*) new Wall(1200, 500));
    stack->Add((Entity*) new EnemyTank(200, 200));   //Вражеский танк

    stack->Add(box);   //Объезжаемый квадрат
    stack->Add(box1);   //Объезжаемый квадрат
    stack->Add(box2);   //Объезжаемый квадрат
    //Создание стека видимых сущностей
    visible = new EntityStack();
    visible->Add(box);   //Объезжаемый квадрат
    visible->Add(box1);   //Объезжаемый квадрат
    visible->Add(box2);   //Объезжаемый квадрат

    //Создание таймера, запускающего _update() раз в 15 мс.
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(game_update()));
    timer->start(15);
    player->BuildPath(0, 0);
    path_graph = new graph();

    uiUpdate();
}

game::~game()
{
    //TODO - сделать очистку памяти
}

bool game::event(QEvent* ev)   //Обработчик событий
{
    if (ev->type() == QEvent::KeyPress)    //Рассматриваем события нажатия кнопки
    {
        QKeyEvent *ke = (QKeyEvent*)ev;
        int keycode = ke->key();    //Получаем код кнопки
        switch(keycode) //TODO - переделать с помощью Enum и цикла
        {
        case Qt::Key_W:
        case Qt::Key_Up:
            key[0]=true;
            break;
        case Qt::Key_A:
        case Qt::Key_Left:
            key[1]=true;
            break;
        case Qt::Key_S:
        case Qt::Key_Down:
            key[2]=true;
            break;
        case Qt::Key_D:
        case Qt::Key_Right:
            key[3]=true;
            break;
        case Qt::Key_Space:
            key[4]=true;    //Стрельба
            break;
        case Qt::Key_Control:
            key[5]=true;
            break;
        case Qt::Key_F1:
            {
                UI_ACTIVE = !UI_ACTIVE;
                if (UI_ACTIVE)
                    Menu->show();
                else
                {
                    Menu->hide();
                    this->setFocus();
                }

            }
            break;
        }
        return true;
    }
    else if (ev->type() == QEvent::KeyRelease) //Отпускание кнопки
    {
        QKeyEvent *ke = (QKeyEvent*)ev;
        int keycode = ke->key();    //Получаем код кнопки
        switch(keycode)
        {
        case Qt::Key_W:
        case Qt::Key_Up:
            key[0]=false;
            break;
        case Qt::Key_A:
        case Qt::Key_Left:
            key[1]=false;
            break;
        case Qt::Key_S:
        case Qt::Key_Down:
            key[2]=false;
            break;
        case Qt::Key_D:
        case Qt::Key_Right:
            key[3]=false;
            break;
        case Qt::Key_Space:
            key[4]=false;   //Стрельба
            break;
        case Qt::Key_Control:
            key[5]=false;
            break;
        //case Qt::Key_F1:
            //key[5]=false;
        }
        return true;
    }
    return QWidget::event(ev);  //Если произошло другое событие, пропускаем обработку
};

void game::mousePressEvent(QMouseEvent *event)    //Обработка событий мыши
{
    if (event->button() == Qt::RightButton) {   //Поворот башни танка
        double mouse_angle = radtodeg( atan( ( - event->pos().y() + player->GetY()) / (event->pos().x() - player->GetX()) ) );
        if (event->pos().x() < player->GetX())
            mouse_angle += 180;
        player->SetCannonAngle(mouse_angle - player->GetAngle());
    }
    else if (event->button() == Qt::LeftButton)
    {
        target_x = event->pos().x();
        target_y = event->pos().y();
    }
}

void game::paintEvent(QPaintEvent *) //Отрисовка буфера в окне
{
    QPainter pntr(this);
    pntr.drawPixmap(0,0,width,height,*picture,0,0,width,height);
    //UI

    pntr.setPen(QColor(0,0,0));
    pntr.drawText(10, 10, "Number of objects:\t " + QString::number(stack->size));    //Количество обьектов
    pntr.drawText(10, 20, "Press F1 to open debug menu");
    if (player->reload_timeout == 0)
        pntr.drawText(10, 30, "Cannon ready");

}

void game::game_update()  //Функция, вызывающаяся каждый такт
{
    bool toBuild = false;
    picture->fill();
    //double acc = 0.3; //Ускорение
    //double delta_angle = 2; //Поворот
    //Управление игроком
    if(key[0])   //кнопка W - газ
    {
        //player->SetSpeed(player->GetSpeed() + acc);
        player->Accelerate();
    }
    if(key[1])   //кнопка A - поворот влево
    {
        //player->Turn(delta_angle);
        player->RotateL();
    }
    if(key[2])   //кнопка S - поворот вправо
    {
        //player->SetSpeed(player->GetSpeed() - acc);
        player->Deccelerate();
    }
    if(key[3])   //кнопка D - назад
    {
        //player->Turn(-delta_angle);
        player->RotateR();
    }
    if(key[4])   //Пробел - выстрел
    {
        player->Shoot();
    }
    if (key[5]) //ОБЪЕЗД ПРЕПЯТСТВИЯ
    {
        //player->RideTo(box);
        //player->FollowPath();
        toBuild = true;
    }

    for (stack->Reset(); stack->current != NULL; stack->Next()) //Обновление всех сущностей в стеке
    {
        stack->current->entity->EntityUpdate();
        //Проверка столкновений
        EntityStackItem* saved = stack->current;
        stack->Next();
        for (; stack->current != NULL; stack->Next())
        {
            if (saved->entity->collision_mask->CheckCollision(stack->current->entity->collision_mask))
            {
                saved->entity->collision_mask->collisions++;
                //qDebug()<<saved->entity->collision_mask->collisions;
                stack->current->entity->collision_mask->collisions++;
            }
        }
        stack->current = saved;
        //Отрисовка
        stack->current->entity->Show();
    }
    if (SHOW_COLLIDERS) //Отрисовка столкновений
        for (stack->Reset(); stack->current != NULL; stack->Next())
            stack->current->entity->collision_mask->ShowCollider();
    //player->SetSpeed(0);    //Остановка танка

    double base_width = 80;
    double base_length = 100;
    double threshold = sqrt(base_width*base_width + base_length*base_length)/2 + 10;
    double side = 2 * box->a * 0.8; //Диагональ коробки

    if (toBuild)
    {

        //Обновление пути
        path_graph->clear();
        obstacle* obst= new obstacle [visible->size+2];   //Две точки - старт и финиш, и остальные обьекты
        obst[0].shape = POINT;
        obst[0].point = new Point(player->GetX(), player->GetY());
        obst[1].shape = POINT;
        obst[1].point = new Point(target_x, target_y);
        int i = 2;
        for (visible->Reset(); visible->current!=NULL; visible->Next())
        {
            obst[i].shape = CIRCLE;
            obst[i].point = new Point(visible->current->entity->GetX(),
                                      visible->current->entity->GetY());
            obst[i].rA = threshold+side;
            i++;
        }


        path_graph = build_graph(obst, visible->size+2);
        path_graph->AStar();
        player->graph_to_path(path_graph);
        player->FollowPath();

    }

    //Отрисовка интерфейса
    if (UI_ACTIVE)
        uiUpdate();

    if (SHOW_PATH)
    {
        path_graph->Show();
        player->ShowPath();
        QPainter pntr(picture);
        pntr.setPen(QColor(255,0,0));
        /*
        for (visible->Reset(); visible->current!=NULL; visible->Next())
        {
            pntr.drawEllipse(visible->current->entity->GetX() - (threshold+side),
                             visible->current->entity->GetY() - (threshold+side),
                             2 * (threshold+side), 2*(threshold+side));
        }*/
        pntr.setPen(QColor(0, 0, 255));
        pntr.drawRect(target_x - 5, target_y - 5, 10, 10);  //Маркер
        /*
        //===ТЕСТОВЫЙ ПОКАЗ КОРОБКИ===
        pntr.setPen(QColor(255, 0, 0));
        double base_width = 80;
        double base_length = 100;
        double threshold = sqrt(base_width*base_width + base_length*base_length)/2 + 10;
        double side = 2 * box->a * 0.8; //Диагональ коробки
        pntr.drawEllipse(box->GetX()-side-threshold, box->GetY()-side-threshold, 2 * (side + threshold), 2 * (side + threshold));*/
    }
    update();
};

void game::player_set_path()
{
    player->BuildPath(target_x, target_y, box);
}


//Получение значений
int game::GetW()
{
    return width;
}

int game::GetH()
{
    return height;
}

void game::uiUpdate()   //Обновление меню
{
    SHOW_COLLIDERS = Menu->ShowCollisions->isChecked();
    SHOW_PATH = Menu->ShowPaths->isChecked();
    PAUSE = Menu->Pause->isChecked();

    //Список сущностей
    //Menu->tableWidget->clear();
    Menu->tableWidget->setRowCount(stack->size);
    Menu->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(QString::fromLocal8Bit("Объекты")));
    QItemSelectionModel *select = Menu->tableWidget->selectionModel();
    int a = -1;
    if (select->selectedIndexes().size() > 0)
        a = select->selectedIndexes().first().row();
    else
        Menu->Info->setText("");
    int i = 0;
    for (stack->Reset(); stack->current != NULL; stack->Next())
    {
        Menu->tableWidget->setItem(i, 0, new QTableWidgetItem(stack->current->entity->GetName()));
        if (i == a)
        {
            Menu->Info->setText(stack->current->entity->GetInfo());
            QPainter pntr(picture);
            pntr.setPen(QColor(180,180,0));
            pntr.drawEllipse(stack->current->entity->GetX() - 20, stack->current->entity->GetY() - 20, 40, 40);
            pntr.drawEllipse(stack->current->entity->GetX() - 22, stack->current->entity->GetY() - 22, 44, 44);
            pntr.drawEllipse(stack->current->entity->GetX() - 24, stack->current->entity->GetY() - 24, 48, 48);
            this->setFocus();
        }
        i++;
    }
}






