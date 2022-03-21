#include "game.h"
#include <ratio>
#include <QTime>
extern QPixmap* picture;
extern EntityStack* stack;


//=== Game class realization ===
game::game(int w, int h, QWidget *parent)   //Window creation and initialization
    : QMainWindow(parent)
{
    target_x = 0;
    target_y = 0;

    stack = new EntityStack();  //Entity stack creation

    QFile save(":/test.json");
    qDebug() << save.open(QFile::ReadOnly);
    loadSave(QJsonDocument().fromJson(save.readAll()));

    player = new Tank(player_start_x, player_start_y);
    stack->Add((Entity*) player);



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

    box = new Box(1000, 400);
    Box* box1 = new Box(1200, 600);
    HexBox* hex1 = new HexBox(500, 300);
    Barell* circ1 = new Barell(800, 550, 50);
    WallChain* wc = new WallChain(900, 100);
    Wall* ln = new Wall(300, 200);

//    stack->Add((Entity*) new Wall(1200, 500));
//    stack->Add((Entity*) new EnemyTank(200, 200));   //Enemy tank

    stack->Add((Entity*) box);      //Obstacles
    stack->Add((Entity*) box1);
    stack->Add((Entity*) hex1);
    stack->Add((Entity*) circ1);
    stack->Add((Entity*) wc);
    stack->Add((Entity*) ln);
    //Visible obstacles
    visible = new EntityStack();
    visible->Add((Entity*) box);
    visible->Add((Entity*) box1);
    visible->Add((Entity*) hex1);
    visible->Add((Entity*) circ1);
    visible->Add((Entity*) wc);
    visible->Add((Entity*) ln);
    //Updating this every 15ms
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(game_update()));
    timer->start(15);
//    player->BuildPath(0, 0);
    path_graph = nullptr;

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
        case Qt::Key_F5:
            UI_MODE = OBJECTS;
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
    else if (event->button() == Qt::LeftButton && (UI_MODE == GRAPH || UI_MODE == PATH))
    {
        target_x = event->pos().x();
        target_y = event->pos().y();
    }
    else if (event->button() == Qt::LeftButton && UI_MODE == OBJECTS)
    {

    }
}

void game::paintEvent(QPaintEvent *) //Drawing buffer content
{
    QPainter pntr(this);
    pntr.drawPixmap(0,0,width,height,*picture,0,0,width,height);
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


    QTime start = QTime::currentTime();
    QTime graph_end;
    QTime path_end;
    if (toBuild)
    {
        //Path updating
        if (path_graph != nullptr)
        {
            delete path_graph;
        }

        auto chr_start = std::chrono::high_resolution_clock::now();
        //Clearing ol obstacles
        if (obst != nullptr)
        {
            for (int i = 0; i < obst_num; i++)
                DeleteObstacle(obst+i);
            delete[] obst;
            obst = nullptr;
            obst_num = 0;
        }
        obst = new obstacle [visible->size+2];   //Two is for start and end points.
        obst_num = visible->size+2;
        obst[0].shape = POINT;
        obst[0].point = new Point(player->GetX(), player->GetY());
        obst[1].shape = POINT;
        obst[1].point = new Point(target_x, target_y);
        int i = 2;
        for (visible->Reset(); visible->current!=NULL; visible->Next())
        {
            obst[i] = visible->current->entity->collision_mask->GetOutline(50);
            i++;
        }
        assert(i == visible->size+2);

        path_graph = build_graph(obst, obst_num);
        graph_end = QTime::currentTime();
        auto chr_graph_end = std::chrono::high_resolution_clock::now();
        path_graph->AStar();
        player->graph_to_path(path_graph);
        path_end = QTime::currentTime();
        auto chr_path_end = std::chrono::high_resolution_clock::now();

        build_time = chr_graph_end - chr_start;
        pathfind_time = chr_path_end - chr_graph_end;
        toBuild = false;
    }


    if (follow) //Path following
    {
        if (path_graph != nullptr)
        {
            player->FollowPath();
        }
    }

    //Showing menu
    if (UI_ACTIVE)
        uiUpdate();

    if (SHOW_PATH)  //Showing path, temp speeds and related stuff
    {
        if (path_graph != nullptr)
            path_graph->Show();
        player->ShowPath();
        QPainter pntr(picture);
        pntr.setPen(QColor(255,0,0));
        pntr.setPen(QColor(0, 0, 255));
        pntr.drawRect(target_x - 5, target_y - 5, 10, 10);  //End point
    }
    if (UI_MODE == COLLIDERS)
    {
        QPainter pntr(picture);
        pntr.setPen(QColor(0,0,0));
        pntr.drawText(10, 10, "F1 - HIDE, [F2 - COLLISION], F3 - GRAPH, F4 - PATH, F5 - EDITOR");
        pntr.drawText(10, 20, "Number of objects:\t " + QString::number(stack->size));
        if (player->reload_timeout == 0)
            pntr.drawText(10, 30, "Cannon ready");
    }
    else if (UI_MODE == GRAPH)   //Drawing graph, and some related info
    {
        int i = 2;
        for (visible->Reset(); visible->current!=NULL; visible->Next())
        {
            if (obst == nullptr)
                break;
            ShowObstacle(obst+i);
            i++;
        }

        if (path_graph != nullptr)
        {
            QPoint mouse_pos = mapFromGlobal( QCursor::pos());
            path_graph->Show(mouse_pos.x(), mouse_pos.y());
        }
        QPainter pntr(picture);
        pntr.setPen(QColor(0, 0, 255));
        pntr.drawRect(target_x - 5, target_y - 5, 10, 10);  //End point
        pntr.setPen(QColor(0, 0, 0));
        pntr.drawText(10, 10, "F1 - HIDE, F2 - COLLISION, [F3 - GRAPH], F4 - PATH, F5 - EDITOR");   //Head of UI
        if (path_graph != nullptr)
        {
            pntr.drawText(10, 20, "Number of vertices\t " + QString::number(path_graph->vertices.size()));
            pntr.drawText(10, 30, "Number of edges\t " + QString::number(path_graph->edges.size()));
            pntr.drawText(10, 40, "Last graph built in:\t" + QString::number(build_time.count()) + "ms");
            pntr.drawText(10, 50, "Last path found in:\t" + QString::number(pathfind_time.count()) + "ms");
        }
    }
    else if (UI_MODE == PATH)   //Drawing graph, and some related info
    {
        QPainter pntr(picture);
        pntr.setPen(QColor(0, 0, 0));
        pntr.drawText(10, 10, "F1 - HIDE, F2 - COLLISION, F3 - GRAPH, [F4 - PATH], F5 - EDITOR");   //Head of UI
        if (path_graph != nullptr)
        {
            if (path_graph->found_way)
            {
                pntr.drawText(10, 20, "Path length:\t " + QString::number(path_graph->way_length));
                pntr.drawText(10, 30, "Number of waypoints:\t " + QString::number(player->path->num));
            }
            else
            {
                pntr.drawText(10, 20, "Path not found");
            }
        }
    }
    else if (UI_MODE == GRAPH)   //Drawing graph, and some related info
    {
        int i = 2;
        for (visible->Reset(); visible->current!=NULL; visible->Next())
        {
            if (obst == nullptr)
                break;
            ShowObstacle(obst+i);
            i++;
        }

        if (path_graph != nullptr)
        {
            QPoint mouse_pos = mapFromGlobal( QCursor::pos());
            path_graph->Show(mouse_pos.x(), mouse_pos.y());
        }
        QPainter pntr(picture);
        pntr.setPen(QColor(0, 0, 255));
        pntr.setPen(QColor(0, 0, 0));
        pntr.drawRect(target_x - 5, target_y - 5, 10, 10);  //End point
        pntr.drawText(10, 10, "F1 - HIDE, F2 - COLLISION, [F3 - GRAPH], F4 - PATH, F5 - EDITOR");   //Head of UI
        if (path_graph != nullptr)
        {
            pntr.drawText(10, 20, "Number of vertices\t " + QString::number(path_graph->vertices.size()));
            pntr.drawText(10, 30, "Number of edges\t " + QString::number(path_graph->edges.size()));
            pntr.drawText(10, 40, "Last graph built in:\t" + QString::number(build_time.count()) + "ms");
            pntr.drawText(10, 50, "Last path found in:\t" + QString::number(pathfind_time.count()) + "ms");
        }
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

bool game::loadSave(const QJsonDocument &json)
{
    QJsonObject player = json["player"].toObject();
    player_start_x = player["x"].toDouble();
    player_start_y = player["y"].toDouble();
    QJsonArray objects = json["objects"].toArray();
    for (int i = 0; i < objects.size(); i++)
    {
        if ((objects[i].toObject())["type"].toString() == "circle")
        {
            stack->Add((Entity*) new Barell((objects[i].toObject())["x"].toDouble(),
                                            (objects[i].toObject())["y"].toDouble(),
                                            (objects[i].toObject())["r"].toDouble()));
        }
        else if ((objects[i].toObject())["type"].toString() == "poly")
        {
            int num = qFloor((objects[i].toObject())["num"].toDouble());

        }
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




