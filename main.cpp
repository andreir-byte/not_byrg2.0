#include "globalshit.h"
#include <vector>
#include <set>
#include <algorithm>
#include <sstream>
#include <limits>

/*
    Board notation:
    0 - empty cell
    1 - my blocks
    2 - opponent blocks
    3 - illegal cell
    4 - corner cell
    -1 - out of bounds
*/
bool inHand[3][NUM_PIECE+1];

void copyMove(POINT dest[], POINT src[]){
    for(int i=0; i<=src[0].x; ++i){
        dest[i]=src[i];
    }
}

void printMatrix(int a[][GRID_SIZE+5]){
    for(int i=1; i<=GRID_SIZE; ++i){
        std::cerr << 15-i << "  ";
        if(i>=6)
            std::cerr << " ";
        for(int j=1; j<=GRID_SIZE; ++j){
            switch(a[i][j]) {
                case 0: // Empty space
                    std::cerr << "\033[0m" << "." << " "; // Default color for empty
                    break;
                case 1: // Player's blocks
                    std::cerr << "\033[34m" << "P" << " "; // Blue for player
                    break;
                case 2: // Opponent's blocks
                    std::cerr << "\033[31m" << "O" << " "; // Red for opponent
                    break;
                case 4: // Corners
                    std::cerr << "\033[32m" << "C" << " "; // Green for corners
                    break;
                case 3:
                    std::cerr << "\033[33m" << "I" << " "; // Yellow for illegal
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

const int INFINITY=1e9;
const int MAX_ROTATIONS=91;




std::vector<Piece> basic_pieces={
    {"*..", "*..", "***"},
    {"*.", "*.", "*.", "**"},
    {"**.", ".*.", ".**"},
    {".*", ".*", "**", "*."},
    {"*..", "**.", ".**"},
    {".*.", "***", ".*."},
    {".**",
     "**",
     ".*."},
    {"*", "*", "*", "*", "*"},
    {"***", ".*.", ".*."},
    {".*", "**", ".*", ".*"},
    {"**", "**", "*."},
    {"*.*", "***"},
    {"*.", "*", "**"},
    {"*", "*", "*", "*"},
    {"***", ".*."},
    {"**.", ".**"},
    {"**", "**"},
    {"*.", "**"},
    {"*", "*", "*"},
    {"**"},
    {"*"}
};

Coordinates toCoord(Piece p)
{
    Coordinates coords;
    
    for(int r=0; r < p.size(); ++r){
        for(int f=0; f < p[0].size(); ++f){
            if(p[r][f]=='*')
                coords.insert({r,f});
        }
    }
    return coords;
}

Coordinates rotate90(Coordinates pieces){
    Coordinates rotated;
    for(const auto& [x,y] : pieces){
        rotated.insert({y,-x});
    }
    return rotated;
}

Coordinates mirror(Coordinates pieces){
    Coordinates mirrored;
    for(const auto& [x,y] : pieces){
        mirrored.insert({x,-y});
    }
    return mirrored;
}

Coordinates normalize(Coordinates piece){
    Coordinates newPiece;
    int minX=INFINITY;
    for(const auto& [x,y] : piece){
        if(x<minX)
            minX=x;
    }
    int minY=INFINITY;
    for(const auto& [x,y] : piece){
        if(y<minY)
            minY=y;
    }
    for(const auto& [x,y] : piece){
        newPiece.insert({x-minX,y-minY});
    }
    return newPiece;
}

std::set<Coordinates> generate_rotations(std::vector<Piece> pieces){
    std::set<Coordinates> rotations;
    int i=0, k=0;
    for(const auto& p : pieces){
        Coordinates tmp;
        tmp=toCoord(p);
        for(int i=0; i<4; ++i){
            rotations.insert(normalize(tmp));
            rotations.insert(normalize(mirror(tmp)));
            tmp=rotate90(tmp);
        }
        i++;
    }
    return rotations;
}
bool withinBounds(POINT piece[]){
    for(int i=1; i<=piece[0].x; ++i){
        int x=piece[i].x;
        int y=piece[i].y;
        if(!(x>=0 && x<=14 && y>=0 && y<=14))
            return 0;
    }
    return 1;
}
void rotatePiece90(POINT p[]){
    for(int i=1; i<=p[0].x; ++i){
        int x=p[i].x;
        int y=p[i].y;
        p[i].x=y;
        p[i].y=-x;
    }
}
void mirrorPiece(POINT p[]){
    for(int i=1; i<=p[0].x; ++i){
        int x=p[i].x;
        int y=p[i].y;
        p[i].x=x;
        p[i].y=-y;
    }
}
std::set<Coordinates> whatEver;

POINT rotations[MAX_ROTATIONS+1][MAX_PIECE_SIZE+1];
int a[GRID_SIZE+1][GRID_SIZE+1][MAX_ROTATIONS*MAX_PIECE_SIZE+1];

void fromAtoPiece(int id, POINT p[]){
    for(int i=0; i<=rotations[id][0].x; ++i)
        p[i]=rotations[id][i];
}
std::string trim(std::string s){
    int l=0, r=s.size()-1;
    while(l<r && s[l]==' ')
        l++;
    while(l<r && s[r]==' ')
        r--;
    return s.substr(l, r-l+1);
}
void fromBlokus(std::string s, POINT p[]){
    s=trim(s);
    std::vector<std::string> words;
    std::istringstream stream(s);
    std::string word;
    while (std::getline(stream, word, ',')) {
        words.push_back(word);
    }
    p[0].x=0;
    for(const auto& w : words){
        
          int x, y;
          y=w[0]-'a'+1;
          x=w[1]-'0';
          if(w.size()==3){
              x*=10;
              x+=w[2]-'0';
          }
          p[0].x++;
          p[p[0].x].x=15-x;
          p[p[0].x].y=y;
    }
}
std::string toBlokus(POINT p[], int x0, int y0) {
    std::string result;
    for (int i = 1; i <= p[0].x; ++i) {
        p[i].x=15-p[i].x-x0;
        p[i].y+=y0;
        char y = p[i].y + 'a' - 1, x;
        result += y;
        if(p[i].x >= 10){
            x = '0' + p[i].x/10;
            result += x;
            x = '0' + p[i].x%10;
            result += x;
        }
        else{
            x = '0' + p[i].x;
            result += x;
        }
        result += ",";
    }
    result=result.substr(0, result.size()-1);
    return result;
}


void fromPointToCoordinates(POINT p[], Coordinates& c){
    for(int i=1; i<=p[0].x; ++i){
        c.insert({p[i].x, p[i].y});
    }
}
int whichShape(POINT piece[]){
    Coordinates c;
    fromPointToCoordinates(piece, c);
    for(int i=0; i<4; ++i){
        int j=0;
        for(const auto& p : basic_pieces){
            if(toCoord(p)==c)
                return j;
            c=normalize(mirror(c));
            if(toCoord(p)==c)
                return j;
            c=normalize(mirror(c));
            j++;
        }
        c=normalize(rotate90(c));
    }
    return -1;
}

class BOARD{
    public:
        int board[GRID_SIZE+5][GRID_SIZE+5];
        POINT corners[GRID_SIZE*GRID_SIZE+5]={0};
        int cornerIndex[3][GRID_SIZE*GRID_SIZE+5]={0};
        int illegalIndex[3][GRID_SIZE*GRID_SIZE+5]={0};
        POINT illegal[GRID_SIZE*GRID_SIZE+5]={0};
        bool gameOver[2]={0,0};
        void clearBoard(){
            for(int i=1; i<=GRID_SIZE; ++i){
                for(int j=1; j<=GRID_SIZE; ++j){
                    board[i][j]=0;
                }
            }
            corners[0].x=0;
            illegal[0].x=0;
            board[5][5]=4;
            corners[0].x++;
            corners[1].x=5;
            corners[1].y=5;
        }
        bool validMove(POINT piece[], int xo, int yo){
            for(int i=1; i<=piece[0].x; ++i){
                int x=piece[i].x + xo;
                int y=piece[i].y + yo;
                if(!(x>0 && x<=GRID_SIZE && y>0 && y<=GRID_SIZE) || board[x][y]==3 || board[x][y]==1 || board[x][y]==2 || board[x][y]==-1)
                    return 0;
            }
            return 1;
        }
        void setCorners(POINT piece[], int x0, int y0, int player){
            int dx[4]={-1, 1, 1, -1};
            int dy[4]={1, 1,-1, -1};
            /*
            printMatrix(board);
            for(int i=1; i<=illegal[0].x; ++i){
                std::cerr << illegal[i].x << " " << illegal[i].y << '\n';
            }
            */
            cornerIndex[player][0]++;
            bool ok=1;
            for(int i=1; i<=piece[0].x; ++i){
                for(int k=0; k<4; ++k){
                    int x=piece[i].x+dx[k]+x0;
                    int y=piece[i].y+dy[k]+y0;
                    if(board[x][y]==0){
                        board[x][y]=4;
                        corners[0].x++;
                        corners[corners[0].x]=POINT{x, y};
                        if(ok){
                            cornerIndex[player][cornerIndex[player][0]]=corners[0].x;
                            ok=0;
                        }
                    }
                }
            }
            cornerIndex[player][0]-=ok;
        }
        void unSetCoreners(int player){
            for(int i=cornerIndex[player][cornerIndex[player][0]]; i<=corners[0].x; ++i){
                board[corners[i].x][corners[i].y]=0;
                //printMatrix(board);
            }
            if(cornerIndex[player][0]!=corners[0].x){
                corners[0].x=cornerIndex[player][cornerIndex[player][0]]-1;
                cornerIndex[player][0]--;
            }
                
            
        }
        void markIllegal(POINT piece[], int x0, int y0, int player){
            int dx[4]={-1, 0, 1, 0};
            int dy[4]={0, 1, 0, -1};
            illegalIndex[player][0]++;
            int ok=1;
            for(int i=1; i<=piece[0].x; ++i){
                for(int k=0; k<4; ++k){
                    int x=piece[i].x+dx[k]+x0;
                    int y=piece[i].y+dy[k]+y0;
                    if(board[x][y]==0 || board[x][y]==4){
                        board[x][y]=3;
                        illegal[0].x++;
                        illegal[illegal[0].x]=POINT{x, y};
                        if(ok){
                            illegalIndex[player][illegalIndex[player][0]]=illegal[0].x;
                            ok=0;
                        }
                    }
                }
            }
            illegalIndex[player][0]-=ok;
        }
        void unMarkIllegal(int player){
            for(int i=illegalIndex[player][illegalIndex[player][0]]; i<=illegal[0].x; ++i){
                board[illegal[i].x][illegal[i].y]=0;
                //printMatrix(board);
            }
            if(illegalIndex[player][0]!=illegal[0].x){
                illegal[0].x=illegalIndex[player][0]-1;
                illegalIndex[player][0]--;
                
            }
                
        }
        void makeMove(POINT piece[], int x, int y, int player, int& protocol){ 
            protocol=0;
            for(int i=1; i<=piece[0].x; ++i){
                if(board[x+piece[i].x][y+piece[i].y]==4)
                    protocol=1;
                board[x+piece[i].x][y+piece[i].y]=player;
            }
            if(player==1){
                markIllegal(piece, x, y, player);
                setCorners(piece, x, y, player);
            }
            if(protocol==0 && player==1){
                undoMove(piece,x,y,player);
            }
            //printMatrix(board);
        }
        void undoMove(POINT piece[], int x, int y, int player){
            if(player==1){
                unMarkIllegal(player);
                unSetCoreners(player);
            }
            for(int i=1; i<=piece[0].x; ++i){
                board[piece[i].x+x][piece[i].y+y]=0;
            }
            for(int i=1; i<=corners[0].x; ++i){
                if(board[corners[i].x][corners[i].y]==0)
                    board[corners[i].x][corners[i].y]=4;
                if(board[corners[i].x][corners[i].y]==3)
                    corners[i]={0,0};
                //std::cerr << corners[i].x << " " << corners[i].y << '\n';
            }
            int shape=whichShape(piece);
            inHand[player][shape]=1;
            //printMatrix(board);
        }
        int evaluateBoard(){
            int score=0;
            for(int i=1; i<=GRID_SIZE; ++i){
                for(int j=1; j<=GRID_SIZE; ++j){
                    if(board[i][j]==1)
                        score++;
                    else if(board[i][j]==2)
                        score--;
                }
            }
            return score;
        
        }
        std::vector<std::vector<POINT>> generateAllMoves(int player){
            std::vector<std::vector<POINT>> moves;
            for(int i=1; i<=corners[0].x; ++i){
                int cornerX=corners[i].x, cornerY=corners[i].y;
                if(board[cornerX][cornerY]!=4)
                    continue;
                for(int j=1; j<=a[cornerX][cornerY][0]; ++j){
                    POINT p[6]={0};
                    fromAtoPiece(a[cornerX][cornerY][j],p);
                    int shape=whichShape(p);
                    if(inHand[player][shape]==0)
                        continue;
                    POINT tmp[6];
                    copyMove(tmp, p);
                    for(int k=1; k<=p[0].x; ++k){
                        copyMove(tmp, p);
                        for(int l=1; l<=p[0].x; ++l){
                            tmp[l].x-=p[k].x;
                            tmp[l].y-=p[k].y;
                        }
                        if(validMove(tmp,cornerX,cornerY)){
                            std::vector<POINT> tmp1;
                            for(int f=1; f<=p[0].x; ++f){
                                tmp1.push_back(POINT{tmp[f].x+cornerX, tmp[f].y+cornerY});
                            }
                            moves.push_back(tmp1);
                        }

                    }
                    
                }
            }
            return moves;
        }
        bool isGameOver(){
            return generateAllMoves(1).empty() && generateAllMoves(2).empty();
        }
};
//write a function that turns a vector of points to a c array of points
void fromVectorToCArray(std::vector<POINT> v, POINT p[]){
    p[0].x=v.size();
    for(int i=0; i<v.size(); ++i){
        p[i+1]=v[i];
    }
}
//*
int minimax(BOARD& board, int depth, bool isMaximizingPlayer){
    if(depth == 0 || board.isGameOver())
        return board.evaluateBoard();
    if(isMaximizingPlayer){
        int maxEval = std::numeric_limits<int>::min();
        for(const auto& move : board.generateAllMoves(1)){
            POINT p[6];
            fromVectorToCArray(move, p);
            int shape=whichShape(p);
            if(inHand[1][shape]==0)
                continue;
            int protocol=0;
            board.makeMove(p,0,0,1,protocol);
            if(protocol==0)
                continue;
            //printMatrix(board.board);
            int eval = minimax(board, depth-1, false);
            maxEval = std::max(maxEval, eval);
            board.undoMove(p,0,0, 1);
        }
        return maxEval;
    }
    else{
        int minEval = std::numeric_limits<int>::max();
        for(const auto& move : board.generateAllMoves(2)){
            POINT p[6];
            fromVectorToCArray(move, p);
            int shape=whichShape(p);
            if(inHand[2][shape]==0)
                continue;
            int protocol=0;
            board.makeMove(p,0,0, 2, protocol);
            if(protocol==0)
                continue;
            //printMatrix(board.board);
            int eval = minimax(board, depth-1, true);
            minEval = std::min(minEval, eval);
            board.undoMove(p,0,0, 2);
        }
        return minEval;
    }
}
bool genMovesRandomly(BOARD& board){
    std::vector<std::vector<POINT>> moves;
    moves=board.generateAllMoves(1);
    if(moves.empty()){
        std::cout << "= pass\n\n";
        return 0;
    }
    int i=moves.size();
    int protocol=0;
    POINT p[6]={0,0};
    while(protocol==0 && i>0){
        i--;
        int index=rand()%moves.size();
        
        fromVectorToCArray(moves[index], p);
        board.makeMove(p, 0, 0, 1, protocol);
    }
    
    int shape=whichShape(p);
    inHand[1][shape]=0;
    std::string s;
    s=toBlokus(p, 0, 0);
    std::cout << "= " << s << "\n\n";
    return 1;
}


std::vector<POINT> findBestMove(BOARD& board){
    int bestVal = std::numeric_limits<int>::min();
    std::vector<POINT> bestMove;
    for(const auto& move : board.generateAllMoves(1)){
        POINT p[6];
        fromVectorToCArray(move, p);
        int shape=whichShape(p);
        if(inHand[1][shape]==0)
            continue;
        int protocol=0;
        board.makeMove(p, 0, 0, 1, protocol);
        printMatrix(board.board);
        if(protocol==0)
            continue;
        int moveVal = minimax(board, 1, false);
        //printMatrix(board.board);
        board.undoMove(p, 0, 0, 1);
        if(moveVal > bestVal){

            bestMove=move;
            bestVal = moveVal;
        }
    }
    return bestMove;
}
//*/
int main(){
    
    whatEver=generate_rotations(basic_pieces);
    
    int i=0;
    for(const auto& rotated : whatEver){
        int j=1;
        rotations[i][0].x=rotated.size();
        for(const auto& rot : rotated){
            rotations[i][j].x=rot.first;
            rotations[i][j].y=rot.second;
            j++;
        }
        i++;
    }
    for(i=0; i<GRID_SIZE; ++i){
        for(int j=0; j<GRID_SIZE; ++j){
            for(int k=0; k<MAX_ROTATIONS; ++k){
                
                for(int l=1; l<=rotations[k][0].x; ++l)
                {
                    POINT b[MAX_PIECE_SIZE+1];
                    for(int f=1; f<=rotations[k][0].x; ++f){
                        b[l].x=i+rotations[k][f].x-rotations[k][l].x;
                        b[l].y=j+rotations[k][f].y-rotations[k][l].y;
                    }
                    b[0]=rotations[k][0];
                    if(withinBounds(b)){
                        int tmp=a[i][j][0];
                        a[i][j][0]++;
                        a[i][j][tmp]=k;
                        continue;
                    }  
                }
                
            }
        }
    }
    int sum=0;
    for(i=0; i<GRID_SIZE; ++i){
        for(int j=0; j<GRID_SIZE; ++j){
            sum+=a[i][j][0];
        }
    }
    int j;
    BOARD board;
    board.clearBoard();
    POINT* piece = new POINT[MAX_PIECE_SIZE+1];
    int id=a[5][5][1];
    for(i=0; i<=rotations[id][0].x; ++i){
        piece[i].x=rotations[id][i].x;
        piece[i].y=rotations[id][i].y;
    }
    /*
    for(i=1; i<=piece[0].x; ++i){
        std::cout<<piece[i].x<<" "<<piece[i].y<<'\n';
    }
    /*/ 
    for(int x=0; x<=GRID_SIZE+1; ++x){
        board.board[0][x]=-1;
        board.board[GRID_SIZE+1][x]=-1;
        board.board[x][0]=-1;
        board.board[x][GRID_SIZE+1]=-1;
    }
    for(int x=0; x<=NUM_PIECE+1; ++x){
        inHand[1][x]=1;
        inHand[2][x]=1;
    }
  int state=T_UNKNOWN;
  POINT lastMove[MAX_PIECE_SIZE+1];
  int lastx=0, lasty=0;
  while(state!=T_QUIT)
  {
      std::string s, player;
      std::cin >> s;
      if(s=="set_game")
      {
        state=T_SET_GAME;
        std::cin >> s;
        std::cin >> s;
        if(s=="Duo")
        {
            std::cout << "= \n\n";
        }
        else
        {
          std::cout << "Unknown game type:" << s << " try the game mode Duo\n\n";
          return 0;
        }
      }
      else if(s=="clear_board")
      {
          state=T_CLEAR_BOARD;
          board.clearBoard();
          printMatrix(board.board);
          std::cout << "= \n\n";
      }
      else if(s=="cputime")
      {
          state=T_CPUTIME;
          std::cout << "= 0\n\n";
      }
      else if(s=="genmove")
      {
        /*
          state=T_GENMOVE;
          std::cin >> s;
          bool ok=1;
          for(int index=1; index<=board.corners[0].x && ok; ++index){
                int cornerX=board.corners[index].x, cornerY=board.corners[index].y;
                for(int j=1; j<=a[cornerX][cornerY][0] && ok; ++j){
                    POINT p[6]={0};
                    fromAtoPiece(a[cornerX][cornerY][j],p);
                    int shape=whichShape(p);
                    if(board.validMove(p,cornerX,cornerY) && inHand[1][shape]){
                           inHand[1][shape]=0;
                           ok=0;
                            board.makeMove(p,cornerX,cornerY,1);
                            s=toBlokus(p,cornerX,cornerY);
                            printMatrix(board.board);
                            fromAtoPiece(a[cornerX][cornerY][j],p);
                            copyMove(lastMove, p);
                            lastx=cornerX;
                            lasty=cornerY;
                           printMatrix(board.board);
                           std::cout << "= " << s << "\n\n";
                    }
                }
          }
          /*
          std::vector<POINT> bestMove;
            bestMove=findBestMove(board);
            POINT p[6];
            fromVectorToCArray(bestMove, p);
            //std::cerr << "-----------------------adfs";
            //printMatrix(board.board);
            int protocol=0;
            board.makeMove(p, 0, 0, 1, protocol);
            int shape=whichShape(p);
            inHand[1][shape]=0;
            printMatrix(board.board);
            /*/
            bool ok=genMovesRandomly(board);
            for(int i=0; i<=board.corners[0].x; ++i){
                std::cerr << board.corners[i].x << " " << board.corners[i].y << '\n';
            }
            printMatrix(board.board);
            //*/
      }
      else if(s=="play")
      {
          state=T_PLAY;
          std::cin >> s;
          std::getline(std::cin, s);
          s=trim(s);
          std::cerr << "\n";
          POINT p[MAX_PIECE_SIZE+1];
          fromBlokus(s, p);
          int protocol=0;
          board.makeMove(p,0,0,2, protocol);
          int shape=whichShape(p);
            inHand[2][shape]=0;
          printMatrix(board.board);
          std::cout << "= \n\n";
      }
      else if(s=="quit")
      {
        std::cout << "= \n\n";
        state=T_QUIT;
      }
      else if(s=="undo"){
        board.undoMove(lastMove, lastx, lasty, 1);
        printMatrix(board.board);
      }

  }
  return 0;

}