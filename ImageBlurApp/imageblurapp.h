#pragma once

#include <QtWidgets/QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include "ui_imageblurapp.h"
#include <thread>

#include "PerformanceCounter.h"
#include "imageHandler.h"

QT_CHARTS_USE_NAMESPACE

class ImageBlurApp : public QWidget
{
    Q_OBJECT

public:
    ImageBlurApp(QWidget* parent = Q_NULLPTR);

private:
    Ui::ImageBlurAppClass ui;
    // bmp file paths
    QString bmpInputFilepath;
    QString bmpOutputFilepath;
    bool isLoadPathAssigned = false;
    bool isSavePathAssigned = false;
    ImageHandler* image;
    // computer threads count
    unsigned int processor_count;
    // time counter
    PerformanceCounter performanceCounter;
    // charts for histogram
    QChart* chartInput = new QChart();
    QChart* chartOutput = new QChart();
    bool isShownGraphic1 = false;
    bool isShownGraphic2 = false;

private slots:
    void on_fileLoadPathButton_clicked();
    void on_fileSavePathButton_clicked();
    void on_cppButton_clicked();
    void on_asmButton_clicked();
    void on_endButton_clicked();
    void on_histogramButton_clicked();
};
