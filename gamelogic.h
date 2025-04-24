#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "cell.h"

#include <QGridLayout>
#include <QObject>

class GameLogic : public QObject
{
    Q_OBJECT

public:
    GameLogic(bool &changeDbg, bool &leftHanded, bool &firstMove, bool &rus, int &currentWidth, int &currentHeight, int &remaining, QGridLayout *gameGrid, QObject *parent = nullptr);

    void handleCellClick(Cell *cell, Qt::MouseButton button);
    void middleClick(Cell *cell);
    void placeMines(int width, int height, int mines);
    void placeMineSafely(Cell *firstClickedCell);
    void calculateAdjacentMines(int width, int height);
    void openAdjacentCells(Cell *cell);
    void revealAllCells(Cell *clickedMine = nullptr);
    void revealSilently();
    void checkWinCondition();

signals:
    void showMessage(const QString &message1, const QString &message2);

private:
    bool &changeDbg;
    bool &isLeftHandedMode;
    bool &isFirstMove;
    bool &isRus;

    int &currentWidth;
    int &currentHeight;
    int &remainingMines;

    QGridLayout *gameGridLayout;
};

#endif	  // GAMELOGIC_H
