#include "Connect4.h"

#include <vector>
#include <algorithm>
#include <limits>

Connect4::Connect4() : Game() {
    _grid = new Grid(CONNECT4_COLS, CONNECT4_ROWS);
}

Connect4::~Connect4() {
    delete _grid;
}

void Connect4::setUpBoard() {
    setNumberOfPlayers(2);
    _gameOptions.rowX = CONNECT4_COLS;
    _gameOptions.rowY = CONNECT4_ROWS;

    _grid->initializeSquares(80, "square.png");

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }

    startGame();
}

Bit* Connect4::createPiece(Player* player) {
    Bit* bit = new Bit();
    bool isRed = (player == getPlayerAt(0));
    bit->LoadTextureFromFile(isRed ? "red.png" : "yellow.png");
    bit->setOwner(player);
    bit->setGameTag(isRed ? RED_PIECE : YELLOW_PIECE);
    return bit;
}

Bit* Connect4::createPiece(int pieceType) {
    Bit* bit = new Bit();
    bool isRed = (pieceType == RED_PIECE);
    bit->LoadTextureFromFile(isRed ? "red.png" : "yellow.png");
    bit->setOwner(isRed ? getPlayerAt(0) : getPlayerAt(1));
    bit->setGameTag(pieceType);
    return bit;
}

int Connect4::aiLowestRow(const std::string& state, int col) {
    const int rows = CONNECT4_ROWS;
    for (int row = rows - 1; row >= 0; --row) {
        int index = row * CONNECT4_COLS + col;
        if (index >= 0 && index < (int)state.size() && state[index] == '0') return row;
    }
    return -1;
}

int Connect4::getLowestEmptyRowForColumn(int x) const {
    for (int y = CONNECT4_ROWS - 1; y >= 0; --y) {
        ChessSquare* sq = _grid->getSquare(x, y);
        if (sq && !sq->bit()) return y;
    }
    return -1;
}

bool Connect4::actionForEmptyHolder(BitHolder &holder) {
    ChessSquare* square = static_cast<ChessSquare*>(&holder);
    if (!square) return false;

    int col = square->getColumn();
    int row = getLowestEmptyRowForColumn(col);
    if (row < 0) return false;

    ChessSquare* dest = _grid->getSquare(col, row);
    if (!dest) return false;

    Bit* bit = createPiece(getCurrentPlayer());
    ImVec2 target = dest->getPosition();
    ImVec2 start = ImVec2(target.x, target.y - 80.0f * (row + 1));
    bit->setPosition(start);
    dest->setBit(bit);
    bit->moveTo(target);

    if (checkForWinner() || checkForDraw()) {
        endTurn();
        return true;
    }

    endTurn();
    return true;
}

bool Connect4::canBitMoveFrom(Bit &/*bit*/, BitHolder &/*src*/) { return false; }
bool Connect4::canBitMoveFromTo(Bit &/*bit*/, BitHolder &/*src*/, BitHolder &/*dst*/) { return false; }

int Connect4::countConsecutive(int x, int y, int dx, int dy, Player* owner) const {
    int count = 0;
    int cx = x + dx;
    int cy = y + dy;
    while (_grid->isValid(cx, cy)) {
        ChessSquare* sq = _grid->getSquare(cx, cy);
        if (!sq) break;
        Bit* b = sq->bit();
        if (!b || b->getOwner() != owner) break;
        ++count;
        cx += dx;
        cy += dy;
    }
    return count;
}

Player* Connect4::checkForWinner() {
    Player* winner = nullptr;
    const int dirs[4][2] = {{1,0},{0,1},{1,1},{1,-1}};

    _grid->forEachEnabledSquare([&](ChessSquare* square, int x, int y) {
        if (winner) return;
        Bit* b = square->bit();
        if (!b) return;
        Player* owner = b->getOwner();
        for (int i = 0; i < 4; ++i) {
            int dx = dirs[i][0];
            int dy = dirs[i][1];
            int total = 1 + countConsecutive(x, y, dx, dy, owner) + countConsecutive(x, y, -dx, -dy, owner);
            if (total >= 4) { winner = owner; return; }
        }
    });

    if (winner) _winner = winner;
    return winner;
}

bool Connect4::checkForDraw() {
    bool full = true;
    _grid->forEachEnabledSquare([&full](ChessSquare* square, int x, int y) {
        if (!square->bit()) full = false;
    });
    return full && !checkForWinner();
}

void Connect4::stopGame() {
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

std::string Connect4::initialStateString() {
    return std::string(CONNECT4_COLS * CONNECT4_ROWS, '0');
}

std::string Connect4::stateString() {
    std::string s;
    s.reserve(CONNECT4_COLS * CONNECT4_ROWS);
    _grid->forEachEnabledSquare([&s,this](ChessSquare* square, int x, int y) {
        Bit* bit = square->bit();
        if (!bit) s += '0';
        else s += (bit->gameTag() == RED_PIECE ? '1' : '2');
    });
    return s;
}

void Connect4::setStateString(const std::string &s) {
    if ((int)s.length() != CONNECT4_COLS * CONNECT4_ROWS) return;
    int index = 0;
    _grid->forEachEnabledSquare([&](ChessSquare* square, int x, int y) {
        if (index < (int)s.length()) {
            char c = s[index++];
            square->destroyBit();
            if (c == '1') {
                Bit* piece = createPiece(RED_PIECE);
                piece->setPosition(square->getPosition());
                square->setBit(piece);
            } else if (c == '2') {
                Bit* piece = createPiece(YELLOW_PIECE);
                piece->setPosition(square->getPosition());
                square->setBit(piece);
            }
        }
    });
}

static bool isAIBoardFull(const std::string& state) {
    return state.find('0') == std::string::npos;
}

static int evaluateAIBoard(const std::string& state) {
    const int rows = Connect4::CONNECT4_ROWS;
    const int cols = Connect4::CONNECT4_COLS;
    std::vector<int> values(rows * cols, 1);

    if (cols == 7 && rows == 6) {
        const int vals[] = {
            1,1,2,3,2,1,1,
            1,1,2,3,2,1,1,
            1,1,2,3,2,1,1,
            1,1,2,3,2,1,1,
            1,1,2,3,2,1,1,
            1,1,2,3,2,1,1
        };
        values.assign(vals, vals + rows * cols);
    } else {
        int center = cols / 2;
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
                values[r * cols + c] = std::max(1, center - abs(c - center) + 1);
    }

    int value = 0;
    for (int i = 0; i < (int)state.size(); ++i) {
        if (state[i] == '2') value += values[i];
        else if (state[i] == '1') value -= values[i];
    }
    return value;
}

int Connect4::negamax(std::string& state, int depth, int playerColor) {
    const int INF = std::numeric_limits<int>::max() / 4;
    return negamax(state, depth, playerColor, -INF, INF);
}

int Connect4::negamax(std::string& state, int depth, int playerColor, int alpha, int beta) {
    const int MAX_DEPTH = 6;
    if (depth >= MAX_DEPTH || isAIBoardFull(state))
        return evaluateAIBoard(state) * playerColor;

    int bestVal = -1000000;
    static const int order[7] = {3, 2, 4, 1, 5, 0, 6};

    for (int i = 0; i < CONNECT4_COLS; ++i) {
        int col = order[i];
        if (col >= CONNECT4_COLS) continue;
        int row = aiLowestRow(state, col);
        if (row < 0) continue;

        int idx = row * CONNECT4_COLS + col;
        state[idx] = (playerColor == 1) ? '2' : '1';

        int val = -negamax(state, depth + 1, -playerColor, -beta, -alpha);

        state[idx] = '0';

        if (val > bestVal) bestVal = val;
        if (bestVal > alpha) alpha = bestVal;
        if (alpha >= beta) break;
    }
    return bestVal;
}

void Connect4::updateAI() {
    if (!gameHasAI() || getCurrentPlayer() != getPlayerAt(AI_PLAYER)) return;

    std::string state = stateString();
    int bestCol = -1;
    int bestVal = -1000000;

    static const int order[CONNECT4_COLS] = {3, 2, 4, 1, 5, 0, 6};
    for (int i = 0; i < CONNECT4_COLS; ++i) {
        int col = order[i];
        int row = aiLowestRow(state, col);
        if (row < 0) continue;
        int idx = row * CONNECT4_COLS + col;

        state[idx] = '2';
        int val = -negamax(state, 0, -1);
        state[idx] = '0';

        if (val > bestVal) { bestVal = val; bestCol = col; }
    }

    if (bestCol >= 0) {
        BitHolder* top = _grid->getSquare(bestCol, 0);
        if (top) actionForEmptyHolder(*top);
    } else {
        endTurn();
    }
}
