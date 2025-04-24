#ifndef CELL_H
#define CELL_H

#include <QPushButton>

class Cell : public QPushButton
{
    Q_OBJECT

public:
    enum State
    {
        Hidden,
        Opened,
        Flagged,
        Question
    };
    Cell(int row, int col, QWidget *parent = nullptr);

    bool isOpened() const;
    bool isMine() const;

    int row() const;
    int col() const;
    int adjacentMines() const;

    void open();
    void setMine(bool hasMine);
    void setAdjacentMines(int count);
    void toggleFlagQuestion();
    void removeFlagQuestion();

    State currentState() const;

signals:
    void cellClicked(Cell *cell, Qt::MouseButton button);
    void flagChanged(int change);
    void openAdjacentCells(Cell *cell);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    bool m_isMine;

    int m_row;
    int m_col;
    int m_adjacentMines;

    State m_state;
};

#endif	  // CELL_H
