
#include "globalshit.h"
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

void fromBlokus(std::string s, POINT p[]){
    std::vector<std::string> words;
    std::istringstream stream(s);
    std::string word;
    while (stream >> word) {
        words.push_back(word);
    }
    for(const auto& w : words){
        
          int x, y=0;
          x=w[0]-'a'+1;
          y=15-(int)(w[1]-'0');
          if(w.size()==3){
              y*=10;
              y+=w[2]-'0';
          }
          p[0].x++;
          p[p[0].x].x=x;
          p[p[0].x].y=y;
    }
}

std::string toBlokus(POINT p[]){
    std::string s="";
    for(int i=1; i<=p[0].x; ++i){
        s+=(char)(p[i].x-1+'a');
        s+=std::to_string(15-p[i].y);
        s+=",";
    }
    return s;
}
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
    
    std::cout << sum;
    return 0;
}
