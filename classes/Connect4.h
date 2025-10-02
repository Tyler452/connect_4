#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bit.h"
#include "ChessSquare.h"

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
    

private:
	Grid* _grid;

	int getLowestEmptyRowForColumn(int x) const;

    int aiLowestRow(const std::string& state, int col);
	int countConsecutive(int x, int y, int dx, int dy, Player* owner) const;
    int negamax(std::string& state, int depth, int playerColor);
};

