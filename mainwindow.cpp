#include "mainwindow.h"
#include "./ui_mainwindow.h"


std::string db_path = "./.src/Database";
std::string db_file = db_path + "/FinanceDataBase.db";
QString date;
QString year;
QSqlDatabase db;
time_t last_add_expense_stamp;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    date = ui->calendarWidget->selectedDate().toString("yyyy.MM.dd");
    year = date.split('.')[0];
    //Create Path directory if not existent
    if(!std::filesystem::exists(db_file))
    {
        std::filesystem::create_directories(db_path);

        //connect to DB
        db = QSqlDatabase::addDatabase("QSQLITE");
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
                      "(ID INTEGER NOT NULL, "
                       "category VARCHAR(28) PRIMARY KEY)");
        query.exec();

        std::string stnd_catg[] = {"Unkategorisiert",
                                   "Nahrungsmittel",
                                   "Unterwegs Essen",
                                   "Trinken",
                                   "Bildung",
                                   "Unterhaltung",
                                   "Auto & Transport",
                                   "Rechnungen & Dienste",
                                   "Gesundheit & Fitness",
                                   "Versicherungen",
                                   "Miete/Hypothek",
                                   "Shopping",
                                   "Urlaub",
                                   "Familie",
                                   "Lohn",
                                   "Bonus Einkommen"};
        std::string catg_sql_cmd = "INSERT INTO Categories VALUES ";

        for(int i = 0; i <= stnd_catg->length(); ++i) {
            catg_sql_cmd += "(" + std::to_string(i+1) + ", '" + stnd_catg[i] + "')";
            if(i < (stnd_catg->length())) {
                catg_sql_cmd += ", ";
            } else if(i == (stnd_catg->length())) {
                catg_sql_cmd += ";";
            }
        }
        query.prepare(QString::fromStdString(catg_sql_cmd));

        query.exec();
    }else{
        //connect to DB
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(QString::fromStdString(db_file));
    }

    update_expensesCategoryComboBox();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update_expensesCategoryComboBox(){
    db.open();
    QSqlQuery query(db);
    query.prepare("SELECT category FROM Categories");
    query.exec();
    while(query.next()) {
        QString val = query.value(0).toString();
        if(ui->expensesCategoryComboBox->findText(val) == -1) {
            ui->expensesCategoryComboBox->addItem(val);
        }
    }
}

//button to add expenses
void MainWindow::on_add_expenses_Button_clicked()
{
    //DEC
    QString category = ui->expensesCategoryComboBox->currentText();
    QString price = ui->expensesPriceSpin->cleanText().replace(',', '.');
    QString comment = ui->expensesComment->toPlainText();

    if((category.isEmpty()) || ((price.toDouble() < 0.10) && (price.toDouble() > -0.10))) {
        ui->sqlStatusLine->setText("Du brauchst einen Preis und Kategorie");
    }else if((time(0)-last_add_expense_stamp)<2) {
        ui->sqlStatusLine->setText("Zu schnell hintereinander hinzugefügt");
    }else{
        if(ui->expensesCategoryComboBox->findText(category) == -1){
            /**********
             * Missing Dialog to add new category with Image
            **********/
            QMessageBox Msgbox;
            Msgbox.setText("sum of numbers are....");
            Msgbox.exec();
        }
        db.open();
        QSqlQuery query(db);

        //Add expenses to table
        query.prepare("INSERT INTO '"+ year + "' (buy_date, category, price, comment) "
                      "VALUES(:bd, :c, :p, :cc)");


        query.bindValue(":bd", date);
        query.bindValue(":c", category);
        query.bindValue(":p", price);
        query.bindValue(":cc", comment);

        query.exec();

        db.close();
        ui->expensesPriceSpin->setValue(0);
        ui->sqlStatusLine->setText("Ausgaben hinzugefügt: "+price+"€ "+category);
        last_add_expense_stamp = time(0);
    }
}


void MainWindow::on_dashboardPushButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_expensesPushButton_clicked()
{

    ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_expensesCategoryComboBox_activated(int index)
{
    update_expensesCategoryComboBox();
}

