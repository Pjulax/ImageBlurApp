#define NOMINMAX 
#include "imageblurapp.h"
#include "stdafx.h"
#include <thread>

#include "bmpfile.h"
#include "windows.h"

QBarSet* set0 = new QBarSet("Red");
QBarSet* set1 = new QBarSet("Green");
QBarSet* set2 = new QBarSet("Blue");

QBarSeries* series = new QBarSeries();

QStringList categories;
QValueAxis* axisX = new QValueAxis();
QValueAxis* axisY = new QValueAxis();


ImageBlurApp::ImageBlurApp(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.cppButton->setDisabled(true);
    ui.asmButton->setDisabled(true);
    processor_count = std::thread::hardware_concurrency();
    QString threadNum = QString::number(processor_count - 1);
    QString threadText = QString("Optymalnie uzyj: ");
    threadText.push_back(threadNum);
    threadText.push_back(" watkow");
    ui.threadLabel->setText(threadText);
    performanceCounter = PerformanceCounter();


    for (int i = 0; i < 128; i++) {
        *set0 << i%75;
        *set1 << (i%35)+31;
        *set2 << (i+31)%97;
    }
    set0->setColor(QColor(255,0,0));
    set1->setColor(QColor(0,255,0));
    set2->setColor(QColor(0,0,255));
    series->append(set0);
    series->append(set1);
    series->append(set2);

    chartInput->addSeries(series);
    chartInput->setTitle("Histogram obrazu wejsciowego");
    chartInput->setAnimationOptions(QChart::SeriesAnimations);

    for (int i = 0; i < 128; i++) {
        categories << QString().fromStdString(std::to_string(i));
    }

    axisY->setRange(0, 100);
    chartInput->addAxis(axisY, Qt::AlignLeft);
    axisX->setRange(0, 255);
    axisX->setTickCount(17);
    axisX->setLabelFormat("%i");
    chartInput->addAxis(axisX, Qt::AlignBottom);
    chartInput->legend()->setVisible(true);
    chartInput->legend()->setAlignment(Qt::AlignBottom);
    series->setBarWidth(1);
    ui.graphicsView->setChart(chartInput);
    ui.graphicsView->setRenderHint(QPainter::Antialiasing);
    ui.asmButton->setEnabled(true);
}

void ImageBlurApp::on_fileLoadPathButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Wybierz obraz"), "", tr("Obrazy (*.bmp)"));
    if (!filePath.isEmpty()) {
        
        bmpInputFilepath = filePath;
        ui.fileLoadPathEdit->setText(QString(filePath));
        isLoadPathAssigned = true;

        if (isSavePathAssigned) {
            ui.cppButton->setEnabled(true);
            ui.asmButton->setEnabled(true);
        }
    }
}

void ImageBlurApp::on_fileSavePathButton_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Wybierz miejsce do zapisu"), "", tr("Obrazy (*.bmp)"));
    if (!filePath.isEmpty()) {

        bmpOutputFilepath = filePath;
        ui.fileSavePathEdit->setText(QString(filePath));
        isSavePathAssigned = true;

        if (isLoadPathAssigned) {
            ui.cppButton->setEnabled(true);
            ui.asmButton->setEnabled(true);
        }
    }

}

void ImageBlurApp::on_cppButton_clicked()
{
    performanceCounter.startCounting();


    //ui.asmButton->setDisabled(true);
    //Bitmap bitmap(ui.fileSavePathEdit->text().toStdString());
}

void ImageBlurApp::on_asmButton_clicked()
{
    

    typedef int(CALLBACK* MYPROC1)(DWORD x, DWORD y);
    typedef int(CALLBACK* MYPROC2)(DWORD x, DWORD y);

    HINSTANCE hDLL;               // Handle to DLL
    MYPROC1 procPtr;    // Function pointer
    MYPROC1 procPtr2;    // Function pointer
    int retValue;
    hDLL = LoadLibraryA("ImageBlurDLLAsm");
    if (hDLL != NULL)
    {
        procPtr = (MYPROC1)GetProcAddress(hDLL, "MyProc1");
        procPtr2 = (MYPROC2)GetProcAddress(hDLL, "MyProc2");
        if (!procPtr || !procPtr2)
        {
            // handle the error
            FreeLibrary(hDLL);
            // here is place to some expection
        }
        else
        {
            // call the function
            retValue = procPtr(3, 4);
            retValue = procPtr2(4, 3);
        }

        FreeLibrary(hDLL);
    }

    performanceCounter.stopCounting();
    if (performanceCounter.calculateTime()) {
        ui.cppTimeLabel->setText(QString().fromStdString(performanceCounter.getTime()));
    }
    //ui.cppButton->setDisabled(true);

}

void ImageBlurApp::on_endButton_clicked()
{

    this->close();
}

void ImageBlurApp::on_histogramButton_clicked()
{
    if (isShownGraphic1) {
        ui.histogramButton->setText("Histogram wejsciowy");
        ui.graphicsView_2->show();
        ui.graphicsView->hide();
        isShownGraphic2 = true;
        isShownGraphic1 = false;
    }
    else {
        ui.histogramButton->setText("Histogram wyjsciowy");
        ui.graphicsView->show();
        ui.graphicsView_2->hide();
        isShownGraphic1 = true;
        isShownGraphic2 = false;
    }
}


//typedef void(CALLBACK* FIBONACCI_INIT)(const unsigned long long uParam1, const unsigned long long uParam2);
//typedef bool(CALLBACK* FIBONACCI_NEXT)();
//typedef unsigned long long(CALLBACK* FIBONACCI_CURRENT)();
//typedef unsigned int(CALLBACK* FIBONACCI_INDEX)();
//typedef int(CALLBACK* MYPROC1)(DWORD x, DWORD y);
//
//HINSTANCE hDLL;               // Handle to DLL
//FIBONACCI_INIT initfun;    // Function pointer
//FIBONACCI_NEXT nextfun;    // Function pointer
//FIBONACCI_CURRENT currentfun;    // Function pointer
//FIBONACCI_INDEX indexfun;    // Function pointer
//MYPROC1 indexfun;    // Function pointer
//const unsigned long long uParam1 = 1, uParam2 = 1;
//unsigned long long number;
//unsigned int index;

//typedef int(CALLBACK* MYPROC1)(DWORD x, DWORD y);
//
//HINSTANCE hDLL;               // Handle to DLL
//MYPROC1 procPtr;    // Function pointer
//int retValue;
//    hDLL = LoadLibraryA("ImageBlurDLLCpp");
//    if (hDLL != NULL)
//    {
//        proc = (MYPROC1)GetProcAddress(hDLL, "MyProc1");
//        if (!proc)
//        {
//            // handle the error
//            FreeLibrary(hDLL);
//            // here is place to some expection
//        }
//        else
//        {
//            // call the function
//             retValue = procPtr(3,4);
//        }
//
//        FreeLibrary(hDLL);
//    }
//}


//void LoadAndCallSomeFunction()
//{
//    HINSTANCE hDLL;               // Handle to DLL
//    FIBONACCI_INIT initfun;    // Function pointer
//    FIBONACCI_NEXT nextfun;    // Function pointer
//    FIBONACCI_CURRENT currentfun;    // Function pointer
//    FIBONACCI_INDEX indexfun;    // Function pointer
//    const unsigned long long uParam1 = 1, uParam2 = 1;
//
//    hDLL = LoadLibraryA("ImageBlurDLLCpp");
//    if (hDLL != NULL)
//    {
//        initfun = (FIBONACCI_INIT)GetProcAddress(hDLL, "fibonacci_init");
//        nextfun = (FIBONACCI_NEXT)GetProcAddress(hDLL, "fibonacci_next");
//        currentfun = (FIBONACCI_CURRENT)GetProcAddress(hDLL, "fibonacci_current");
//        indexfun = (FIBONACCI_INDEX)GetProcAddress(hDLL, "fibonacci_index");
//        if (!initfun || !nextfun || !currentfun || !indexfun)
//        {
//            // handle the error
//            FreeLibrary(hDLL);
//            // here is place to some expection
//        }
//        else
//        {
//            // call the function
//            initfun(uParam1, uParam2);
//            unsigned long long number;
//            unsigned int index;
//            do {
//                index = indexfun();
//                number = currentfun();
//                
//            } while (nextfun());
//
//        }
//
//        FreeLibrary(hDLL);
//    }
//}