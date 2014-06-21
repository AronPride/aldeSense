/***************************************************************************
**                                                                        **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2011, 2012, 2013, 2014 Emanuel Eichhammer               **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: http://www.qcustomplot.com/                          **
**             Date: 07.04.14                                             **
**          Version: 1.2.1                                                **
****************************************************************************/

/************************************************************************************************************
**                                                                                                         **
**  This is the example code for QCustomPlot.                                                              **
**                                                                                                         **
**  It demonstrates basic and some advanced capabilities of the widget. The interesting code is inside     **
**  the "setup(...)Demo" functions of MainWindow.                                                          **
**                                                                                                         **
**  In order to see a demo in action, call the respective "setup(...)Demo" function inside the             **
**  MainWindow constructor. Alternatively you may call setupDemo(i) where i is the index of the demo       **
**  you want (for those, see MainWindow constructor comments). All other functions here are merely a       **
**  way to easily create screenshots of all demos for the website. I.e. a timer is set to successively     **
**  setup all the demos and make a screenshot of the window area and save it in the ./screenshots          **
**  directory.                                                                                             **
**                                                                                                         **
*************************************************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>
#include <QSerialPort>
#include <QSerialPortInfo>

QSerialPort serial;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serialCheck();

    setGeometry(400, 250, 542, 390);

    setupDemo(1);
    // 10: setupRealtimeDataDemo(ui->customPlot);

    // for making screenshots of the current demo or all demos (for website screenshots):
    //QTimer::singleShot(1500, this, SLOT(allScreenShots()));
    //QTimer::singleShot(4000, this, SLOT(screenShot()));
}

void MainWindow::setupDemo(int demoIndex)
{


    switch (demoIndex)
    {
    case 1: setupRealtimeDataDemo(ui->customPlot); break;
    }
    setWindowTitle("QCustomPlot: "+demoName);
    statusBar()->clearMessage();
    currentDemoIndex = demoIndex;
    ui->customPlot->replot();
}

void MainWindow::serialCheck() {
    serial.setPortName("com6");
    if (serial.open(QIODevice::ReadWrite))
    {
        serial.setBaudRate(QSerialPort::Baud9600);
        serial.setDataBits(QSerialPort::Data8);
        serial.setParity(QSerialPort::NoParity);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setFlowControl(QSerialPort::NoFlowControl);
        ui->statusBar->showMessage(
                    QString("COM Port Successfully Linked"));

    }
    else{
        serial.close();
        ui->statusBar->showMessage(
                    QString("Unable to Reach COM Port"));

    }
}


void MainWindow::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    QMessageBox::critical(this, "", "You're using Qt < 4.7, the realtime data demo needs functions that are available with Qt 4.7 to work properly");
#endif
    demoName = "Real Time Data Demo";

    // include this section to fully disable antialiasing for higher performance:
    /*
  customPlot->setNotAntialiasedElements(QCP::aeAll);
  QFont font;
  font.setStyleStrategy(QFont::NoAntialias);
  customPlot->xAxis->setTickLabelFont(font);
  customPlot->yAxis->setTickLabelFont(font);
  customPlot->legend->setFont(font);
  */
    customPlot->addGraph(); // blue line
    customPlot->graph(0)->setPen(QPen(Qt::blue));
    customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    customPlot->graph(0)->setAntialiasedFill(false);
    customPlot->addGraph(); // red line
    customPlot->graph(1)->setPen(QPen(Qt::red));
    customPlot->graph(0)->setChannelFillGraph(customPlot->graph(1));

    customPlot->addGraph(); // blue dot
    customPlot->graph(2)->setPen(QPen(Qt::blue));
    customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    customPlot->addGraph(); // red dot
    customPlot->graph(3)->setPen(QPen(Qt::red));
    customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
    customPlot->xAxis->setAutoTickStep(false);
    customPlot->xAxis->setTickStep(2);
    customPlot->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}



void MainWindow::realtimeDataSlot()
{
    // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    double key = 0;
#else
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.01) // at most add point every 10 ms
    {
        double value0 = qSin(key); //sin(key*1.6+cos(key*1.7)*2)*10 + sin(key*1.2+0.56)*20 + 26;
        double value1 = qCos(key); //sin(key*1.3+cos(key*1.2)*1.2)*7 + sin(key*0.9+0.26)*24 + 26;
        // add data to lines:
        ui->customPlot->graph(0)->addData(key, value0);
        ui->customPlot->graph(1)->addData(key, value1);
        // set data of dots:
        ui->customPlot->graph(2)->clearData();
        ui->customPlot->graph(2)->addData(key, value0);
        ui->customPlot->graph(3)->clearData();
        ui->customPlot->graph(3)->addData(key, value1);
        // remove data of lines that's outside visible range:
        ui->customPlot->graph(0)->removeDataBefore(key-8);
        ui->customPlot->graph(1)->removeDataBefore(key-8);
        // rescale value (vertical) axis to fit the current data:
        ui->customPlot->graph(0)->rescaleValueAxis();
        ui->customPlot->graph(1)->rescaleValueAxis(true);
        lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
    ui->customPlot->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
        ui->statusBar->showMessage(
                    QString("%1 FPS, Total Data points: %2")
                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
                    .arg(ui->customPlot->graph(0)->data()->count()+ui->customPlot->graph(1)->data()->count())
                    , 0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

void MainWindow::bracketDataSlot()
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    double secs = 0;
#else
    double secs = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif

    // update data to make phase move:
    int n = 500;
    double phase = secs*5;
    double k = 3;
    QVector<double> x(n), y(n);
    for (int i=0; i<n; ++i)
    {
        x[i] = i/(double)(n-1)*34 - 17;
        y[i] = qExp(-x[i]*x[i]/20.0)*qSin(k*x[i]+phase);
    }
    ui->customPlot->graph()->setData(x, y);

    itemDemoPhaseTracer->setGraphKey((8*M_PI+fmod(M_PI*1.5-phase, 6*M_PI))/k);

    ui->customPlot->replot();

    // calculate frames per second:
    double key = secs;
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
        ui->statusBar->showMessage(
                    QString("%1 FPS, Total Data points: %2")
                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
                    .arg(ui->customPlot->graph(0)->data()->count())
                    , 0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    serial.close();
}

void MainWindow::screenShot()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QPixmap pm = QPixmap::grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#else
    QPixmap pm = qApp->primaryScreen()->grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#endif
    QString fileName = demoName.toLower()+".png";
    fileName.replace(" ", "");
    pm.save("./screenshots/"+fileName);
    qApp->quit();
}

void MainWindow::allScreenShots()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QPixmap pm = QPixmap::grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#else
    QPixmap pm = qApp->primaryScreen()->grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#endif
    QString fileName = demoName.toLower()+".png";
    fileName.replace(" ", "");
    pm.save("./screenshots/"+fileName);

    if (currentDemoIndex < 18)
    {
        if (dataTimer.isActive())
            dataTimer.stop();
        dataTimer.disconnect();
        delete ui->customPlot;
        ui->customPlot = new QCustomPlot(ui->centralWidget);
        ui->verticalLayout->addWidget(ui->customPlot);
        setupDemo(currentDemoIndex+1);
        // setup delay for demos that need time to develop proper look:
        int delay = 250;
        if (currentDemoIndex == 10) // Next is Realtime data demo
            delay = 12000;
        else if (currentDemoIndex == 15) // Next is Item demo
            delay = 5000;
        QTimer::singleShot(delay, this, SLOT(allScreenShots()));
    } else
    {
        qApp->quit();
    }
}

//Button Functionality
//*  It measures continuously the voltage on pin A9,
//*  Write 'c' to turn on lightPin (11),
//*  Write 'a' to turn off lightPin (11);
//*  Write 'sample' to sample (200 times in 1 second)

void MainWindow::on_pushButton_clicked()
{
    serial.write("c");
}

void MainWindow::on_pushButton_2_clicked()
{
    serial.write("a");
}

void MainWindow::on_pushButton_3_clicked()
{
    serial.write("sample");
}
