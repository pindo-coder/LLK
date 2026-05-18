#include "widget.h"
#include "ui_widget.h"
#include <QPushButton>
#include <QGridLayout>
#include <QRandomGenerator64>
#include <vector>
#include <QFile>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QInputDialog>
#include <QDebug>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QSlider>

void Widget::setOder(int n){
    if(ui->page_2->layout()!=nullptr){
        QLayoutItem* item;
        QLayout* oldLayout=ui->page_2->layout();
        while((item=oldLayout->takeAt(0))!=nullptr){
            if(item->widget()){
                delete item->widget();
            }
            delete item;
        }
        delete oldLayout;
    }
    map.clear();
    btnMap.clear();
    pathToDraw.clear();
    if(isTraditional){
        timeLeft=TotalTime;
        ui->lcd_timer->display(timeLeft);
        gameTimer->start(1000);
        ui->lcd_timer->setStyleSheet("color:#FF0000;background-color:rgba(0,0,0,150);border:2px solid #EAA112;");
    }else{
        gameTimer->stop();
    }
    this->order=n;
    this->eliminateCount=0;
    map.resize(order+2,std::vector<int>(order+2,0));
    btnMap.resize(order+2,std::vector<QPushButton*>(order+2,nullptr));
    std::vector<int>cards(order*order);
    for(int i=0;i<order*order;i+=2){
        int pid =((i/2)%10)+1;
        cards[i]=pid;
        if(i+1<order*order){
            cards[i+1]=pid;
        }
    }
    for(int i=0;i<order*order;i++){
        int ran=QRandomGenerator64::global()->bounded(order*order);
        int temp=cards[i];
        cards[i]=cards[ran];
        cards[ran]=temp;
    }
    for(int i=0;i<order*order;i++){
        map[i/order+1][i%order+1]=cards[i];
    }
    game.map=&(this->map);
    QGridLayout *gridLayout = new QGridLayout(ui->page_2);
    for(int row=0;row<order+2;row++){
        for(int col=0;col<order+2;++col){
            QPushButton *btn=new QPushButton();
            QSizePolicy sp=btn->sizePolicy();
            sp.setRetainSizeWhenHidden(true);
            btn->setSizePolicy(sp);
            int pid=map[row][col];
            btn->setFixedSize(50,50);
            if(pid>0){
                QIcon buttonIcon;
                QString imgPath=QString(":/picture/button%1.png").arg(pid);
                btn->setIcon(QIcon(imgPath));
                btn->setIconSize(QSize(45,45));
                btn->setFlat(true);
                btn->setStyleSheet(
                    "QPushButton {"
                    "   background-color: transparent;" // 背景完全透明
                    "   border: none;"                 // 去掉边框
                    "   color: transparent;"           // 让原本的数字文字彻底消失
                    "}"
                    "QPushButton:hover {"
                    "   background-color: rgba(255, 255, 255, 60);" // 鼠标悬停时有个淡淡的白光，方便玩家知道选了哪个
                    "   border-radius: 5px;"
                    "}"
                    "QPushButton:pressed {"
                    "   background-color: rgba(255, 255, 255, 100);" // 按下时更亮一点
                    "}"
                    );
            }
            btnMap[row][col]=btn;
            if(map[row][col]==0)btnMap[row][col]->hide();
            gridLayout->addWidget(btn,row,col);
            connect(btn,&QPushButton::clicked,this,[=](){
                onCardClicked(row,col);
            });
        }
    }
    gridLayout->setSpacing(10);
    gridLayout->setAlignment(Qt::AlignCenter);
    gridLayout->setContentsMargins(50,50,50,50);
}

void Widget::updateTimer(){    //计时器
    if(timeLeft>0){
        timeLeft--;
        ui->lcd_timer->display(timeLeft);
        if(timeLeft<=10){
            ui->lcd_timer->setStyleSheet("color:red;background:black;");
        }
    }else{
        gameTimer->stop();
        ui->stackedWidget->setCurrentIndex(3);
        ui->lcd_timer->setStyleSheet("color:#FF0000;background-color:rgba(0,0,0,150);border:2px solid #EAA112;");
        pathToDraw.clear();
        update();
    }
}
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    gameTimer=new QTimer(this);
    connect(gameTimer,&QTimer::timeout,this,&Widget::updateTimer);
    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("linkgame.db");
    if(db.open()){
        //qDebug()<<"数据库连接成功";
        QSqlQuery query;
        QString createTableStr="CREATE TABLE IF NOT EXISTS ranklist(         "
                               "id INTEGER PRIMARY KEY AUTOINCREMENT,        "
                               "player_name            VARCHAR(50),          "
                               "grid_size              INTEGER,              "
                               "time_use               INTEGER,              "
                               "play_time DATETIME DEFAULT CURRENT_TIMESTAMP)";
        if(!query.exec(createTableStr)){
            qDebug()<<"建表失败："<<query.lastError().text();
        }else{
            //qDebug()<<"建表成功！";
        }
    }else{
        qDebug()<<"数据库连接失败"<<db.lastError().text();
    }
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()   //再来一局
{
    ui->stackedWidget->setCurrentIndex(1);
    setOder(4);
}
void Widget::on_btnTraditional_clicked() //传统模式
{
    isLevelModel=false;
    isTraditional=true;
    TotalTime=totalTimeLimit;
    ui->lcd_timer->show();
    ui->stackedWidget->setCurrentIndex(1);
    setOder(currentOrder);
}
void Widget::onCardClicked(int row,int col){  //点击
    if(isLocked)return;
    if(map[row][col]==0)return;
    btnMap[row][col]->setStyleSheet("border:3px solid black;font-size:16px;font-weight:bold;background-color:lightgray;");
    if(isFirstClick){
        firstRow=row;
        firstCol=col;
        isFirstClick=false;

    }else{
        if(firstCol==col &&firstRow==row){   //重复点击同一个按钮
            isFirstClick=true;
            btnMap[row][col]->setStyleSheet(
                "QPushButton {"
                "   background-color: transparent;" // 背景完全透明
                "   border: none;"                 // 去掉边框
                "   color: transparent;"           // 让原本的数字文字彻底消失
                "}"
                "QPushButton:hover {"
                "   background-color: rgba(255, 255, 255, 60);" // 鼠标悬停时有个淡淡的白光，方便玩家知道选了哪个
                "   border-radius: 5px;"
                "}"
                "QPushButton:pressed {"
                "   background-color: rgba(255, 255, 255, 100);" // 按下时更亮一点
                "}"
                );
            return;
        }
        if(map[firstRow][firstCol]!=map[row][col]){  //点击的两个按钮图按不同
            isFirstClick=true;
            btnMap[firstRow][firstCol]->setStyleSheet(
                "QPushButton {"
                "   background-color: transparent;" // 背景完全透明
                "   border: none;"                 // 去掉边框
                "   color: transparent;"           // 让原本的数字文字彻底消失
                "}"
                "QPushButton:hover {"
                "   background-color: rgba(255, 255, 255, 60);" // 鼠标悬停时有个淡淡的白光，方便玩家知道选了哪个
                "   border-radius: 5px;"
                "}"
                "QPushButton:pressed {"
                "   background-color: rgba(255, 255, 255, 100);" // 按下时更亮一点
                "}"
                );
            btnMap[row][col]->setStyleSheet(
                "QPushButton {"
                "   background-color: transparent;" // 背景完全透明
                "   border: none;"                 // 去掉边框
                "   color: transparent;"           // 让原本的数字文字彻底消失
                "}"
                "QPushButton:hover {"
                "   background-color: rgba(255, 255, 255, 60);" // 鼠标悬停时有个淡淡的白光，方便玩家知道选了哪个
                "   border-radius: 5px;"
                "}"
                "QPushButton:pressed {"
                "   background-color: rgba(255, 255, 255, 100);" // 按下时更亮一点
                "}"
                );
            return;
        }
        if(game.checkPath_Graph(firstRow,firstCol,row,col)){;
            isLocked=true;
            pathToDraw=game.currentPath;
            isDrawingPath=true;
            this->update();
            int r1=firstRow;
            int c1=firstCol;
            int r2=row;
            int c2=col;
            QTimer::singleShot(500,this,[=](){
                isDrawingPath=false;
                btnMap[r1][c1]->hide();
                btnMap[r2][c2]->hide();
                map[r2][c2]=0;
                map[r1][c1]=0;
                eliminateCount+=2;
                qDebug()<<"当前消除数："<<eliminateCount<<"目标通关数："<<(order*order);
                if(eliminateCount==order*order){
                    gameTimer->stop();
                    if(isTraditional&&isLevelModel==false){
                        // 假设 timeUsed 算出来的通关秒数
                        int timeUsed = TotalTime - timeLeft;
                        // 使用 HTML 标签，单独给时间数字和评价上色（#EAA112 是 CS2 黄）
                        QString resultText = QString(
                                                 "任务完成：C4 炸弹已拆除<br>" // <br> 是换行符
                                                 "拆弹耗时：<span style='color:#EAA112; font-weight:bold;'>%1</span> 秒<br>"
                                                 "战术评价：<span style='color:#EAA112; font-weight:bold;'>全球精英 (Global Elite)</span>"
                                                 ).arg(timeUsed);

                        ui->label_timeCost->setText(resultText);
                        QTimer::singleShot(500,this,[=](){
                            bool ok;
                            QString promptText=QString("拆弹耗时：%1秒\n特种兵，请留下你的代号登入英雄榜：").arg(timeUsed);
                            QString playerName=QInputDialog::getText(this,"记录战绩",promptText,QLineEdit::Normal,"player",&ok);
                            if(ok && !playerName.isEmpty()){
                                QSqlQuery query;
                                query.prepare("INSERT INTO ranklist(player_name,grid_size,time_use)"
                                              "VALUES(:name,:size,:time)");
                                query.bindValue(":name",playerName);
                                query.bindValue(":size",order);
                                query.bindValue(":time",timeUsed);
                                if(!query.exec()){
                                    qDebug()<<"战绩录入失败："<<query.lastError().text();
                                }else{
                                    qDebug()<<"战绩录入成功";
                                }
                            }
                        });
                        // 确保 Label 支持富文本解析，并让文字居中对齐
                        ui->label_timeCost->setTextFormat(Qt::RichText);
                        ui->label_timeCost->setAlignment(Qt::AlignCenter);
                    }
                    else if(isLevelModel==true&&isTraditional==true){
                        if(currentPlayingLevel==unlockedLevel && unlockedLevel<10){
                            unlockedLevel++;
                            QLabel *toast=new QLabel(this);
                            toast->setText(QString("情报解密：成功解锁第%1关！").arg(unlockedLevel));
                            toast->setStyleSheet(
                                "background-color: rgba(234, 161, 18, 230);" // 半透明亮黄底色
                                "color: #000000;"                            // 纯黑字体
                                "font-family: 'Microsoft YaHei', sans-serif;"
                                "font-size: 18px;"
                                "font-weight: bold;"
                                "padding: 10px 20px;"                        // 内边距撑开横幅
                                "border-radius: 6px;"                        // 圆角
                                );
                            toast->adjustSize();
                            toast->move((this->width()-toast->width())/2,40);
                            toast->show();
                            toast->raise();
                            QTimer::singleShot(2500,toast,&QLabel::deleteLater);
                        }
                        int timeUsed=TotalTime-timeLeft;
                        QString resultText = QString(
                                                 "任务完成：C4 炸弹已拆除<br>"
                                                 "第 <span style='color:#EAA112; font-weight:bold;'>%1</span> 关通关<br>"
                                                 "拆弹耗时：<span style='color:#EAA112; font-weight:bold;'>%2</span> 秒<br>"
                                                 "战术评价：<span style='color:#EAA112; font-weight:bold;'>全球精英 (Global Elite)</span>"
                                                 ).arg(currentPlayingLevel).arg(timeUsed);
                        ui->label_timeCost->setText(resultText);
                        ui->label_timeCost->setTextFormat(Qt::RichText);
                        ui->label_timeCost->setAlignment(Qt::AlignCenter);
                    }
                    else{
                        ui->label_timeCost->setText("休闲模式通关！");
                    }
                    QTimer::singleShot(500,this,[=](){
                        ui->stackedWidget->setCurrentIndex(2);
                        pathToDraw.clear();
                        update();
                    });
                }
                this->update();
                isLocked=false;
            });
        }else{
            btnMap[row][col]->setStyleSheet(
                "QPushButton {"
                "   background-color: transparent;" // 背景完全透明
                "   border: none;"                 // 去掉边框
                "   color: transparent;"           // 让原本的数字文字彻底消失
                "}"
                "QPushButton:hover {"
                "   background-color: rgba(255, 255, 255, 60);" // 鼠标悬停时有个淡淡的白光，方便玩家知道选了哪个
                "   border-radius: 5px;"
                "}"
                "QPushButton:pressed {"
                "   background-color: rgba(255, 255, 255, 100);" // 按下时更亮一点
                "}"
                );
            btnMap[firstRow][firstCol]->setStyleSheet(
                "QPushButton {"
                "   background-color: transparent;" // 背景完全透明
                "   border: none;"                 // 去掉边框
                "   color: transparent;"           // 让原本的数字文字彻底消失
                "}"
                "QPushButton:hover {"
                "   background-color: rgba(255, 255, 255, 60);" // 鼠标悬停时有个淡淡的白光，方便玩家知道选了哪个
                "   border-radius: 5px;"
                "}"
                "QPushButton:pressed {"
                "   background-color: rgba(255, 255, 255, 100);" // 按下时更亮一点
                "}"
                );
        }
        isFirstClick=true;
    }
}
void Widget::paintEvent(QPaintEvent* event){  //连线
    QWidget::paintEvent(event);
    QPainter painter(this);
    if(ui->stackedWidget->currentIndex()==1){
        QPixmap bg(":/picture/2.png");
        painter.drawPixmap(0,0,this->width(),this->height(),bg);
    }
    if(!isDrawingPath || pathToDraw.size()<2)return;
    QPen pen(Qt::red,6,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
    painter.setPen(pen);
    for(size_t i=0;i < pathToDraw.size()-1;i++){
        int r1=pathToDraw[i].row;
        int c1=pathToDraw[i].col;
        int r2=pathToDraw[i+1].row;
        int c2=pathToDraw[i+1].col;
        QPoint p1=btnMap[r1][c1]->mapTo(this,btnMap[r1][c1]->rect().center());
        QPoint p2=btnMap[r2][c2]->mapTo(this,btnMap[r2][c2]->rect().center());
        painter.drawLine(p1,p2);
    }
}

void Widget::on_pushButton_3_clicked() //休闲模式
{
    isLevelModel=false;
    isTraditional=false;
    ui->lcd_timer->hide();
    ui->stackedWidget->setCurrentIndex(1);
    setOder(10);
}

void Widget::startLevel(int level){
    currentPlayingLevel=level;
    int targetLevel=4;
    int targetTime=300;
    switch(level){
        case 1:targetLevel=4;targetTime=300;break;
        case 2:targetLevel=4;targetTime=200;break;
        case 3:targetLevel=6;targetTime=200;break;
        case 4:targetLevel=8;targetTime=200;break;
        case 5:targetLevel=8;targetTime=150;break;
        case 6:targetLevel=8;targetTime=100;break;
        case 7:targetLevel=8;targetTime=60;break;
        case 8:targetLevel=10;targetTime=150;break;
        case 9:targetLevel=10;targetTime=100;break;
        case 10:targetLevel=10;targetTime=60;break;
    }
    isTraditional=true;
    isLevelModel=true;
    TotalTime=targetTime;
    timeLeft=targetTime;
    ui->stackedWidget->setCurrentIndex(1);
    setOder(targetLevel);
}

void Widget::on_pushButton_8_clicked()  //打乱功能
{
    std::vector<int> aliveCards;
    std::vector<QPoint> alivePos;
    for(int row=1;row<=order;row++){
        for(int col=1;col<=order;col++){
            if(map[row][col]>0){
                aliveCards.push_back(map[row][col]);
                alivePos.push_back(QPoint(row,col));
            }
        }
    }
    if(aliveCards.size()<2)return;
    for(int i=0;i<aliveCards.size();i++){
        int ran=QRandomGenerator::global()->bounded(aliveCards.size());
        int temp=aliveCards[i];
        aliveCards[i]=aliveCards[ran];
        aliveCards[ran]=temp;
    }
    for(size_t i=0;i<alivePos.size();i++){
        int row=alivePos[i].x();
        int col=alivePos[i].y();
        int newPid=aliveCards[i];
        map[row][col]=aliveCards[i];
        QPushButton* btn=btnMap[row][col];
        if(btn){
            QString imgPath=QString(":/picture/button%1.png").arg(newPid);
            btn->setIcon(QIcon(imgPath));
            btn->setIconSize(QSize(45,45));
        }
    }
    pathToDraw.clear();
    isFirstClick=true;
    update();
}


void Widget::on_pushButton_2_clicked()  //返回主页
{
    ui->stackedWidget->setCurrentIndex(0);
}


void Widget::on_pushButton_9_clicked() //重新开始
{
    ui->stackedWidget->setCurrentIndex(0);
}



void Widget::on_pushButton_7_clicked()  //关卡模式
{
    updateLevelButtons();
    ui->stackedWidget->setCurrentIndex(4);
}


void Widget::on_pushButton_10_clicked()//提示功能
{
    for(int r1=1;r1<=order;r1++){
        for(int c1=1;c1<=order;c1++){
            int pid1=map[r1][c1];
            if(pid1==0)continue;
            for(int r2=1;r2<=order;r2++){
                for(int c2=1;c2<=order;c2++){
                    int pid2=map[r2][c2];
                    if(r1==r2&&c1==c2)continue;
                    if(pid1==pid2&&game.checkPath_Graph(r1,c1,r2,c2)){
                        btnMap[r1][c1]->setStyleSheet("border:3px solid #00FF00;background-color:rgba(0,255,0,50);");
                        btnMap[r2][c2]->setStyleSheet("border:3px solid #00FF00;background-color:rgba(0,255,0,50);");
                        QTimer::singleShot(1500,this,[=](){
                            if(map[r1][c1]!=0){
                                btnMap[r1][c1]->setStyleSheet("background:transparent;border:none;");
                            }
                            if(map[r2][c2]!=0){
                                btnMap[r2][c2]->setStyleSheet("background:transparent;border:none;");
                            }
                        });
                        return;
                    }
                }
            }
        }
    }
    qDebug()<<"当前没有任何可以消除的方块！\n";
    on_pushButton_8_clicked();
}

void Widget::on_pushLevel_1_clicked()
{
    updateLevelButtons();
    startLevel(1);
}


void Widget::on_pushLevel_2_clicked()
{
    updateLevelButtons();
    startLevel(2);
}


void Widget::on_pushLevel_3_clicked()
{
    updateLevelButtons();
    startLevel(3);
}


void Widget::on_pushLevel_4_clicked()
{
    updateLevelButtons();
    startLevel(4);
}


void Widget::on_pushLevel_5_clicked()
{
    updateLevelButtons();
    startLevel(5);
}


void Widget::on_pushLevel_6_clicked()
{
    updateLevelButtons();
    startLevel(6);
}


void Widget::on_pushLevel_7_clicked()
{
    updateLevelButtons();
    startLevel(7);
}


void Widget::on_pushLevel_8_clicked()
{
    updateLevelButtons();
    startLevel(8);
}


void Widget::on_pushLevel_9_clicked()
{
    updateLevelButtons();
    startLevel(9);
}


void Widget::updateLevelButtons(){
    QPushButton* levels[10] = {
        ui->pushLevel_1, ui->pushLevel_2, ui->pushLevel_3, ui->pushLevel_4, ui->pushLevel_5,
        ui->pushLevel_6, ui->pushLevel_7, ui->pushLevel_8, ui->pushLevel_9, ui->pushLevel_10
    };

    // 皮肤 A：已解锁状态 (亮黄边框，鼠标悬停发光)
    QString unlockedStyle =
        "QPushButton {"
        "   background-color: rgba(30, 32, 36, 200);" /* 深色透视底板 */
        "   color: #EAA112;"                          /* CS2 警戒黄字体 */
        "   border: 2px solid #EAA112;"               /* 亮黄色边框 */
        "   border-radius: 4px;"
        "   font-family: 'Microsoft YaHei';"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #EAA112;"               /* 鼠标放上去全黄 */
        "   color: #000000;"                          /* 字体变黑 */
        "}";

    // 皮肤 B：未解锁状态 (暗灰底色，虚线边框)
    QString lockedStyle =
        "QPushButton {"
        "   background-color: rgba(40, 40, 40, 150);" /* 暗灰色底板 */
        "   color: rgba(255, 255, 255, 60);"          /* 黯淡的半透明白字 */
        "   border: 2px dashed rgba(255, 255, 255, 30);" /* 虚线边框，像未拆封的档案 */
        "   border-radius: 4px;"
        "   font-family: 'Microsoft YaHei';"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}";
    //  3. 遍历判定，分发皮肤
    for (int i = 0; i < 10; ++i) {
        int levelNum = i + 1;

        if (levelNum <= unlockedLevel) {
            // 已解锁：激活按钮，穿上亮黄皮肤
            levels[i]->setEnabled(true);
            levels[i]->setStyleSheet(unlockedStyle);
            levels[i]->setText(QString("第 %1 关").arg(levelNum));
        } else {
            // 未解锁：禁用按钮，穿上暗灰皮肤
            levels[i]->setEnabled(false);
            levels[i]->setStyleSheet(lockedStyle);
            levels[i]->setText(QString("机密档案 %1").arg(levelNum));
        }
    }
}

void Widget::on_pushLevel_10_clicked()
{
    updateLevelButtons();
    startLevel(10);
}


void Widget::on_pushButton_4_clicked()//排行榜功能
{
    QDialog *rankDialog=new QDialog(this);
    rankDialog->setWindowTitle("CSGO英雄榜");
    rankDialog->resize(700,500);
    //创建主垂直布局
    QVBoxLayout *mainLayout=new QVBoxLayout(rankDialog);
    //创建顶部选择区
    QHBoxLayout *topLayout=new QHBoxLayout();
    QLabel *label=new QLabel("选择挑战阶数：",rankDialog);
    QComboBox *comboBox=new QComboBox(rankDialog);
    comboBox->addItem("4阶模式",4);
    comboBox->addItem("6阶模式",6);
    comboBox->addItem("8阶模式",8);
    comboBox->addItem("10阶模式",10);
    topLayout->addWidget(label);
    topLayout->addWidget(comboBox);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);
    //创建数据表格
    QTableWidget *table=new QTableWidget(rankDialog);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"排名","特种兵代号","拆弹耗时（秒）"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mainLayout->addWidget(table);
    //定义更新表格函数、
    auto updateTable=[=](){
        table->setRowCount(0);
        int selectedOrder=comboBox->currentData().toInt();
        QSqlQuery query;
        query.prepare("SELECT player_name,time_use FROM ranklist "
                      "WHERE grid_size=:size ORDER BY time_use ASC LIMIT 10");
        query.bindValue(":size",selectedOrder);
        if(!query.exec()){
            qDebug()<<"查询失败："<<query.lastError().text();
        }
        int row=0;
        while(query.next()){
            table->insertRow(row);
            table->setItem(row,0,new QTableWidgetItem(QString::number(row+1)));
            table->setItem(row,1,new QTableWidgetItem(query.value(0).toString()));
            table->setItem(row,2,new QTableWidgetItem(query.value(1).toString()));
            table->item(row,0)->setTextAlignment(Qt::AlignCenter);
            table->item(row,1)->setTextAlignment(Qt::AlignCenter);
            table->item(row,2)->setTextAlignment(Qt::AlignCenter);
            row++;
        }
    };
    connect(comboBox,&QComboBox::currentIndexChanged,rankDialog,updateTable);
    updateTable();
    rankDialog->exec();
}


void Widget::on_pushButton_5_clicked()//设置功能
{
    QDialog *setDialog=new QDialog(this);
    setDialog->setWindowTitle("游戏设置");
    setDialog->resize(350,250);
    QVBoxLayout *mainLayout=new QVBoxLayout(setDialog);
    QHBoxLayout *OrderLayout=new QHBoxLayout();
    QLabel *OrderTitle=new QLabel("传统模式阶数:",setDialog);
    QLabel *OrderValue=new QLabel(QString::number(currentOrder)+"阶",setDialog);
    QFont font=OrderValue->font();
    font.setBold(true);
    font.setPointSize(12);
    OrderValue->setFont(font);
    QSlider *slider=new QSlider(Qt::Horizontal,setDialog);
    slider->setRange(4,10);
    slider->setSingleStep(2);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(2);
    slider->setValue(currentOrder);
    OrderLayout->addWidget(OrderTitle);
    OrderLayout->addWidget(OrderValue);
    OrderLayout->addWidget(slider);
    mainLayout->addLayout(OrderLayout);

    QHBoxLayout *timeLayout=new QHBoxLayout();
    QLabel *timeTitle=new QLabel("时限：",setDialog);
    QLabel *timeValue=new QLabel(QString::number(totalTimeLimit)+"秒",setDialog);
    timeValue->setFont(font);
    QSlider *timeSlider=new QSlider(Qt::Horizontal,setDialog);
    timeSlider->setRange(30,300);
    timeSlider->setSingleStep(10);
    timeSlider->setTickPosition(QSlider::TicksBelow);
    timeSlider->setTickInterval(30);
    timeSlider->setValue(totalTimeLimit);
    timeLayout->addWidget(timeTitle);
    timeLayout->addWidget(timeValue);
    timeLayout->addWidget(timeSlider);
    mainLayout->addLayout(timeLayout);

    QPushButton *btnOk=new QPushButton("确认",setDialog);
    mainLayout->addWidget(btnOk);
    connect(slider,&QSlider::valueChanged,[=](int value){
        if(value%2!=0){
            slider->setValue(value>slider->value() ? value+1:value-1);
            return;
        }
        OrderValue->setText(QString::number(slider->value())+"阶");
    });
    connect(timeSlider,&QSlider::valueChanged,[=](int value){
        timeValue->setText(QString::number(value)+"秒");
    });
    connect(btnOk,&QPushButton::clicked,[=](){
        currentOrder=slider->value();
        totalTimeLimit=timeSlider->value();
        setDialog->accept();
    });
    setDialog->exec();


}


void Widget::on_pushButton_6_clicked()//帮助
{
    QString helpText = "【特种兵拆弹手册】\n\n"
                       "1. 核心目标：找出两个图案完全相同的方块（C4）。\n"
                       "2. 拆除规则：它们之间的连线不能超过两个拐角。\n"
                       "3. 竞技模式：规定时间内清空全场，用时越短，排名越高。\n\n"
                       "祝你好运，全球精英！";

    QMessageBox::information(this, "战术简报", helpText);
}

