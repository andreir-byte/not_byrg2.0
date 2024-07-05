
#pragma once
#include<iostream>
#include <vector>
#include <set>
enum{T_UNKNOWN, T_SET_GAME,T_CLEAR_BOARD, T_CPUTIME, T_GENMOVE, T_PLAY, T_QUIT};
static const int GRID_SIZE=14;
static const int MAX_PIECE_SIZE=5;
static const int NUM_PIECE=21;
using Coordinates = std::set<std::pair<int,int>>;
using Piece = std::vector<std::string>;
struct POINT{
    int x, y;
};
class cell
{
    private:
        int x, y;
    public:
        void translate(int dx, int dy);
};
