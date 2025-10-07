#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bit.h"
#include "ChessSquare.h"
#include <string>

class Connect4 : public Game {
public:
    Connect4();
    ~Connect4();

    static const int CONNECT4_COLS = 7;
    static const int CONNECT4_ROWS = 6;

    // Game interface
    void setUpBoard() override;
    Bit* createPiece(Player* player);
    bool actionForEmptyHolder(BitHolder &holder) override;
    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    Player* checkForWinner() override;
    bool checkForDraw() override;
    void stopGame() override;
    void updateAI() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    Grid* getGrid() override { return _grid; }
    bool gameHasAI() override { return true; }

private:
    Grid* _grid;

    static const int EMPTY = 0;
    static const int RED_PIECE = 1;
    static const int YELLOW_PIECE = 2;

    // Internal helpers
    Bit* createPiece(int pieceType);
    int countConsecutive(int x, int y, int dx, int dy, Player* owner) const;
    int getLowestEmptyRowForColumn(int x) const;

    // AI helpers
    int aiLowestRow(const std::string& state, int col);
    int negamax(std::string& state, int depth, int playerColor);
    int negamax(std::string& state, int depth, int playerColor, int alpha, int beta);
};
