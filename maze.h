/************************************************************
Sokoban project - Maze file
Copyright Florent DIEDLER
Date : 27/02/2016

Please do not remove this header, if you use this file !
************************************************************/

#ifndef MAZE_H_INCLUDED
#define MAZE_H_INCLUDED

#include <vector>
#include <string>
#include "elements.h"
#include <queue>
#include <stack>
#include <algorithm>


// Max size for the field
#define NB_MAX_WIDTH     30
#define NB_MAX_HEIGHT    30

class Graphic;

enum
{
    SPRITE_GROUND = 0, SPRITE_WALL = 1, SPRITE_BOX = 2,
    SPRITE_BOX_PLACED = 3, SPRITE_GOAL = 4, SPRITE_MARIO = 5,
    SPRITE_DEADSQUARE = 9
};

class Maze
{
    private:
        unsigned char m_lig, m_col;
        unsigned short m_pos_player;
        char m_dir_player;
        std::string m_level_path;
        bool isDead;

        std::vector<unsigned char> m_field; // field
        std::vector<unsigned short> m_pos_boxes; // box positions
        std::vector<unsigned short> m_pos_goals; // goal positions
        std::vector<unsigned short> m_pos_dead; // dead lock positions
        std::vector<unsigned short> m_dir; // solution algo


        friend std::ostream& operator << (std::ostream& O, const Maze& n);

        bool _isCompleted() const;
        bool _canPushBox(unsigned short posBox, char dir) const;
        bool _load(const std::string& path);

    public:
        Maze(const std::string& path);
        ~Maze();

        bool init();
        bool updatePlayer(char dir);
        void draw(const Graphic& g);

        ///ALGOS
        void bruteForce(Graphic& g);
        void bfs(Graphic& g);
        void dfs(Graphic& g);
        void afficherSolution(Graphic& g);
        void reset(Elements e);
        int bfsPoids (Elements e);
        void bestFirstSearch(Graphic& g);
        void astar(Graphic& g);
        bool updateBox (unsigned short posBox, char dir);
        Elements elemZero;
        bool isGoalUnique (int pos, std::vector<unsigned short> a);
        std::vector<unsigned short> invert(std::vector<unsigned short> a);
        static bool sortByPoids(const Elements &a, const Elements &b);


        ///ALGOS

        //Deadlock
        void deadLock();

        // Specific getters for field
        bool isSquareWalkable(unsigned short pos) const;
        bool isSquareGround(unsigned short pos) const;
        bool isSquareBox(unsigned short pos) const;
        bool isSquareGoal(unsigned short pos) const;
        bool isSquareWall(unsigned short pos) const;
        bool isSquareDead(unsigned short pos) const;
        bool isSquareBoxPlaced(unsigned short pos) const;


        // Other getters
        const std::string& getLevelPath() const;
        unsigned short getPosPlayer() const;
        unsigned int getSize() const;
        void setSquare(unsigned short pos, unsigned char s);
        const std::vector<unsigned char>& getField() const;
        const std::vector<unsigned short>& getGoals() const;
        std::vector<unsigned short>& getDead();
        std::vector<unsigned short> getPosBoxes() const;
        unsigned short getPosBoxe() const;

        // Setter
        void setPlayerPos(unsigned short p);
        void setBoxesPos (std::vector<unsigned short> a);
        void setBoxesPos (int num, unsigned short pos);
};

// Inline implementation of getters and setters (DO NOT TOUCH !)

inline void Maze::setPlayerPos(unsigned short p) {
    if (p < this->getSize()) // NB: p always > 0 because unsigned short
        this->m_pos_player = p;
}

inline void Maze::setBoxesPos(int num, unsigned short pos) {
this->m_pos_boxes[num] = pos;
}


inline const std::string& Maze::getLevelPath() const { return this->m_level_path; }
inline const std::vector<unsigned short>& Maze::getGoals() const { return this->m_pos_goals; }
inline const std::vector<unsigned char>& Maze::getField() const { return this->m_field; }
inline std::vector<unsigned short>& Maze::getDead() { return this->m_pos_dead; }

inline unsigned int Maze::getSize() const { return this->m_field.size(); }
inline std::vector<unsigned short> Maze::getPosBoxes() const { return m_pos_boxes; }
inline unsigned short Maze::getPosPlayer() const { return m_pos_player;}

inline void Maze::setSquare(unsigned short pos, unsigned char s)
{
    if (pos < (this->m_lig*this->m_col))
    {
        this->m_field[pos] = s;
    }
}

inline bool Maze::isSquareWalkable(unsigned short pos) const
{
    return (this->m_field[pos] == SPRITE_GROUND || this->m_field[pos] == SPRITE_GOAL || this->m_field[pos] == SPRITE_DEADSQUARE ? true : false);
}

inline bool Maze::isSquareGround(unsigned short pos) const
{
    return ((this->m_field[pos] == SPRITE_GROUND) ? true : false);
}

inline bool Maze::isSquareBox(unsigned short pos) const
{
    return ((this->m_field[pos] == SPRITE_BOX || this->m_field[pos] == SPRITE_BOX_PLACED) ? true : false);
}

inline bool Maze::isSquareGoal(unsigned short pos) const
{
    return (this->m_field[pos] == SPRITE_GOAL ? true : false);
}

inline bool Maze::isSquareWall(unsigned short pos) const
{
    return (this->m_field[pos] == SPRITE_WALL ? true : false);
}

inline bool Maze::isSquareBoxPlaced(unsigned short pos) const
{
    return (this->m_field[pos] == SPRITE_BOX_PLACED ? true : false);
}

inline bool Maze::isSquareDead(unsigned short pos) const
{
    if(!this->m_pos_dead.empty()){
        for(unsigned int i=0; i<this->m_pos_dead.size(); i++){
            if(this->m_pos_dead[i] == pos) return true;
        }
    }
    return false;
}





#endif // MAZE_H_INCLUDED
