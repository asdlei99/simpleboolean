#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QWidget>

class RenderWidget;

class MainWindow : public QWidget 
{
    Q_OBJECT

public:
    MainWindow(void);

protected:
    void keyPressEvent(QKeyEvent *event);

private:
	RenderWidget *m_renderWidget;
};

#endif