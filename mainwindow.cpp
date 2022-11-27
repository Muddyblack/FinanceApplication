#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <filesystem>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//button to add expenses
void MainWindow::on_add_expenses_Button_clicked()
{
    //DEC
    std::string db_path = "./.src/Database";
    std::string db_file = db_path + "/FinanceDataBase.db";

    QString date = ui->calendarWidget->selectedDate().toString("yyyy.MM.dd");
    QString year = date.split('.')[0];
    QString category = ui->expensesCategoryComboBox->currentText();
    QString price = ui->expensesPriceSpin->cleanText().replace(',', '.');
    QString comment = ui->expensesComment->toPlainText();


    //Create Path directory if not existent
    if(!std::filesystem::is_directory(db_path) || std::filesystem::exists(db_path))
    {
        std::filesystem::create_directories(db_path);
    }



    //connect to DB
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QString::fromStdString(db_file));

    //open DB and execute Code
    db.open();
    QSqlQuery query(db);

    //Create Table if not existing
    query.prepare("CREATE TABLE IF NOT EXISTS '" + year + "' "
                  "(buy_date DATE, "
                   "category VARCHAR(28), "
                   "price FLOAT, "
                   "comment TINYTEXT)");
    query.exec();

    //Categories Table
    query.prepare("CREATE TABLE IF NOT EXISTS Categories "
                  "(ID INTEGER, "
                   "category VARCHAR(28))");
    query.exec();


    //Add expenses to table
    query.prepare("INSERT INTO '"+ year + "' (buy_date, category, price, comment) "
                  "VALUES(:bd, :c, :p, :cc)");


    query.bindValue(":bd", date);
    query.bindValue(":c", category);
    query.bindValue(":p", price);
    query.bindValue(":cc", comment);

    query.exec();

    db.close();
}


void MainWindow::on_dashboardPushButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_expensesPushButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

