#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gamelogic.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QToolBar>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(bool dbg, QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void displayMessage(const QString &message1, const QString &message2);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    bool isFirstMove = true;
    bool isLeftHandedMode = false;
    bool isDbg = false;
    bool isRus = false;
    bool changeDbg = false;
    bool validateInput(int &width, int &height, int &mines);

    int remainingMines = 0;
    int currentWidth = 0;
    int currentHeight = 0;
    int currentMines = 0;

    void cleaning();
    void startNewGame();
    void createMenu();
    void createGameArea(int width, int height, int mines);
    void saveGameState();
    void loadGameState();
    void restartWithSameParameters();
    void restartWithNewParameters();
    void enRuMenu();
    void ruEnMenu();
    void enRuGame();
    void ruEnGame();

    QWidget *gameAreaWidget;
    GameLogic *gameLogic = nullptr;
    QLabel *mineCounterLabel = nullptr;
    QLabel *widthLabel = nullptr;
    QLabel *heightLabel = nullptr;
    QLabel *minesLabel = nullptr;
    QLineEdit *widthInput = nullptr;
    QLineEdit *heightInput = nullptr;
    QLineEdit *minesInput = nullptr;
    QGridLayout *gameGridLayout = nullptr;
    QToolBar *toolBar = nullptr;
    QPushButton *startButton = nullptr;
    QPushButton *changeEngRus = nullptr;
    QPushButton *changeRusEng = nullptr;
    QAction *sameNewGame = nullptr;
    QAction *newNewGame = nullptr;
    QAction *leftHanded = nullptr;
    QAction *dbgMode = nullptr;
    QAction *changeEnRu = nullptr;
    QAction *changeRuEn = nullptr;
    QString getIniFilePath() const;
};

#endif	  // MAINWINDOW_H
