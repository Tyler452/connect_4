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

    if (_gameOptions.AIPlaying) {
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
    for (int y = 0; y < CONNECT4_ROWS; ++y) {
        for (int x = 0; x < CONNECT4_COLS; ++x) {
            if (winner) break;
            ChessSquare* square = _grid->getSquare(x, y);
            if (!square) continue;
            Bit* b = square->bit();
            if (!b) continue;
            Player* owner = b->getOwner();
            for (int i = 0; i < 4; ++i) {
                int dx = dirs[i][0];
                int dy = dirs[i][1];
                int total = 1 + countConsecutive(x, y, dx, dy, owner) + countConsecutive(x, y, -dx, -dy, owner);
                if (total >= 4) { winner = owner; break; }
            }
        }
    }
    if (winner) _winner = winner;
    return winner;
}

bool Connect4::checkForDraw() {
    for (int y = 0; y < CONNECT4_ROWS; ++y) {
        for (int x = 0; x < CONNECT4_COLS; ++x) {
            ChessSquare* sq = _grid->getSquare(x, y);
            if (!sq) return false;
            if (!sq->bit()) return false;
        }
    }
    return !checkForWinner();
}

void Connect4::stopGame() {
    for (int y = 0; y < CONNECT4_ROWS; ++y) {
        for (int x = 0; x < CONNECT4_COLS; ++x) {
            ChessSquare* sq = _grid->getSquare(x, y);
            if (sq) sq->destroyBit();
        }
    }
}

std::string Connect4::initialStateString() {
    return std::string(CONNECT4_COLS * CONNECT4_ROWS, '0');
}

std::string Connect4::stateString() {
    std::string s;
    s.reserve(CONNECT4_COLS * CONNECT4_ROWS);
    for (int y = 0; y < CONNECT4_ROWS; ++y) {
        for (int x = 0; x < CONNECT4_COLS; ++x) {
            ChessSquare* sq = _grid->getSquare(x, y);
            if (!sq) { s += '0'; continue; }
            Bit* b = sq->bit();
            if (!b) s += '0';
            else s += (b->gameTag() == RED_PIECE ? '1' : '2');
        }
    }
    return s;
}

void Connect4::setStateString(const std::string &s) {
    if ((int)s.length() != CONNECT4_COLS * CONNECT4_ROWS) return;
    int index = 0;
    for (int y = 0; y < CONNECT4_ROWS; ++y) {
        for (int x = 0; x < CONNECT4_COLS; ++x) {
            if (index >= (int)s.length()) break;
            char c = s[index++];
            ChessSquare* square = _grid->getSquare(x, y);
            if (!square) continue;
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
    }
}
/*This Ai uses the windowed method that the professor mentioned in class. 
Although it took a while by looking around to try and try to understand it how it would work.
it also uses the alpha-beta method and should fully work.
*/
static bool isAIBoardFull(const std::string& state) {
    return state.find('0') == std::string::npos;
}

static int countDirection(const std::string &state, int rows, int cols, int r, int c, int dr, int dc, char player) {
    int count = 0;
    r += dr; c += dc;
    while (r >= 0 && r < rows && c >= 0 && c < cols) {
        int idx = r * cols + c;
        if (state[idx] != player) break;
        ++count;
        r += dr; c += dc;
    }
    return count;
}

static bool isWinningMove(std::string state, int cols, int rows, int col, char player) {
    int row = -1;
    for (int r = rows - 1; r >= 0; --r) {
        int idx = r * cols + col;
        if (state[idx] == '0') { row = r; break; }
    }
    if (row < 0) return false;
    int idx = row * cols + col;
    state[idx] = player;
    const int dirs[4][2] = {{1,0},{0,1},{1,1},{1,-1}};
    for (int i = 0; i < 4; ++i) {
        int dx = dirs[i][0];
        int dy = dirs[i][1];
        int total = 1 + countDirection(state, rows, cols, row, col, 0 + dy, 0 + dx, player) + countDirection(state, rows, cols, row, col, 0 - dy, 0 - dx, player);
        if (total >= 4) return true;
    }
    return false;
}

static int evaluateAIBoard(const std::string& state) {
    const int rows = Connect4::CONNECT4_ROWS;
    const int cols = Connect4::CONNECT4_COLS;
    const int WIN_SCORE = 100000;
    const int THREE_SCORE = 100;
    const int TWO_SCORE = 10;

    int score = 0;
    auto scoreWindow = [&](int a, int b, int c, int d) {
        int ai = 0, hu = 0;
        if (state[a] == '2') ++ai; else if (state[a] == '1') ++hu;
        if (state[b] == '2') ++ai; else if (state[b] == '1') ++hu;
        if (state[c] == '2') ++ai; else if (state[c] == '1') ++hu;
        if (state[d] == '2') ++ai; else if (state[d] == '1') ++hu;
        if (ai > 0 && hu > 0) return 0;
        if (ai == 4) return WIN_SCORE;
        if (hu == 4) return -WIN_SCORE;
        if (ai == 3 && hu == 0) return THREE_SCORE;
        if (ai == 2 && hu == 0) return TWO_SCORE;
        if (hu == 3 && ai == 0) return -THREE_SCORE;
        if (hu == 2 && ai == 0) return -TWO_SCORE;
        return 0;
    };

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols - 3; ++c) {
            int a = r * cols + c;
            int b = r * cols + c + 1;
            int c2 = r * cols + c + 2;
            int d = r * cols + c + 3;
            score += scoreWindow(a,b,c2,d);
        }
    }

    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows - 3; ++r) {
            int a = r * cols + c;
            int b = (r + 1) * cols + c;
            int c2 = (r + 2) * cols + c;
            int d = (r + 3) * cols + c;
            score += scoreWindow(a,b,c2,d);
        }
    }

    for (int r = 0; r < rows - 3; ++r) {
        for (int c = 0; c < cols - 3; ++c) {
            int a = r * cols + c;
            int b = (r + 1) * cols + c + 1;
            int c2 = (r + 2) * cols + c + 2;
            int d = (r + 3) * cols + c + 3;
            score += scoreWindow(a,b,c2,d);
        }
    }

    for (int r = 3; r < rows; ++r) {
        for (int c = 0; c < cols - 3; ++c) {
            int a = r * cols + c;
            int b = (r - 1) * cols + c + 1;
            int c2 = (r - 2) * cols + c + 2;
            int d = (r - 3) * cols + c + 3;
            score += scoreWindow(a,b,c2,d);
        }
    }

    int centerCol = cols / 2;
    for (int r = 0; r < rows; ++r) {
        int idx = r * cols + centerCol;
        if (state[idx] == '2') score += 3;
        else if (state[idx] == '1') score -= 3;
    }

    return score;
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
    const int rows = CONNECT4_ROWS;
    const int cols = CONNECT4_COLS;

    std::vector<std::pair<int,int>> moves;
    for (int col = 0; col < cols; ++col) {
        int row = aiLowestRow(state, col);
        if (row < 0) continue;
        int idx = row * cols + col;
        char playerChar = (playerColor == 1) ? '2' : '1';
        state[idx] = playerChar;
        int h = evaluateAIBoard(state) * playerColor;
        state[idx] = '0';
        moves.emplace_back(h, col);
    }

    std::sort(moves.begin(), moves.end(), [](const std::pair<int,int>& a, const std::pair<int,int>& b){ return a.first > b.first; });

    for (auto &p : moves) {
        int col = p.second;
        int row = aiLowestRow(state, col);
        if (row < 0) continue;
        int idx = row * cols + col;
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
        if (aiLowestRow(state, col) < 0) continue;
        if (isWinningMove(state, CONNECT4_COLS, CONNECT4_ROWS, col, '2')) {
            BitHolder* top = _grid->getSquare(col, 0);
            if (top) { actionForEmptyHolder(*top); return; }
        }
    }

    for (int i = 0; i < CONNECT4_COLS; ++i) {
        int col = order[i];
        if (aiLowestRow(state, col) < 0) continue;
        if (isWinningMove(state, CONNECT4_COLS, CONNECT4_ROWS, col, '1')) {
            BitHolder* top = _grid->getSquare(col, 0);
            if (top) { actionForEmptyHolder(*top); return; }
        }
    }

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
