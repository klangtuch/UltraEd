#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_scene(new UltraEd::Scene)
{
    m_ui->setupUi(this);
    m_scene->Create(reinterpret_cast<HWND>(m_ui->scene->winId()));
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}
