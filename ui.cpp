#include "game.h"

extern QPixmap* picture;
extern EntityStack* stack;

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

void game::uiUpdate()   //Update of menu
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
