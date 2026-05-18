#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <Qpainter>
#include <QTimer>
#include <qpushbutton.h>
#include <vector>
#include "gamelogic.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;
    void setOder(int n);
    void updateTimer();
    void startLevel(int level);

private slots:
    void on_pushButton_clicked();
    void onCardClicked(int row,int col);
    void on_btnTraditional_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_10_clicked();


    void on_pushLevel_1_clicked();

    void on_pushLevel_2_clicked();

    void on_pushLevel_3_clicked();

    void on_pushLevel_4_clicked();

    void on_pushLevel_5_clicked();

    void on_pushLevel_6_clicked();

    void on_pushLevel_7_clicked();

    void on_pushLevel_8_clicked();

    void on_pushLevel_9_clicked();

    void on_pushLevel_10_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

protected:
    void paintEvent(QPaintEvent* event)override;

private:
    Ui::Widget *ui;
    int order=4;
    std::vector<std::vector<int>>map;
    std::vector<std::vector<QPushButton*>>btnMap;
    int firstRow=-1;
    int firstCol=-1;
    bool isFirstClick=true;
    GameLogic game;
    std::vector<PathNode> pathToDraw;
    bool isDrawingPath=false;
    bool isLocked=false;
    int eliminateCount=0;
    QTimer* gameTimer=nullptr;
    int timeLeft=60;
    int TotalTime=60;
    bool isTraditional=false;
    int currentOrder=4;    //设置
    int totalTimeLimit=60;
    int remainTIme=60;
    bool isLevelModel=false;
    int unlockedLevel=1;
    int currentPlayingLevel=4;
    void updateLevelButtons();
};
#endif // WIDGET_H
