#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "SinusOnJack.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	SinusOnJack* m_sinusOnJack;

protected:
	void closeEvent(QCloseEvent * event);

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_soundButton_clicked();

	void on_gainSlider_valueChanged(int value);

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
