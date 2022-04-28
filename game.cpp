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
    width = w;
    height = h;

    initWindow();
    initLevel();

}

game::~game()
{
    clearLevel();
    delete timer;
    delete Menu;
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

    if (UI_MODE == COLLIDERS) //Collision check
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




//Getters
int game::GetW()
{
    return width;
}

int game::GetH()
{
    return height;
}



void game::loadFile()   //Open file browser and then load level from JSON
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "",
                                                    tr("JSON (*.json)"));
    qDebug() << fileName;
    QFile f(fileName);
    f.open(QFile::ReadOnly);
    QByteArray json_r = f.readAll();

    QJsonDocument json = QJsonDocument::fromJson(json_r);
    initLevel();
    loadLevel(json);
}

bool game::loadLevel(const QJsonDocument &json)
{
    QJsonObject _player = json["player"].toObject();
    player_start_x = _player["x"].toDouble();
    player_start_y = _player["y"].toDouble();

    if (player != nullptr)
        player->MoveTo(player_start_x, player_start_y);

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

void game::saveFile()   //Open file browser and then save level to JSON
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Open File"),
                                                    "",
                                                    tr("JSON (*.json)"));
    qDebug() << fileName;
    QFile f(fileName);
    f.open(QFile::WriteOnly);

    QJsonDocument save;
    saveLevel(save);

    f.write(save.toJson());
}

void game::saveLevel(QJsonDocument& json)
{
    QJsonObject res;
    QJsonObject player_info;
    player_info["x"] = player->GetX();
    player_info["y"] = player->GetY();
    res["player"] = player_info;

    QJsonArray objects;

    for (stack->Reset(); stack->current != NULL; stack->Next())
    {
        if (stack->current->entity == player)
            break;
        if (stack->current->entity->collision_mask != nullptr)
        {
            QJsonObject obj = stack->current->entity->collision_mask->toJson();
            if (!obj.isEmpty())
                objects.append(obj);
        }
    }
    res["objects"] = objects;

    json.setObject(res);
}

void game::initWindow() //Global initialization
{
    //Graph intitialization
    for (int i = 0; i < SPEED_GUI_SIZE; i++)
    {
        speeds[i] = 0;
        max_speeds[i] = 0;
    }
    player = nullptr;

    //Setting up UI
    UI_ACTIVE = false;
    Menu = new Ui_DebugMenu(this);
    Menu->setupUi();
    connect(Menu->LoadScene, SIGNAL(clicked()), this, SLOT(loadFile()));
    connect(Menu->SaveScene, SIGNAL(clicked()), this, SLOT(saveFile()));
    Menu->hide();

    //Window settings
    resize(width, height);
    for(int i=0;i<7;i++)    //Input buffer
        key[i]=false;

    //Updating this every 15ms
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(game_update()));
    timer->start(15);

    //Editor mode initialization
    editing = nullptr; //Point, that is moved by mouse in current moment
    chosen = nullptr; //Whole entity
    is_building_poly = false;  //Is new polygon building now?

//    orig_x = 0;
//    orig_y = 0;
}

//void game::editor_step(double mx, double my)
//{
//    if (editing != nullptr) //If there is captured point
//    {
//        editing->MoveTo(mx, my);
//    }
//    else if (chosen != nullptr)
//    {
//        chosen->MoveTo(mx + orig_x, my + orig_y);
//    }
//}

//void game::editor_leftbtn(double mx, double my)
//{
//    if (editing != nullptr) //End of moving object/points
//        editing = nullptr;
//    else if (chosen != nullptr)
//        chosen = nullptr;
//    else
//    {
//        EntityStackItem* saved = stack->current;
//        stack->Next();
//        for (; stack->current != NULL; stack->Next())
//        {
//            if (saved->entity->collision_mask->CheckCollision(stack->current->entity->collision_mask))
//            {
//                if (saved->entity == player || stack->current->entity == player)
//                    total_collisions++;
//                saved->entity->collision_mask->collisions++;
//                stack->current->entity->collision_mask->collisions++;
//            }
//        }
//        stack->current = saved;
//    }
//}

void game::initLevel()  //Initialization of level
{
    //Target point
    target_x = 0;
    target_y = 0;

    total_collisions = 0;   //Number of frames, when car is in collision with obstacles

    stack = new EntityStack();  //Entity stack creation

    player = new Car(100, 100); //Adding car
    stack->Add((Entity*) player);

    path_graph = nullptr;
    uiUpdate();
}

void game::clearLevel() //Clear all obstacles
{
    //Delete all obstacles
    delete path_graph;
    delete stack;
}








