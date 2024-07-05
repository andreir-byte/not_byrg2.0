#include "globalshit.h"
#include <vector>
#include <set>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <limits>

/* Board notation:
    0 - empty cell
    1 - my blocks
    2 - opponent blocks
    3 - illegal cell
    4 - corner cell
    -1 - out of bounds
*/

const int SIMULATION_COUNT = 1000;
const double EXPLORATION_PARAMETER = std::sqrt(2.0);

void printMatrix(int a[][GRID_SIZE+5]){
    for(int i=1; i<=GRID_SIZE; ++i){
        std::cerr << 15-i << "  ";
        if(i>=6)
            std::cerr << " ";
        for(int j=1; j<=GRID_SIZE; ++j){
            switch(a[i][j]) {
                case 0:
                    std::cerr << "\033[0m" << "." << " ";
                    break;
                case 1:
                    std::cerr << "\033[34m" << "P" << " ";
                    break;
                case 2:
                    std::cerr << "\033[31m" << "O" << " ";
                    break;
                case 4:
                    std::cerr << "\033[32m" << "C" << " ";
                    break;
                case 3:
                    std::cerr << "\033[33m" << "I" << " ";
                    break;
            }
        }
        std::cerr<< "\033[0m\n";
    }
    std::cerr << "\033[0m";
    std::cerr << "    ";
    for(int i=0; i<GRID_SIZE; ++i){
        std::cerr << (char)('a'+i) << " ";
    }
    std::cerr << "\n-----------------\n";
}

class BOARD{
public:
    int board[GRID_SIZE+5][GRID_SIZE+5];
    POINT corners[GRID_SIZE*GRID_SIZE+5] = {0};
    int lastMoveCornerCount = 0, lastMoveIllegalCount = 0;
    POINT illegal[GRID_SIZE*GRID_SIZE+5] = {0};

    void clearBoard() {
        for(int i = 1; i <= GRID_SIZE; ++i){
            for(int j = 1; j <= GRID_SIZE; ++j){
                board[i][j] = 0;
            }
        }
        corners[0].x = 0;
        illegal[0].x = 0;
        board[5][5] = 4;
        corners[0].x++;
        corners[1].x = 5;
        corners[1].y = 5;
    }

    bool validMove(POINT piece[], int xo, int yo) {
        for(int i = 1; i <= piece[0].x; ++i){
            int x = piece[i].x + xo;
            int y = piece[i].y + yo;
            if(!(x > 0 && x <= GRID_SIZE && y > 0 && y <= GRID_SIZE) || 
               board[x][y] == 3 || board[x][y] == 1 || board[x][y] == 2 || board[x][y] == -1)
                return false;
        }
        return true;
    }

    void setCorners(POINT piece[], int x0, int y0) {
        lastMoveCornerCount = 0;
        int dx[4] = {-1, 1, 1, -1};
        int dy[4] = {1, 1, -1, -1};
        for(int i = 1; i <= piece[0].x; ++i){
            for(int k = 0; k < 4; ++k){
                int x = piece[i].x + dx[k] + x0;
                int y = piece[i].y + dy[k] + y0;
                if(board[x][y] == 0){
                    board[x][y] = 4;
                    corners[0].x++;
                    corners[corners[0].x] = POINT{x, y};
                    lastMoveCornerCount++;
                }
            }
        }
    }

    void unSetCorners() {
        for(int i = corners[0].x - lastMoveCornerCount; i <= corners[0].x; ++i){
            board[corners[i].x][corners[i].y] = 0;
        }
        corners[0].x -= lastMoveCornerCount;
    }

    void markIllegal(POINT piece[], int x0, int y0) {
        lastMoveIllegalCount = 0;
        illegal[0].x = 0;
        int dx[4] = {-1, 0, 1, 0};
        int dy[4] = {0, 1, 0, -1};
        for(int i = 1; i <= piece[0].x; ++i){
            for(int k = 0; k < 4; ++k){
                int x = piece[i].x + dx[k] + x0;
                int y = piece[i].y + dy[k] + y0;
                if(board[x][y] == 0){
                    board[x][y] = 3;
                    illegal[0].x++;
                    illegal[illegal[0].x] = POINT{x, y};
                    lastMoveIllegalCount++;
                }
            }
        }
    }

    void unMarkIllegal() {
        for(int i = illegal[0].x - lastMoveIllegalCount; i <= illegal[0].x; ++i){
            board[illegal[i].x][illegal[i].y] = 0;
        }
    }

    void makeMove(POINT piece[], int x, int y, int player) {   
        for(int i = 1; i <= piece[0].x; ++i){
            board[x + piece[i].x][y + piece[i].y] = player;
        }
        if(player == 1){
            markIllegal(piece, x, y);
            setCorners(piece, x, y);
        }
    }

    void undoMove(POINT piece[], int x, int y) {
        unMarkIllegal();
        unSetCorners();
        for(int i = 1; i <= piece[0].x; ++i){
            board[x + piece[i].x][y + piece[i].y] = 0;
        }
        corners[0].x -= lastMoveCornerCount;
        illegal[0].x -= lastMoveIllegalCount;
    }
};


const int MAX_ROTATIONS = 91;

std::vector<Piece> basic_pieces = {
    {"*..", "*..", "***"},
    {"*.", "*.", "*.", "**"},
    {"**.", ".*.", ".**"},
    {".*", ".*", "**", "*."},
    {"*..", "**.", ".**"},
    {".*.", "***", ".*."},
    {".**", "**.", ".*."},
    {"*", "*", "*", "*", "*"},
    {"***", ".*.", ".*."},
    {".*", "**", ".*", ".*"},
    {"**", "**", "*."},
    {"*.*", "***"},
    {"*.", "*.", "**"},
    {"*", "*", "*", "*"},
    {"***", ".*."},
    {"**.", ".**"},
    {"**", "**"},
    {"*.", "**"},
    {"*", "*", "*"},
    {"**"},
    {"*"}
};

Coordinates toCoord(Piece p) {
    Coordinates coords;
    for(int r = 0; r < p.size(); ++r){
        for(int f = 0; f < p[0].size(); ++f){
            if(p[r][f] == '*')
                coords.insert({r, f});
        }
    }
    return coords;
}

Coordinates rotate90(Coordinates pieces) {
    Coordinates rotated;
    for(const auto& [x, y] : pieces){
        rotated.insert({y, -x});
    }
    return rotated;
}

Coordinates mirror(Coordinates pieces) {
    Coordinates mirrored;
    for(const auto& [x, y] : pieces){
        mirrored.insert({x, -y});
    }
    return mirrored;
}

Coordinates normalize(Coordinates piece) {
    Coordinates newPiece;
    int minX = INFINITY;
    for(const auto& [x, y] : piece){
        if(x < minX)
            minX = x;
    }
    int minY = INFINITY;
    for(const auto& [x, y] : piece){
        if(y < minY)
            minY = y;
    }
    for(const auto& [x, y] : piece){
        newPiece.insert({x - minX, y - minY});
    }
    return newPiece;
}

std::set<Coordinates> generate_rotations(std::vector<Piece> pieces) {
    std::set<Coordinates> rotations;
    for(const auto& p : pieces){
        Coordinates tmp = toCoord(p);
        for(int i = 0; i < 4; ++i){
            rotations.insert(normalize(tmp));
            rotations.insert(normalize(mirror(tmp)));
            tmp = rotate90(tmp);
        }
    }
    return rotations;
}

bool withinBounds(POINT piece[]) {
    for(int i = 1; i <= piece[0].x; ++i){
        int x = piece[i].x;
        int y = piece[i].y;
        if(!(x >= 0 && x <= 14 && y >= 0 && y <= 14))
            return false;
    }
    return true;
}

void rotatePiece90(POINT p[]) {
    for(int i = 1; i <= p[0].x; ++i){
        int x = p[i].x;
        int y = p[i].y;
        p[i].x = y;
        p[i].y = -x;
    }
}

void mirrorPiece(POINT p[]) {
    for(int i = 1; i <= p[0].x; ++i){
        int x = p[i].x;
        int y = p[i].y;
        p[i].x = x;
        p[i].y = -y;
    }
}

void fromPointToCoordinates(POINT p[], Coordinates& c) {
    for(int i = 1; i <= p[0].x; ++i){
        c.insert({p[i].x, p[i].y});
    }
}

int whichShape(POINT piece[]) {
    Coordinates c;
    fromPointToCoordinates(piece, c);
    for(int i = 0; i < 4; ++i){
        int j = 0;
        for(const auto& p : basic_pieces){
            if(toCoord(p) == c)
                return j;
            c = normalize(mirror(c));
            if(toCoord(p) == c)
                return j;
            c = normalize(mirror(c));
            j++;
        }
        c = normalize(rotate90(c));
    }
    return -1;
}

std::set<Coordinates> whatEver;

POINT rotations[MAX_ROTATIONS + 1][MAX_PIECE_SIZE + 1];
int a[GRID_SIZE + 1][GRID_SIZE + 1][MAX_ROTATIONS + 1];
bool inHandMe[NUM_PIECE + 1], inHandOpp[NUM_PIECE + 1];

void fromAtoPiece(int id, POINT p[]) {
    for(int i = 0; i <= rotations[id][0].x; ++i)
        p[i] = rotations[id][i];
}

std::string trim(std::string s) {
    int l = 0, r = s.size() - 1;
    while(l < r && s[l] == ' ')
        l++;
    while(l < r && s[r] == ' ')
        r--;
    return s.substr(l, r - l + 1);
}

void fromBlokus(std::string s, POINT p[]) {
    s = trim(s);
    std::vector<std::string> words;
    std::istringstream stream(s);
    std::string word;
    while(std::getline(stream, word, ',')) {
        words.push_back(word);
    }
    p[0].x = 0;
    for(const auto& w : words){
        int x, y;
        y = w[0] - 'a' + 1;
        x = w[1] - '0';
        if(w.size() == 3){
            x *= 10;
            x += w[2] - '0';
        }
        p[0].x++;
        p[p[0].x].x = 15 - x;
        p[p[0].x].y = y;
    }
}

std::string toBlokus(POINT p[], int x0, int y0) {
    std::string result;
    for(int i = 1; i <= p[0].x; ++i){
        p[i].x = 15 - p[i].x - x0;
        p[i].y += y0;
        char y = p[i].y + 'a' - 1, x;
        result += y;
        if(p[i].x >= 10){
            x = '0' + p[i].x / 10;
            result += x;
            x = '0' + p[i].x % 10;
            result += x;
        }
        else{
            x = '0' + p[i].x;
            result += x;
        }
        result += ",";
    }
    result = result.substr(0, result.size() - 1);
    return result;
}

class Node {
public:
    BOARD state;
    Node* parent;
    std::vector<Node*> children;
    int visitCount;
    double winCount;
    POINT move;
    int player;

    Node(BOARD state, Node* parent = nullptr, POINT move = {0, 0}, int player = 1)
        : state(state), parent(parent), move(move), player(player), visitCount(0), winCount(0) {}

    bool isFullyExpanded() {
        return !children.empty();
    }

    bool isTerminal() {
        // Check if there are no valid moves left for the player
        for(int index = 1; index <= state.corners[0].x; ++index){
            int cornerX = state.corners[index].x, cornerY = state.corners[index].y;
            for(int j = 1; j <= a[cornerX][cornerY][0]; ++j){
                POINT p[MAX_PIECE_SIZE+1];
                fromAtoPiece(a[cornerX][cornerY][j], p);
                if(state.validMove(p, cornerX, cornerY) && (player == 1 ? inHandMe[whichShape(p)] : inHandOpp[whichShape(p)])){
                    return false;
                }
            }
        }
        return true;
    }

    Node* expand() {
        // Generate all valid moves and create child nodes
        for(int index = 1; index <= state.corners[0].x; ++index){
            int cornerX = state.corners[index].x, cornerY = state.corners[index].y;
            for(int j = 1; j <= a[cornerX][cornerY][0]; ++j){
                POINT p[MAX_PIECE_SIZE+1];
                fromAtoPiece(a[cornerX][cornerY][j], p);
                int shape = whichShape(p);
                if(state.validMove(p, cornerX, cornerY) && (player == 1 ? inHandMe[shape] : inHandOpp[shape])){
                    BOARD newState = state;
                    newState.makeMove(p, cornerX, cornerY, player);
                    children.push_back(new Node(newState, this, POINT{cornerX, cornerY}, 3 - player));
                }
            }
        }
        return children.empty() ? nullptr : children.back();
    }
};

double UCTValue(int totalVisit, double nodeWinCount, int nodeVisitCount) {
    if(nodeVisitCount == 0) {
        return std::numeric_limits<double>::max();
    }
    return (nodeWinCount / nodeVisitCount) + EXPLORATION_PARAMETER * std::sqrt(std::log(totalVisit) / nodeVisitCount);
}

Node* SelectBestNodeWithUCT(Node* node) {
    int parentVisitCount = node->visitCount;
    return *std::max_element(node->children.begin(), node->children.end(), [parentVisitCount](Node* a, Node* b) {
        return UCTValue(parentVisitCount, a->winCount, a->visitCount) < UCTValue(parentVisitCount, b->winCount, b->visitCount);
    });
}

double SimulateRandomPlayout(Node* node) {
    BOARD currentState = node->state;
    int currentPlayer = node->player;
    while(true){
        bool hasValidMove = false;
        for(int index = 1; index <= currentState.corners[0].x; ++index){
            int cornerX = currentState.corners[index].x, cornerY = currentState.corners[index].y;
            for(int j = 1; j <= a[cornerX][cornerY][0]; ++j){
                POINT p[MAX_PIECE_SIZE+1];
                fromAtoPiece(a[cornerX][cornerY][j], p);
                int shape = whichShape(p);
                if(currentState.validMove(p, cornerX, cornerY) && (currentPlayer == 1 ? inHandMe[shape] : inHandOpp[shape])){
                    currentState.makeMove(p, cornerX, cornerY, currentPlayer);
                    currentPlayer = 3 - currentPlayer;
                    hasValidMove = true;
                    break;
                }
            }
            if(hasValidMove) break;
        }
        if(!hasValidMove) break;
    }
    return currentPlayer == 1 ? -1.0 : 1.0; // Assume win for the other player if no valid moves left
}

void Backpropagate(Node* node, double result) {
    while(node != nullptr) {
        node->visitCount++;
        node->winCount += result;
        node = node->parent;
    }
}

Node* MCTS(BOARD board) {
    Node* root = new Node(board);
    for(int i = 0; i < SIMULATION_COUNT; ++i) {
        Node* promisingNode = root;

        // Selection
        while(promisingNode->isFullyExpanded() && !promisingNode->isTerminal()) {
            promisingNode = SelectBestNodeWithUCT(promisingNode);
        }

        // Expansion
        if(!promisingNode->isFullyExpanded()) {
            promisingNode = promisingNode->expand();
        }

        // Simulation
        double playoutResult = SimulateRandomPlayout(promisingNode);

        // Backpropagation
        Backpropagate(promisingNode, playoutResult);
    }

    return SelectBestNodeWithUCT(root); // Return the best move
}

int main() {
    whatEver = generate_rotations(basic_pieces);

    int i = 0;
    for(const auto& rotated : whatEver){
        int j = 1;
        rotations[i][0].x = rotated.size();
        for(const auto& rot : rotated){
            rotations[i][j].x = rot.first;
            rotations[i][j].y = rot.second;
            j++;
        }
        i++;
    }
    for(i = 0; i < GRID_SIZE; ++i){
        for(int j = 0; j < GRID_SIZE; ++j){
            for(int k = 0; k < MAX_ROTATIONS; ++k){
                POINT b[MAX_PIECE_SIZE+1];
                for(int l = 1; l <= rotations[k][0].x; ++l){
                    b[l].x = i + rotations[k][l].x;
                    b[l].y = j + rotations[k][l].y;
                }
                b[0] = rotations[k][0];
                if(withinBounds(b)){
                    int tmp = a[i][j][0];
                    a[i][j][0]++;
                    a[i][j][tmp] = k;
                }
            }
        }
    }
    int sum = 0;
    for(i = 0; i < GRID_SIZE; ++i){
        for(int j = 0; j < GRID_SIZE; ++j){
            sum += a[i][j][0];
        }
    }

    BOARD board;
    board.clearBoard();
    POINT* piece = new POINT[MAX_PIECE_SIZE+1];
    int id = a[5][5][1];
    for(i = 0; i <= rotations[id][0].x; ++i){
        piece[i].x = rotations[id][i].x;
        piece[i].y = rotations[id][i].y;
    }

    for(int x = 0; x <= GRID_SIZE + 1; ++x){
        board.board[0][x] = -1;
        board.board[GRID_SIZE + 1][x] = -1;
        board.board[x][0] = -1;
        board.board[x][GRID_SIZE + 1] = -1;
    }
    for(int x = 0; x <= NUM_PIECE + 1; ++x){
        inHandMe[x] = 1;
        inHandOpp[x] = 1;
    }

    int state = T_UNKNOWN;
    while(state != T_QUIT) {
        std::string s, player;
        std::cin >> s;
        if(s == "set_game"){
            state = T_SET_GAME;
            std::cin >> s;
            std::cin >> s;
            if(s == "Duo") {
                std::cout << "= \n\n";
            } else {
                std::cout << "Unknown game type:" << s << " try the game mode Duo\n\n";
                return 0;
            }
        } else if(s == "clear_board"){
            state = T_CLEAR_BOARD;
            board.clearBoard();
            printMatrix(board.board);
            std::cout << "= \n\n";
        } else if(s == "cputime"){
            state = T_CPUTIME;
            std::cout << "= 0\n\n";
        } else if(s == "genmove"){
            state = T_GENMOVE;
            Node* bestNode = MCTS(board);
            POINT p[MAX_PIECE_SIZE+1];
            fromAtoPiece(bestNode->move.x, p);
            board.makeMove(p, bestNode->move.x, bestNode->move.y, 1);
            std::string moveStr = toBlokus(p, bestNode->move.x, bestNode->move.y);
            printMatrix(board.board);
            std::cout << "= " << moveStr << "\n\n";
        } else if(s == "play"){
            state = T_PLAY;
            std::cin >> s;
            std::getline(std::cin, s);
            s = trim(s);
            std::cerr << "\n";
            POINT p[MAX_PIECE_SIZE+1];
            fromBlokus(s, p);
            board.makeMove(p, 0, 0, 2);
            printMatrix(board.board);
            std::cout << "= \n\n";
        } else if(s == "quit"){
            std::cout << "= \n\n";
            state = T_QUIT;
        }
    }
    return 0;
}
