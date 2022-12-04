#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <filesystem>
#include <time.h>
#include <unistd.h>
#include <QtCharts>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void update_expensesCategoryComboBox();

    void create_DonutChart();

    void on_add_expenses_Button_clicked();

    void on_dashboardPushButton_clicked();

    void on_expensesPushButton_clicked();

    void on_expensesCategoryComboBox_activated(int index);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
