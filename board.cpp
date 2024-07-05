#include "globalshit.h"
/*
    Board notation:
    0 - empty cell
    1 - my blocks
    2 - opponent blocks
    3 - illegal cell
    4 - corner cell
    -1 - out of bounds
*/
void printMatrix(int a[GRID_SIZE+1][GRID_SIZE+1]){
    for(int i=1; i<=GRID_SIZE; ++i){
        for(int j=1; j<=GRID_SIZE; ++j){
            std::cout<<a[i][j]<<" ";
        }
        std::cout<<'\n';
    }
}

class BOARD{
    public:
        int board[GRID_SIZE+1][GRID_SIZE+1];
        POINT corners[GRID_SIZE*GRID_SIZE+5]={0};
        int lastMoveCornerCount=0, lastMoveIllegalCount=0;
        POINT illegal[GRID_SIZE*GRID_SIZE+5]={0};
        void clearBoard(){
            for(int i=1; i<=GRID_SIZE; ++i){
                for(int j=1; j<=GRID_SIZE; ++j){
                    board[i][j]=0;
                }
            }
        }
        bool validMove(POINT piece[], int xo, int yo){
            for(int i=1; i<=piece[0].x; ++i){
                int x=piece[i].x + xo;
                int y=piece[i].y + yo;
                if(!(x>0 && x<=GRID_SIZE && y>0 && y<=GRID_SIZE) || board[x][y]==3 || board[x][y]==1 || board[x][y]==2 || board[x][y]==-1)
                    return 0;
            }
        }
        void setCorners(POINT piece[], int x0, int y0){
            lastMoveCornerCount=0;
            int dx[4]={-1, 1, 1, -1};
            int dy[4]={1, 1,-1, -1};
            for(int i=1; i<=piece[0].x; ++i){
                for(int k=0; k<4; ++k){
                    int x=piece[i].x+dx[k]+x0;
                    int y=piece[i].y+dy[k]+y0;
                    if(board[x][y]==0){
                        board[x][y]=4;
                        corners[0].x++;
                        corners[corners[0].x]=POINT{x, y};
                        lastMoveCornerCount++;
                    }
                }
            }
        }
        void unSetCoreners(){
            for(int i=corners[0].x-lastMoveCornerCount; i<=corners[0].x; ++i){
                board[corners[i].x][corners[i].y]=0;
            }
            corners[0].x-=lastMoveCornerCount;
        }
        void markIllegal(POINT piece[], int x0, int y0){
            lastMoveIllegalCount=0;
            illegal[0].x=0;
            int dx[4]={-1, 0, 1, 0};
            int dy[4]={0, 1, 0, -1};
            for(int i=1; i<=piece[0].x; ++i){
                for(int k=0; k<4; ++k){
                    int x=piece[i].x+dx[k]+x0;
                    int y=piece[i].y+dy[k]+y0;
                    if(board[x][y]==0){
                        board[x][y]=3;
                        illegal[0].x++;
                        illegal[illegal[0].x]=POINT{x, y};
                        lastMoveIllegalCount++;
                    }
                }
            }
        }
        void unMarkIllegal(){
            for(int i=illegal[0].x-lastMoveIllegalCount; i<=illegal[0].x; ++i){
                board[illegal[i].x][illegal[i].y]=0;
            }
        }
        void makeMove(POINT piece[], int x, int y){   
            for(int i=1; i<=piece[0].x; ++i){
                board[x+piece[i].x][y+piece[i].y]=1;
            }
            markIllegal(piece, x, y);
            setCorners(piece, x, y);
            
        }
        void undoMove(POINT piece[], int x, int y){
            unMarkIllegal();
            printMatrix(board);
            std::cout<<"-------------------\n";
            unSetCoreners();
            printMatrix(board);
            std::cout<<"-------------------\n";
            for(int i=1; i<=piece[0].x; ++i){
                board[x+piece[i].x][y+piece[i].y]=0;
            }
            corners[0].x-=lastMoveCornerCount;
            illegal[0].x-=lastMoveIllegalCount;
        }
};


#include <vector>
#include <set>
#include <algorithm>

const int INFINITY=1e9;
const int MAX_ROTATIONS=91;




std::vector<Piece> basic_pieces={
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
    for(const auto& p : pieces){
        Coordinates tmp;
        tmp=toCoord(p);
        for(int i=0; i<4; ++i){
            rotations.insert(normalize(tmp));
            rotations.insert(normalize(mirror(tmp)));
            tmp=rotate90(tmp);
        }
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

std::set<Coordinates> whatEver;

POINT rotations[MAX_ROTATIONS+1][MAX_PIECE_SIZE+1];

int a[GRID_SIZE+1][GRID_SIZE+1][MAX_ROTATIONS+1];

///function to transform an stl vector int to a c array


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
                POINT b[MAX_PIECE_SIZE+1];
                for(int l=1; l<=rotations[k][0].x; ++l)
                {
                        b[l].x=i+rotations[k][l].x;
                        b[l].y=j+rotations[k][l].y;
                }
                b[0]=rotations[k][0];
                if(withinBounds(b)){
                    int tmp=a[i][j][0];
                    a[i][j][0]++;
                    a[i][j][tmp]=k;
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
    POINT piece[MAX_PIECE_SIZE+1];
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
    return 0;

}
