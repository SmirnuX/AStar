#include "game.h"
#include <ratio>
#include <QTime>
#include <QFileDialog>

extern QPixmap* picture;
extern EntityStack* stack;


//=== Game class realization ===
game::game(int w, int h, QWidget *parent)   //Window creation and initialization
    : QMainWindow(parent)
{
    target_x = 0;
    target_y = 0;

    total_collisions = 0;

    //Graphs intitialize
    for (int i = 0; i < SPEED_GUI_SIZE; i++)
    {
        speeds[i] = 0;
        max_speeds[i] = 0;
    }

    stack = new EntityStack();  //Entity stack creation

    QFile save(":/test.json");
    qDebug() << save.open(QFile::ReadOnly);

    player = new Car(100, 100);
    stack->Add((Entity*) player);

    loadSave(QJsonDocument().fromJson(save.readAll()));\

    player->MoveTo(player_start_x, player_start_y);

    UI_ACTIVE = false;
    Menu = new Ui_DebugMenu(this);
    Menu->setupUi();
    connect(Menu->LoadScene, SIGNAL(clicked()), this, SLOT(load_scene()));
    Menu->hide();
    PAUSE = false;
    width = w;
    height = h;
    resize(width, height);
    for(int i=0;i<7;i++)    //Input buffer
        key[i]=false;

    //Updating this every 15ms
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(game_update()));
    timer->start(15);

    path_graph = nullptr;
    uiUpdate();
}

game::~game()
{
    delete path_graph;
    delete stack;
    delete timer;
}

bool game::event(QEvent* ev)   //Event handler
{
    if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::KeyRelease)    //Keyboard events
    {
        QKeyEvent *ke = (QKeyEvent*)ev;
        int keycode = ke->key();
        switch(keycode)
        {
        case Qt::Key_W:
        case Qt::Key_Up:
            key[0]=(ev->type() == QEvent::KeyPress);
            break;
        case Qt::Key_A:
        case Qt::Key_Left:
            key[1]=(ev->type() == QEvent::KeyPress);
            break;
        case Qt::Key_S:
        case Qt::Key_Down:
            key[2]=(ev->type() == QEvent::KeyPress);
            break;
        case Qt::Key_D:
        case Qt::Key_Right:
            key[3]=(ev->type() == QEvent::KeyPress);
            break;
        case Qt::Key_Space:
            key[4]=(ev->type() == QEvent::KeyPress);
            break;
        case Qt::Key_Control:
            key[5]=(ev->type() == QEvent::KeyPress);
            break;
        case Qt::Key_Shift:
            key[6]=(ev->type() == QEvent::KeyPress);
            break;
        }
        if (ev->type() == QEvent::KeyPress)
        {
            QKeyEvent *ke = (QKeyEvent*)ev;
            int keycode = ke->key();
            switch(keycode)
            {
            case Qt::Key_Escape:    //Show/hide menu
                UI_ACTIVE = !UI_ACTIVE;
                if (UI_ACTIVE)
                    Menu->show();
                else
                {
                    Menu->hide();
                    this->setFocus();
                }
                break;
            case Qt::Key_F1:    //Switch UI modes
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
                UI_MODE = RADAR;
                break;
            case Qt::Key_F6:
                UI_MODE = OBJECTS;
                break;
            }
        }
        return true;
    }
    return QWidget::event(ev);  //Skip any other event
}

void game::mousePressEvent(QMouseEvent *event)    //Mouse event handler
{
    if (event->button() == Qt::LeftButton && !UI_ACTIVE && UI_MODE != OBJECTS)  //Setting target for car
    {
        target_x = event->pos().x();
        target_y = event->pos().y();
    }
    else if (event->button() == Qt::LeftButton && !UI_ACTIVE && UI_MODE == OBJECTS)
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
    player->ResetFlags();

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
    if(key[4])   //[SPACE] - nothing
    {

    }
    if (key[5]) //[CTRL] - building path
    {
        toBuild = true;
    }
    if (key[6]) //[SHIFT] - following path
    {
        follow = true;
    }

    for (stack->Reset(); stack->current != NULL; stack->Next()) //Updating of every entity
    {
        stack->current->entity->EntityUpdate();
    }

    if (SHOW_COLLIDERS) //Collision check
    {
        for (stack->Reset(); stack->current != NULL; stack->Next())
        {
            EntityStackItem* saved = stack->current;
            stack->Next();
            for (; stack->current != NULL; stack->Next())
            {
                if (saved->entity->collision_mask->CheckCollision(stack->current->entity->collision_mask))
                {
                    if (saved->entity == player || stack->current->entity == player)
                        total_collisions++;
                    saved->entity->collision_mask->collisions++;
                    stack->current->entity->collision_mask->collisions++;
                }
            }
            stack->current = saved;
        }
    }

    if (toBuild)
    {
        //Old path
        if (path_graph != nullptr)
        {
            delete path_graph;
        }

        auto chr_start = std::chrono::high_resolution_clock::now();
        //Clearing ol obstacles
        if (obst != nullptr)
        {
            for (uint i = 0; i < obst_num; i++)
                DeleteObstacle(obst+i);
            delete[] obst;
            obst = nullptr;
            obst_num = 0;
        }
        obst = new obstacle [player->map.Size()+2];   //Two is for start and end points.
        obst_num = player->map.Size()+2;
        obst[0].shape = POINT;
        obst[0].point = new Point(player->GetX(), player->GetY());
        obst[1].shape = POINT;
        obst[1].point = new Point(target_x, target_y);
        uint i = 2;

        for (; i < obst_num;)
        {
            obst[i] = player->map.GetObstacle(i-2, 50);
            i++;
        }
        path_graph = build_graph(obst, obst_num);
        auto chr_graph_end = std::chrono::high_resolution_clock::now();
        path_graph->AStar();
        player->graph_to_path(path_graph);
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

    //Entities drawing
    for (stack->Reset(); stack->current != NULL; stack->Next())
    {
        //Drawing every entity
        stack->current->entity->Show();
    }

    showUI();
    update();
}


void game::showUI() //Show UI
{
    //Graph updating
    for (int i = 0; i < SPEED_GUI_SIZE-1; i++)
    {
        speeds[i] = speeds[i+1];
        max_speeds[i] = max_speeds[i+1];
    }
    speeds[SPEED_GUI_SIZE-1] = player->GetSpeed();
    max_speeds[SPEED_GUI_SIZE-1] = player->GetPathMaxSpeed();

    //Showing menu
    if (UI_ACTIVE)
        uiUpdate();

    if (UI_MODE == NONE)
        return;

    //Show UI from different QPainter
    if (UI_MODE == COLLIDERS)
    {
        for (stack->Reset(); stack->current != NULL; stack->Next()) //Show all colliders
            stack->current->entity->collision_mask->ShowCollider();
    }
    else if (UI_MODE == GRAPH)   //Drawing graph, and some related info
    {
        if (path_graph != nullptr)
        {
            QPoint mouse_pos = mapFromGlobal( QCursor::pos());
            path_graph->Show(mouse_pos.x(), mouse_pos.y());
        }
    }
    else if (UI_MODE == PATH)
    {
        if (path_graph != nullptr)
        {
            path_graph->Show();
        }
        player->ShowPath();
    }
    else if (UI_MODE == RADAR)
    {
        picture->fill();
        player->Show();
        player->map.Show();
    }

    //Showing chosen UI mode
    QPainter pntr(picture);
    pntr.setPen(QColor(0,0,0));
    pntr.drawText(10, 10, QString("F1 - HIDE, ") +
                    (UI_MODE == COLLIDERS    ? "[F2 - COLLISION], "  : "F2 - COLLISION, ") +
                    (UI_MODE == GRAPH        ? "[F3 - GRAPH], "      : "F3 - GRAPH, ") +
                    (UI_MODE == PATH         ? "[F4 - PATH], "       : "F4 - PATH, ") +
                    (UI_MODE == RADAR        ? "[F5 - VISION], "     : "F5 - VISION, ") +
                    (UI_MODE == OBJECTS      ? "[F6 - EDITOR], "     : "F6 - EDITOR, "));

    //Show UI from current QPainter
    if (UI_MODE == COLLIDERS)
    {
        pntr.drawText(10, 20, "Number of objects:\t " + QString::number(stack->size));
        pntr.drawText(10, 30, "Number of car collisions:\t " + QString::number(total_collisions));
    }
    else if (UI_MODE == GRAPH)
    {
        if (path_graph != nullptr)
        {
            pntr.drawText(10, 20, "Number of vertices\t " + QString::number(path_graph->vertices.size()));
            pntr.drawText(10, 30, "Number of edges\t " + QString::number(path_graph->edges.size()));
            pntr.drawText(10, 40, "Last graph built in:\t" + QString::number(build_time.count()) + "ms");
            pntr.drawText(10, 50, "Last path found in:\t" + QString::number(pathfind_time.count()) + "ms");
        }

        pntr.setPen(QColor(0, 0, 255));
        pntr.drawRect(target_x - 5, target_y - 5, 10, 10);  //End point
    }
    else if (UI_MODE == PATH)
    {
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
        pntr.setPen(QColor(0, 0, 255));
        pntr.drawRect(target_x - 5, target_y - 5, 10, 10);  //End point

        //Drawing speed graph
        pntr.setPen(QColor(0, 0, 200));
        int maxy = 40;
        int miny = 80;
        int stepx = 2;
        int maxspeed = 8;
        for (int i = 0; i < SPEED_GUI_SIZE-1; i++)
        {
            pntr.drawLine(i*stepx, miny + speeds[i]/maxspeed*(maxy-miny),
                          (i+1)*stepx, miny + speeds[i+1]/maxspeed*(maxy-miny));
        }
        pntr.setPen(QColor(200,0,0));
        for (int i = 0; i < SPEED_GUI_SIZE-1; i++)
        {
            pntr.drawLine(i*stepx, miny + max_speeds[i]/maxspeed*(maxy-miny),
                          (i+1)*stepx, miny + max_speeds[i+1]/maxspeed*(maxy-miny));
        }

        //Drawing angle and target angle
        int cent_x = 800;
        int cent_y = 40;
        int cent_r = 38;
        pntr.setPen(QColor(60,60,60));
        pntr.drawEllipse(cent_x - cent_r, cent_y - cent_r, 2 * cent_r, 2 * cent_r);
        pntr.setPen(QColor(0, 180, 0));
        pntr.drawLine(cent_x, cent_y, cent_x + cent_r * cos(player->GetAngle().GetR()), cent_y - cent_r * sin(player->GetAngle().GetR()));
        pntr.setPen(QColor(180,0,0));
        pntr.drawLine(cent_x, cent_y, cent_x + cent_r * cos(player->target_ui_angle.GetR()), cent_y + cent_r * sin(player->target_ui_angle.GetR()));
    }
    else if (UI_MODE == RADAR)
    {
        pntr.drawText(10, 20, "Number of real objects:\t " + QString::number(stack->size));
        pntr.drawText(10, 30, "Number of found objects:\t " + QString::number(player->map.obstacles.size()));
    }

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



void game::load_scene()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "",
                                                    tr("JSON (*.json)"));
    qDebug() << fileName;
    QFile f(fileName);
    f.open(QFile::ReadOnly);
    QByteArray json_r = f.readAll();

    QJsonDocument json = QJsonDocument::fromJson(json_r);
    loadSave(json);

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
            QJsonArray pts = (objects[i].toObject())["pts"].toArray();
            assert(num == pts.size());
            stack->Add((Entity*) new Poly(pts));
        }
    }
    for (stack->Reset(); stack->current != NULL; stack->Next())
    {
        if (stack->current == NULL || stack->current->entity == NULL)
            break;
        qDebug() << stack->current->entity <<"|" <<stack->current->entity->GetX();
    }
    return true;
}



void game::uiUpdate()   //Update of UI
{
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




