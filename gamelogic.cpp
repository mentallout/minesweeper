#include "gamelogic.h"

#include <QPair>
#include <QRandomGenerator>
#include <QSet>
#include <QTimer>
#include <QWidget>

GameLogic::GameLogic(bool &changeDbg, bool &leftHanded, bool &firstMove, bool &rus, int &currentWidth, int &currentHeight, int &remaining, QGridLayout *gameGrid, QObject *parent) :
    QObject(parent), changeDbg(changeDbg), isLeftHandedMode(leftHanded), isFirstMove(firstMove), isRus(rus),
    currentWidth(currentWidth), currentHeight(currentHeight), remainingMines(remaining), gameGridLayout(gameGrid)
{
}

void GameLogic::handleCellClick(Cell *cell, Qt::MouseButton button)
{
    if (isLeftHandedMode)
    {
        if (button == Qt::LeftButton)
        {
            button = Qt::RightButton;
        }
        else if (button == Qt::RightButton)
        {
            button = Qt::LeftButton;
        }
    }
    if (button == Qt::LeftButton)
    {
        if (isFirstMove)
        {
            if (cell->isMine())
            {
                placeMineSafely(cell);
            }
            isFirstMove = false;
        }
        if (cell->currentState() == Cell::Hidden)
        {
            if (cell->isMine())
            {
                revealAllCells(cell);
                QString message = "You lost!";
                if (isRus)
                {
                    message = "Вы проиграли!";
                }
                emit showMessage(":(", message);
            }
            else
            {
                cell->open();
                if (cell->adjacentMines() == 0)
                {
                    openAdjacentCells(cell);
                }
                checkWinCondition();
            }
        }
    }
    else if (button == Qt::RightButton)
    {
        if (remainingMines == 0 && cell->currentState() == Cell::Hidden)
        {
            cell->toggleFlagQuestion();
        }
        cell->toggleFlagQuestion();
    }
    else if (button == Qt::MiddleButton)
    {
        middleClick(cell);
    }
}

void GameLogic::middleClick(Cell *cell)
{
    int row = cell->row();
    int col = cell->col();
    int flagged = 0;
    int unopened = 0;
    QVector< Cell * > adjacent;
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            if (i == 0 && j == 0)
                continue;
            int newRow = row + i;
            int newCol = col + j;
            if (newRow >= 1 && newRow <= gameGridLayout->rowCount() - 1 && newCol >= 0 && newCol < gameGridLayout->columnCount())
            {
                QWidget *widget = gameGridLayout->itemAtPosition(newRow, newCol)->widget();
                if (Cell *adjCell = qobject_cast< Cell * >(widget))
                {
                    if (!adjCell->isOpened())
                    {
                        unopened++;
                        adjacent.push_back(adjCell);
                        if (adjCell->currentState() == Cell::Flagged)
                        {
                            flagged++;
                        }
                    }
                }
            }
        }
    }
    if (flagged == cell->adjacentMines())
    {
        for (Cell *adjCell : adjacent)
        {
            if (adjCell->currentState() != Cell::Flagged)
            {
                if (isLeftHandedMode)
                    handleCellClick(adjCell, Qt::RightButton);
                else
                    handleCellClick(adjCell, Qt::LeftButton);
            }
        }
    }
    else if (unopened > 0)
    {
        for (Cell *adjCell : adjacent)
        {
            if (adjCell->currentState() == Cell::Hidden)
                adjCell->setStyleSheet("border: 2px solid yellow");
        }
        QTimer::singleShot(
            1000,
            this,
            [adjacent]()
            {
                for (Cell *adjCell : adjacent)
                {
                    if (adjCell->currentState() == Cell::Hidden)
                        adjCell->setStyleSheet("color: black;");
                }
            });
    }
}

void GameLogic::placeMines(int width, int height, int mines)
{
    QSet< QPair< int, int > > minePositions;
    while (minePositions.size() < mines)
    {
        minePositions.insert(
            qMakePair(QRandomGenerator::global()->bounded(1, height + 1), QRandomGenerator::global()->bounded(0, width)));
    }
    for (auto pos : minePositions)
    {
        QWidget *widget = gameGridLayout->itemAtPosition(pos.first, pos.second)->widget();
        if (Cell *cell = qobject_cast< Cell * >(widget))
        {
            cell->setMine(true);
        }
    }
    isFirstMove = true;
}

void GameLogic::placeMineSafely(Cell *firstClickedCell)
{
    bool relocated = false;
    for (int row = 1; row <= currentHeight && !relocated; ++row)
    {
        for (int col = 0; col < currentWidth && !relocated; ++col)
        {
            Cell *cell = qobject_cast< Cell * >(gameGridLayout->itemAtPosition(row, col)->widget());
            if (cell && !cell->isMine() && cell != firstClickedCell)
            {
                firstClickedCell->setMine(false);
                cell->setMine(true);
                relocated = true;
            }
        }
    }
    calculateAdjacentMines(currentWidth, currentHeight);
}

void GameLogic::calculateAdjacentMines(int width, int height)
{
    for (int row = 1; row <= height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            Cell *cell = qobject_cast< Cell * >(gameGridLayout->itemAtPosition(row, col)->widget());
            if (!cell || cell->isMine())
                continue;
            int mineCount = 0;
            for (int i = -1; i <= 1; ++i)
            {
                for (int j = -1; j <= 1; ++j)
                {
                    if (i == 0 && j == 0)
                        continue;
                    int newRow = row + i;
                    int newCol = col + j;
                    if (newRow >= 1 && newRow <= height && newCol >= 0 && newCol < width)
                    {
                        Cell *adjCell = qobject_cast< Cell * >(gameGridLayout->itemAtPosition(newRow, newCol)->widget());
                        if (adjCell && adjCell->isMine())
                        {
                            ++mineCount;
                        }
                    }
                }
            }
            cell->setAdjacentMines(mineCount);
        }
    }
}

void GameLogic::openAdjacentCells(Cell *cell)
{
    cell->open();
    if (cell->adjacentMines() > 0)
    {
        return;
    }
    int row = cell->row();
    int col = cell->col();
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            if (i == 0 && j == 0)
                continue;
            int newRow = row + i;
            int newCol = col + j;
            if (newRow >= 1 && newRow <= gameGridLayout->rowCount() - 1 && newCol >= 0 && newCol < gameGridLayout->columnCount())
            {
                QWidget *widget = gameGridLayout->itemAtPosition(newRow, newCol)->widget();
                if (Cell *adjCell = qobject_cast< Cell * >(widget))
                {
                    if (!adjCell->isOpened() && adjCell->currentState() == Cell::Hidden)
                    {
                        openAdjacentCells(adjCell);
                    }
                }
            }
        }
    }
}

void GameLogic::revealAllCells(Cell *clickedMine)
{
    for (int row = 1; row <= gameGridLayout->rowCount() - 1; ++row)
    {
        for (int col = 0; col < gameGridLayout->columnCount(); ++col)
        {
            QWidget *widget = gameGridLayout->itemAtPosition(row, col)->widget();
            if (Cell *cell = qobject_cast< Cell * >(widget))
            {
                cell->removeFlagQuestion();
                if (cell->isMine())
                {
                    cell->open();
                    if (cell == clickedMine)
                    {
                        cell->setStyleSheet("background-color: darkred");
                    }
                }
                else
                {
                    cell->open();
                }
                cell->setEnabled(false);
            }
        }
    }
}

void GameLogic::revealSilently()
{
    for (int row = 1; row <= gameGridLayout->rowCount() - 1; ++row)
    {
        for (int col = 0; col < gameGridLayout->columnCount(); ++col)
        {
            QWidget *widget = gameGridLayout->itemAtPosition(row, col)->widget();
            if (Cell *cell = qobject_cast< Cell * >(widget))
            {
                if (!isFirstMove && cell->isMine() && cell->currentState() == Cell::Hidden)
                {
                    if (changeDbg)
                        cell->setText("M");
                    else
                        cell->setText(" ");
                }
            }
        }
    }
}

void GameLogic::checkWinCondition()
{
    int openedCells = 0;
    int totalCells = 0;
    int mineCount = 0;
    for (int row = 1; row <= gameGridLayout->rowCount() - 1; ++row)
    {
        for (int col = 0; col < gameGridLayout->columnCount(); ++col)
        {
            QWidget *widget = gameGridLayout->itemAtPosition(row, col)->widget();
            if (Cell *cell = qobject_cast< Cell * >(widget))
            {
                totalCells++;
                if (cell->isMine())
                {
                    mineCount++;
                }
                else if (cell->isOpened())
                {
                    openedCells++;
                }
            }
        }
    }
    if (openedCells == totalCells - mineCount)
    {
        revealAllCells();
        QString message = "You won!";
        if (isRus)
        {
            message = "Вы выиграли!";
        }
        emit showMessage(":)", message);
    }
}
