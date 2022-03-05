#include "game.h"
#include <QTime>
extern QPixmap* picture;
extern EntityStack* stack;


//=== Game class realization ===
game::game(int w, int h, QWidget *parent)   //Window creation and initialization
    : QMainWindow(parent)
{
    //Debug log for path
    path_log = new QFile("path_log.txt");
    path_log->open(QFile::Append | QFile::Text);

    target_x = 0;
    target_y = 0;
    SHOW_COLLIDERS = true;
    UI_ACTIVE = false;
    Menu = new Ui_DebugMenu(this);
    Menu->setupUi();
    connect(Menu->Search, SIGNAL(clicked()), this, SLOT(player_set_path()));
    Menu->hide();
    PAUSE = false;
    width = w;
    height = h;
    resize(width, height);
    for(int i=0;i<7;i++)    //Input buffer
        key[i]=false;
    //Tank creation
    player = new Tank(500, 500);
    box = new Box(1000, 400);
    Box* box1 = new Box(1200, 600);
    Box* box2 = new Box(500, 300);
    Box* box3 = new Box(800, 550);
    WallChain* wc = new WallChain(100, 100);
    //Entity stack creation
    stack = new EntityStack();
    stack->Add((Entity*) player);
//    stack->Add((Entity*) new Wall(1200, 500));
//    stack->Add((Entity*) new EnemyTank(200, 200));   //Enemy tank

    stack->Add((Entity*) box);      //Obstacles
    stack->Add((Entity*) box1);
    stack->Add((Entity*) box2);
    stack->Add((Entity*) box3);
    stack->Add((Entity*) wc);
    //Visible obstacles
    visible = new EntityStack();
    visible->Add((Entity*) box);
    visible->Add((Entity*) box1);
    visible->Add((Entity*) box2);
    visible->Add((Entity*) box3);
    //Updating this every 15ms
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(game_update()));
    timer->start(15);
//    player->BuildPath(0, 0);
    path_graph = new graph();

    uiUpdate();
}

game::~game()
{
    delete path_graph;
    delete visible;
    delete stack;
    delete timer;
}

bool game::event(QEvent* ev)   //Event handler
{
    if (ev->type() == QEvent::KeyPress)    //Keyboard events
    {
        QKeyEvent *ke = (QKeyEvent*)ev;
        int keycode = ke->key();
        switch(keycode)
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
            key[4]=true;
            break;
        case Qt::Key_Control:
            key[5]=true;
            break;
        case Qt::Key_Shift:
            key[6]=true;
            break;
        case Qt::Key_Escape:
            UI_ACTIVE = !UI_ACTIVE;
            if (UI_ACTIVE)
                Menu->show();
            else
            {
                Menu->hide();
                this->setFocus();
            }
            break;
        case Qt::Key_F1:
            UI_MODE = NONE;
            break;
        case Qt::Key_F2:
            UI_MODE = COLLIDERS;
            break;
        case Qt::Key_F3:
            UI_MODE = GRAPH;
            break;
        case Qt::Key_F4:
            UI_MODE = PATH;
            break;
        }
        return true;
    }
    else if (ev->type() == QEvent::KeyRelease)
    {
        QKeyEvent *ke = (QKeyEvent*)ev;
        int keycode = ke->key();
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
            key[4]=false;
            break;
        case Qt::Key_Control:
            key[5]=false;
            break;
        case Qt::Key_Shift:
            key[6]=false;
            break;
        }
        return true;
    }
    return QWidget::event(ev);  //Skip every other event
}

void game::mousePressEvent(QMouseEvent *event)    //Mouse event handler
{
    if (event->button() == Qt::RightButton) {   //Rotating tank head
        double mouse_angle = -direction_to_point(player->GetX(), player->GetY(), event->pos().x(), event->pos().y());
        player->SetCannonAngle(Angle(mouse_angle) - player->GetAngle());
    }
    else if (event->button() == Qt::LeftButton)
    {
        target_x = event->pos().x();
        target_y = event->pos().y();
    }
}

void game::paintEvent(QPaintEvent *) //Drawing buffer content
{
    QPainter pntr(this);
    pntr.drawPixmap(0,0,width,height,*picture,0,0,width,height);

    //Drawing UI
    pntr.setPen(QColor(0,0,0));
    pntr.drawText(10, 10, "Number of objects:\t " + QString::number(stack->size));
    pntr.drawText(10, 20, "Press F1 to open debug menu");
    pntr.drawText(10, 30, PAUSE?"PAUSED":"");
    if (player->reload_timeout == 0)
        pntr.drawText(10, 30, "Cannon ready");

}

void game::game_update()  //Function, called every frame
{
    bool toBuild = false;
    bool follow = false;
    picture->fill();    //Clearing buffer

    //Player control
    if(key[0])   //W - Acceleration
    {
        player->Accelerate();
    }
    if(key[1])   //A - Left turn
    {
        player->RotateL();
    }
    if(key[2])   //S - Brakes/Rear gear
    {
        player->Deccelerate();
    }
    if(key[3])   //D - Right turn
    {
        player->RotateR();
    }
    if(key[4])   //[SPACE] - Shoot
    {
        player->Shoot();
    }
    if (key[5]) //[CTRL] - building path
    {
        //player->RideTo(box);
        //player->FollowPath();
        toBuild = true;
    }
    if (key[6]) //[SHIFT] - following path
    {
        follow = true;
    }

    SHOW_COLLIDERS = UI_MODE == COLLIDERS;
    bool SHOW_PATH = UI_MODE == PATH;
    for (stack->Reset(); stack->current != NULL; stack->Next()) //Updating of every entity
    {
        stack->current->entity->EntityUpdate();
        //Collision check
        if (SHOW_COLLIDERS)
        {
            EntityStackItem* saved = stack->current;
            stack->Next();
            for (; stack->current != NULL; stack->Next())
            {
                if (saved->entity->collision_mask->CheckCollision(stack->current->entity->collision_mask))
                {
                    saved->entity->collision_mask->collisions++;
                    stack->current->entity->collision_mask->collisions++;
                }
            }
            stack->current = saved;
        }
        //Drawing every entity
        stack->current->entity->Show();
    }
    if (SHOW_COLLIDERS) //Drawing colliders
        for (stack->Reset(); stack->current != NULL; stack->Next())
            stack->current->entity->collision_mask->ShowCollider();

    double base_width = 80;
    double base_length = 100;
    double threshold = sqrt(base_width*base_width + base_length*base_length)/2 + 10;
    double side = 2 * box->a * 0.8;
    QTime start = QTime::currentTime();
    QTime graph_end;
    QTime path_end;
    if (toBuild)
    {
        //Path updating
        path_graph->clear();
        obstacle* obst= new obstacle [visible->size+2];   //Two is for start and end points.
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
            obst[i].r = threshold+side;
            i++;
        }

        path_graph = build_graph(obst, visible->size+2);
        graph_end = QTime::currentTime();
        path_graph->AStar();
        player->graph_to_path(path_graph);
        path_end = QTime::currentTime();

        for(uint i = 0; i < visible->size+2; i++)
        {
            delete obst[i].point;
        }
        delete[] obst;
        int delta1 = start.msecsTo(graph_end);
        int delta2 = graph_end.msecsTo(path_end);
        qDebug() << "Graph built in:" << delta1;
        qDebug() << "Path built in:" << delta2;
    }

    if (follow)
    {
        if (path_graph != nullptr)
        {
            player->FollowPath();
        }
    }

    //Showing menu
    if (UI_ACTIVE)
        uiUpdate();

    if (SHOW_PATH)
    {
        path_graph->Show();
        player->ShowPath();
        QPainter pntr(picture);
        pntr.setPen(QColor(255,0,0));
        pntr.setPen(QColor(0, 0, 255));
        pntr.drawRect(target_x - 5, target_y - 5, 10, 10);  //End point
    }
    if (UI_MODE == GRAPH)
    {
        QPoint mouse_pos = mapFromGlobal( QCursor::pos());
        path_graph->Show(mouse_pos.x(), mouse_pos.y());
        QPainter pntr(picture);
        pntr.setPen(QColor(255,0,0));
        pntr.setPen(QColor(0, 0, 255));
        pntr.drawRect(target_x - 5, target_y - 5, 10, 10);  //End point
    }
    update();
}

//Getters
int game::GetW()
{
    return width;
}

int game::GetH()
{
    return height;
}

void game::uiUpdate()   //UI update
{
    PAUSE = Menu->Pause->isChecked();

    //List of entities
    Menu->tableWidget->setRowCount(stack->size);
    Menu->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Объекты"));
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
        if (i == a) //Showing selected object
        {
            Menu->Info->setText(stack->current->entity->GetInfo());
            stack->current->entity->ShowOutline();
            this->setFocus();
        }
        i++;
    }
}

Path::Path(int _num, double tx, double ty)
{
    startx = 0;
    starty = 0;
    num = _num;
    pts = new struct pathpoint[_num];
    i = 0;
    final_x = tx;
    final_y = ty;
}

Path::~Path()
{
    delete[] pts;
}




