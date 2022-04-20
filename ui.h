#ifndef UI_H
#define UI_H

//Code generated in QT Creator

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>
#include <QFrame>

class Ui_DebugMenu: public QWidget
{
public:
    QFrame* Menu;
    QPushButton *LoadScene;  //Load level
    QPushButton *SaveScene;  //Save level
    QCheckBox *ShowCollisions;
    QLabel *label;
    QTableWidget *tableWidget;
    QPushButton *Control;
    QPushButton *Search;
    QCheckBox *ShowPaths;
    QLabel *Info;
    QLabel *label_2;

    Ui_DebugMenu(QWidget* parent):QWidget(parent)
    {
        setupUi();
        //HideUI();
    }

    void setupUi()//QWidget *widget)
    {
        QWidget* widget = this;
        widget->resize(231, 607);
        Menu = new QFrame(widget);
        Menu->setGeometry(QRect(0, 0, 231, 600));
        Menu->setFrameShape(QFrame::Panel);
        Menu->setStyleSheet("");
        Menu->setStyleSheet("background-color: rgb(230,230,230)");
        //Menu = new QWidget(widget);
        //Menu->setObjectName(QString::fromUtf8("Menu"));
        LoadScene = new QPushButton(Menu);
        LoadScene->setObjectName(QString::fromUtf8("Load Scene"));
        LoadScene->setGeometry(QRect(60, 40, 111, 23));
        SaveScene = new QPushButton(Menu);
        SaveScene->setObjectName(QString::fromUtf8("Save Scene"));
        SaveScene->setGeometry(QRect(60, 70, 111, 23));

//        Pause->setCheckable(true);
        ShowCollisions = new QCheckBox(Menu);
        ShowCollisions->setObjectName(QString::fromUtf8("ShowCollisions"));
        ShowCollisions->setGeometry(QRect(10, 80, 161, 18));
        ShowCollisions->setChecked(true);
        label = new QLabel(Menu);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 140, 81, 16));
        tableWidget = new QTableWidget(Menu);
        if (tableWidget->columnCount() < 1)
            tableWidget->setColumnCount(1);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setGeometry(QRect(10, 160, 211, 231));
        tableWidget->setColumnWidth(0, 180);
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        Control = new QPushButton(Menu);
        Control->setObjectName(QString::fromUtf8("Control"));
        Control->setGeometry(QRect(50, 530, 131, 23));
        Control->setCheckable(true);
        Search = new QPushButton(Menu);
        Search->setObjectName(QString::fromUtf8("Search"));
        Search->setGeometry(QRect(50, 560, 131, 23));
        Search->setCheckable(true);
        ShowPaths = new QCheckBox(Menu);
        ShowPaths->setObjectName(QString::fromUtf8("ShowPaths"));
        ShowPaths->setGeometry(QRect(10, 100, 161, 18));
        ShowPaths->setChecked(true);
        Info = new QLabel(Menu);
        Info->setObjectName(QString::fromUtf8("Info"));
        Info->setGeometry(QRect(20, 400, 191, 111));
        Info->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        Info->setWordWrap(true);
        label_2 = new QLabel(Menu);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 10, 81, 16));

        retranslateUi();

        //QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi()
    {
        LoadScene->setText("Загр. сцену");
        SaveScene->setText("Сохр. сцену");
        ShowCollisions->setText("Показывать столкновения");
        label->setText("Объекты");
        QTableWidgetItem *___qtablewidgetitem = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText("error");
        Control->setText("Взять под контроль");
        Search->setText("Поиск пути");
        ShowPaths->setText("Показывать пути");
        Info->setText("");
        label_2->setText("Меню отладки");
    } // retranslateUi
};


#endif // UI_H
