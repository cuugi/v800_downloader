#include "v800main.h"
#include "ui_v800main.h"
#include "v800usb.h"

#include <QThread>
#include <QMessageBox>

V800Main::V800Main(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::V800Main)
{
    qRegisterMetaType<QList<QByteArray> >("QList<QByteArray>");

    v800_ready = false;
    session_cnt = 0;

    QThread *usb_thread = new QThread;
    usb = new V800usb();
    usb->moveToThread(usb_thread);

    connect(usb, SIGNAL(all_sessions(QList<QByteArray>)), this, SLOT(handle_all_sessions(QList<QByteArray>)));
    connect(usb, SIGNAL(session_done()), this, SLOT(handle_session_done()));
    connect(usb, SIGNAL(ready()), this, SLOT(handle_ready()));
    connect(usb, SIGNAL(not_ready()), this, SLOT(handle_not_ready()));
    connect(this, SIGNAL(get_session(QByteArray)), usb, SLOT(get_session(QByteArray)));

    connect(usb_thread, SIGNAL(started()), usb, SLOT(loop()));
    usb_thread->start();

    ui->setupUi(this);
    ui->exerciseTree->setColumnCount(1);
    ui->exerciseTree->setHeaderLabel("Session");

    ui->downloadBtn->setEnabled(false);
    ui->exerciseTree->setEnabled(false);
}

V800Main::~V800Main()
{
    delete ui;
    delete usb;
}

void V800Main::handle_not_ready()
{
    QMessageBox failure;
    failure.setText("Failed to open V800!");
    failure.setIcon(QMessageBox::Critical);
    failure.exec();

    exit(-1);
}

void V800Main::handle_ready()
{
    v800_ready = true;
    ui->downloadBtn->setEnabled(true);
    ui->exerciseTree->setEnabled(true);
}

void V800Main::handle_all_sessions(QList<QByteArray> sessions)
{
    QList<QTreeWidgetItem *> items;
    int sessions_iter = 0;
    for(sessions_iter = 0; sessions_iter < sessions.length(); sessions_iter++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget *)0, QStringList(QString(sessions[sessions_iter].constData())));
        item->setCheckState(0, Qt::Unchecked);
        items.append(item);
    }
    ui->exerciseTree->insertTopLevelItems(0, items);
}

void V800Main::handle_session_done()
{
    session_cnt--;
    if(session_cnt == 0)
    {
        ui->exerciseTree->setEnabled(true);
        ui->downloadBtn->setEnabled(true);
    }
}

void V800Main::on_downloadBtn_clicked()
{
    int item_iter;

    ui->exerciseTree->setEnabled(false);
    ui->downloadBtn->setEnabled(false);

    for(item_iter = 0; item_iter < ui->exerciseTree->topLevelItemCount(); item_iter++)
    {
        // get all the files if this exercise is checked
        if(ui->exerciseTree->topLevelItem(item_iter)->checkState(0) == Qt::Checked)
        {
            session_cnt++;
            emit get_session(ui->exerciseTree->topLevelItem(item_iter)->text(0).toLatin1());
        }
    }
}
