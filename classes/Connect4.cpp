#include "Connect4.h"

Connect4::Connect4() : Game() {
    _grid = new Grid(Connect4::CONNECT4_COLS, Connect4::CONNECT4_ROWS);
}

Connect4::~Connect4() {
	delete _grid;
}

void Connect4::setUpBoard() {
	// TODO: implement board setup (initialize grid, pieces, options)
}

Bit* Connect4::createPiece(Player* /*player*/) {
    // Simple placeholder: create a Bit with no sprite (calling Bit default ctor if exists)
    return new Bit();
}

int Connect4::getLowestEmptyRowForColumn(int x) const {
    for (int row = Connect4::CONNECT4_ROWS - 1; row >= 0; --row) {
        BitHolder* sq = _grid->getSquare(x, row);
        if (sq && sq->empty()) return row;
    }
    return -1;
}

bool Connect4::actionForEmptyHolder(BitHolder &/*holder*/) {
	// TODO: handle a click on an empty holder (drop piece into column)
	return false;
}

bool Connect4::canBitMoveFrom(Bit &/*bit*/, BitHolder &/*src*/) {
	// Pieces are typically not movable after placement in Connect4
	return false;
}

bool Connect4::canBitMoveFromTo(Bit &/*bit*/, BitHolder &/*src*/, BitHolder &/*dst*/) {
	return false;
}

int Connect4::countConsecutive(int /*x*/, int /*y*/, int /*dx*/, int /*dy*/, Player* /*owner*/) const {
	// TODO: count consecutive pieces from (x,y) in direction (dx,dy)
	return 0;
}

Player* Connect4::checkForWinner() {
	// TODO: implement winner detection
	return nullptr;
}

bool Connect4::checkForDraw() {
	// TODO: implement draw detection
	return false;
}

void Connect4::stopGame() {
	// TODO: clear board and free resources specific to a game session
}

std::string Connect4::initialStateString() {
	// Default empty-board representation (42 zeros)
    return std::string(Connect4::CONNECT4_COLS * Connect4::CONNECT4_ROWS, '0');
}

std::string Connect4::stateString() {
	// TODO: serialize board state to a string
	return std::string();
}

void Connect4::setStateString(const std::string &/*s*/) {
	// TODO: deserialize board state from a string
}

int Connect4::aiLowestRow(const std::string& state, int col) {
    for (int row = Connect4::CONNECT4_ROWS - 1; row >= 0; --row) {
        int idx = row * Connect4::CONNECT4_COLS + col;
        if (idx >= 0 && idx < (Connect4::CONNECT4_ROWS * Connect4::CONNECT4_COLS) && state[idx] == '0') return row;
    }
    return -1; 
}

void Connect4::updateAI() 
{
    int bestVal = -1000;
    BitHolder* bestMove = nullptr;
    std::string state = stateString();

    for (int x = 0; x < Connect4::CONNECT4_COLS; ++x) {
        int lowestRow = aiLowestRow(state, x);
        if (lowestRow < 0) continue; 
        int index = lowestRow * Connect4::CONNECT4_COLS + x;
        state[index] = '2';
        int moveVal = -negamax(state, 0, HUMAN_PLAYER);
        state[index] = '0';
        if (moveVal > bestVal) {
            bestVal = moveVal;
            bestMove = _grid->getSquare(x, lowestRow);
        }
    }


    if(bestMove) {
        if (actionForEmptyHolder(*bestMove)) {
        }
    }
}

bool isAIBoardFull(const std::string& state) {
    return state.find('0') == std::string::npos;
}

int evaluateAIBoard(const std::string& state) {
    const int values[] = {1,1,2,3,2,1,1,
                          1,1,2,3,2,1,1,
                          1,1,2,3,2,1,1,
                          1,1,2,5,2,1,1,
                          2,1,2,3,2,1,1,
                          2,1,2,3,2,1,1};
    int value = 0;
    for (int index = 0; index < (Connect4::CONNECT4_ROWS * Connect4::CONNECT4_COLS); ++index) {
        char piece = state[index];
        if (piece == '0') continue;
        if (piece == '2') { // AI
            value += values[index];
        } else if (piece == '1') { // Human
            value -= values[index];
        }
    }
    return value; // No winner
}

int Connect4::negamax(std::string& state, int depth, int playerColor) 
{
    int score = evaluateAIBoard(state);

    if(depth == 5) { 
        return score * playerColor; 
    }

    if(isAIBoardFull(state)) {
        return 0; 
    }

    int bestVal = -1000;
    for (int x = 0; x < Connect4::CONNECT4_COLS; ++x) {
        int lowestRow = aiLowestRow(state, x);
        if (lowestRow < 0) continue;
        int index = lowestRow * Connect4::CONNECT4_COLS + x;
        state[index] = (playerColor == HUMAN_PLAYER) ? '1' : '2';
        int val = -negamax(state, depth + 1, -playerColor);
        state[index] = '0';
        bestVal = std::max(bestVal, val);
    }
    return bestVal;
}

