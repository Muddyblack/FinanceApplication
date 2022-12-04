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
    update_Dashboard_Time_ComboBox();
    create_DonutChart();
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
    db.close();
}

void MainWindow::update_Dashboard_Time_ComboBox() {
    db.open();
    QSqlQuery query(db);
    query.prepare("SELECT name FROM sqlite_schema WHERE type ='table' AND name NOT LIKE 'sqlite_%' AND name NOT LIKE 'Categories';");
    query.exec();
    while(query.next()) {
        QString val = query.value(0).toString();
        if(ui->dash_year_comboBox->findText(val) == -1) {
            ui->dash_year_comboBox->addItem(val);
        }
    }
    ui->dash_year_comboBox->addItem("Alle");
    db.close();
}

void MainWindow::create_DonutChart(){
    QString selected_year = ui->dash_year_comboBox->currentText();
    QString selected_month = ui->dash_monthcomboBox->currentText();
    QString selected_day = ui->dash_daycomboBox->currentText();
    QString sql_date_command = " SELECT category, SUM(price) FROM '";

    if(selected_year == "Alle") {
        sql_date_command += "* ";
    }else{
        sql_date_command += selected_year + "' ";

        if(selected_month != "Alle") {
            sql_date_command += "WHERE buy_date LIKE '" + selected_year + "." + selected_month + ".";

            if(selected_day != "Alle") {
                sql_date_command += selected_day;
            }else{
                sql_date_command += "%";
            }

            sql_date_command += "' ";
        }
    }
    sql_date_command += "GROUP BY category ";


    db.open();
    QSqlQuery query(db);

    query.prepare(sql_date_command);
    query.exec();

    QPieSeries *series = new QPieSeries();
    series->setHoleSize(0.35);


    while(query.next()) {
        QString cat = query.value(0).toString();
        double price = query.value(1).toDouble();
        series->append(cat, price);
    }

    db.close();

    //QPieSlice *slice;
    //slice->setExploded();
    series->setLabelsVisible();

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setAnimationDuration(QChart::SeriesAnimations);
    chart->setTitle("");

    QChartView *chartview = new QChartView(chart);
    chartview->setRenderHint(QPainter::Antialiasing);

    chartview->setParent(ui->expense_chartframe);
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
    create_DonutChart();
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

