#define NOMINMAX 
#include "imageblurapp.h"
#include "stdafx.h"
#include <thread>

#include "imageHandler.h"
#include "windows.h"

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
    ui.asmButton->setDisabled(true);
    performanceCounter.startCounting();

    image = new ImageHandler(bmpInputFilepath.toStdString(), bmpOutputFilepath.toStdString());
    image->loadImagePart();
    uint32_t* inputBGR = image->inputHistogramCalc();
    image->blurImageDLLCPP();
    uint32_t* outputBGR = image->outputHistogramCalc();
    image->saveHeader();
    image->saveImagePart(0,0);

    createCharts(inputBGR, outputBGR);

    performanceCounter.stopCounting();
    if (performanceCounter.calculateTime()) {
        ui.cppTimeLabel->setText(QString().fromStdString(performanceCounter.getTime()));
    }
    delete image;
    ui.asmButton->setDisabled(false);
}

void ImageBlurApp::on_asmButton_clicked()
{
    //typedef int(CALLBACK* MYPROC1)(DWORD x, DWORD y);
    //typedef int(CALLBACK* MYPROC2)(DWORD x, DWORD y);

    //HINSTANCE hDLL;               // Handle to DLL
    //MYPROC1 procPtr;    // Function pointer
    //MYPROC1 procPtr2;    // Function pointer
    //int retValue;
    //hDLL = LoadLibraryA("ImageBlurDLLAsm");
    //if (hDLL != NULL)
    //{
    //    procPtr = (MYPROC1)GetProcAddress(hDLL, "MyProc1");
    //    procPtr2 = (MYPROC2)GetProcAddress(hDLL, "MyProc2");
    //    if (!procPtr || !procPtr2)
    //    {
    //        // handle the error
    //        FreeLibrary(hDLL);
    //        // here is place to some expection
    //    }
    //    else
    //    {
    //        // call the function
    //        retValue = procPtr(3, 4);
    //        retValue = procPtr2(4, 3);
    //    }

    //    FreeLibrary(hDLL);
    //}

    //performanceCounter.stopCounting();
    //if (performanceCounter.calculateTime()) {
    //    ui.cppTimeLabel->setText(QString().fromStdString(performanceCounter.getTime()));
    //}
    ////ui.cppButton->setDisabled(true);

    ui.cppButton->setDisabled(true);
    performanceCounter.startCounting();

    image = new ImageHandler(bmpInputFilepath.toStdString(), bmpOutputFilepath.toStdString());
    image->loadImagePart();
    uint32_t* inputBGR = image->inputHistogramCalc();
//    image->blurImageDLLASM();
    uint32_t* outputBGR = image->outputHistogramCalc();
    image->saveHeader();
    image->saveImagePart(0, 0);

    createCharts(inputBGR, outputBGR);

    performanceCounter.stopCounting();
    if (performanceCounter.calculateTime()) {
        ui.asmTimeLabel->setText(QString().fromStdString(performanceCounter.getTime()));
    }
    delete image;
    ui.cppButton->setDisabled(false);
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

void ImageBlurApp::createCharts(uint32_t* inputSet, uint32_t* outputSet)
{
    QChart* chartInput = new QChart();
    QChart* chartOutput = new QChart();

    QBarSet* set0In = new QBarSet("Red");
    QBarSet* set1In = new QBarSet("Green");
    QBarSet* set2In = new QBarSet("Blue");
    QBarSeries* seriesIn = new QBarSeries();
    QValueAxis* axisXIn = new QValueAxis();
    QValueAxis* axisYIn = new QValueAxis();

    QBarSet* set0Out = new QBarSet("Red");
    QBarSet* set1Out = new QBarSet("Green");
    QBarSet* set2Out = new QBarSet("Blue");
    QBarSeries* seriesOut = new QBarSeries();
    QValueAxis* axisXOut = new QValueAxis();
    QValueAxis* axisYOut = new QValueAxis();

// ========= chart 1 =========================

    uint32_t maxIn = 0;
    for (int i = 0; i < 768; i++) {
        if (inputSet[i] > maxIn) {
            maxIn = inputSet[i];
        }
    }

    for (int i = 0; i < 256; i++) {
        *set0In << inputSet[i + 512];
        *set1In << inputSet[i + 256];
        *set2In << inputSet[i];
        
    }

    set0In->setColor(QColor(255, 0, 0));
    set1In->setColor(QColor(0, 255, 0));
    set2In->setColor(QColor(0, 0, 255));
    seriesIn->append(set0In);
    seriesIn->append(set1In);
    seriesIn->append(set2In);

    chartInput->addSeries(seriesIn);
    chartInput->setTitle("Histogram obrazu wejsciowego");
    chartInput->setAnimationOptions(QChart::SeriesAnimations);

    axisYIn->setRange(0, maxIn);
    chartInput->addAxis(axisYIn, Qt::AlignLeft);
    axisXIn->setRange(0, 255);
    axisXIn->setTickCount(17);
    axisXIn->setLabelFormat("%i");
    chartInput->addAxis(axisXIn, Qt::AlignBottom);
    chartInput->legend()->setVisible(true);
    chartInput->legend()->setAlignment(Qt::AlignBottom);
    seriesIn->setBarWidth(1);
    ui.graphicsView->setChart(chartInput);
    ui.graphicsView->setRenderHint(QPainter::Antialiasing);

// ========== chart 2 ===================================

    uint32_t maxOut = 0;
    for (int i = 0; i < 768; i++) {
        if (outputSet[i] > maxOut) {
            maxOut = outputSet[i];
        }
    }
    

    for (int i = 0; i < 256; i++) {
        *set0Out << outputSet[i + 512];
        *set1Out << outputSet[i + 256];
        *set2Out << outputSet[i];
    }

    set0Out->setColor(QColor(255, 0, 0));
    set1Out->setColor(QColor(0, 255, 0));
    set2Out->setColor(QColor(0, 0, 255));
    seriesOut->append(set0Out);
    seriesOut->append(set1Out);
    seriesOut->append(set2Out);

    chartOutput->addSeries(seriesOut);
    chartOutput->setTitle("Histogram obrazu wyjsciowego");
    chartOutput->setAnimationOptions(QChart::SeriesAnimations);

    axisYOut->setRange(0, maxOut);
    chartOutput->addAxis(axisYOut, Qt::AlignLeft);
    axisXOut->setRange(0, 255);
    axisXOut->setTickCount(17);
    axisXOut->setLabelFormat("%i");
    chartOutput->addAxis(axisXOut, Qt::AlignBottom);
    chartOutput->legend()->setVisible(true);
    chartOutput->legend()->setAlignment(Qt::AlignBottom);
    seriesOut->setBarWidth(1);
    ui.graphicsView_2->setChart(chartOutput);
    ui.graphicsView_2->setRenderHint(QPainter::Antialiasing);
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