
#include "globalshit.h"
#include <vector>
#include <set>
#include <algorithm>
#include <sstream>

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
        s+=" ";
    }
    return s;
}
int main(){
    std::string s="a1 a2 a3";
    POINT piece[6]={{2,0},{1,1},{1,2}};
    s=toBlokus(piece);
    std::cout<<s;
    return 0;
}