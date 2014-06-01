#include "MainWindow.h"
#include "ui_MainWindow.h"

void MainWindow::closeEvent(QCloseEvent* event)
{
	m_sinusOnJack->stop();
	exit(0);
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	m_sinusOnJack = new SinusOnJack(this);
	m_sinusOnJack->start();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_soundButton_clicked()
{
		// C'est ici qu'on demande au son de bien vouloir sortir s'il vous plait!
}
