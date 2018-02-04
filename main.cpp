/************************************************************
Sokoban project - Main file
Copyright Florent DIEDLER
Date : 27/02/2016

Please do not remove this header, if you use this file !
************************************************************/
#include "maze.h"
#include "graphic.h"
#include "utils/console.h"
#include "utils/coord.h"
#include <iostream>

// Init allegro with 800x600 (resolution)
Graphic g(800,600,32);

int main()
{
    // Do not touch !
    Console::getInstance()->setColor(_COLOR_DEFAULT);

    // Load level from a file
    Maze m("./levels/medium/medium_1.dat");
    if (!m.init()) return -1;
    std::cout << m << std::endl;

    bool dead= false;

    if (g.init())
    {
        // While playing...
        while (!g.keyGet(KEY_ESC) && !g.keyGet(KEY_A))
        {
            // Check if user has pressed a key
            if (g.keyPressed())
            {
                bool win = false;
                switch (g.keyRead())
                {
                    case ARROW_UP:
                        win = m.updatePlayer(TOP);
                        break;
                    case ARROW_BOTTOM:
                        win = m.updatePlayer(BOTTOM);
                        break;
                    case ARROW_RIGHT:
                        win = m.updatePlayer(RIGHT);
                        break;
                    case ARROW_LEFT:
                        win = m.updatePlayer(LEFT);
                        break;
                    case KEY_0_PAD://Reset du field
                        m.reset(m.elemZero);
                        std::cout << "RESET" << std::endl;
                        break;
                    case KEY_1_PAD://Bruteforce
                        m.bruteForce(g);
                        std::cout << "-BruteForce " << std::endl;
                        break;
                    case KEY_2_PAD://BFS
                        m.bfs(g);
                        std::cout << "-Breadth First Search ";
                        if(dead) std::cout << "+ Dead Locks";
                        std::cout << std::endl;
                        break;
                    case KEY_3_PAD://DFS
                        m.dfs(g);
                        std::cout << "-Depth First Search ";
                        if(dead) std::cout << "+ Dead Locks";
                        std::cout << std::endl;
                        break;
                    case KEY_4_PAD://Best First Search h(n)
                        m.bestFirstSearch(g);
                        std::cout << "-Best First Search ";
                        if(dead) std::cout << "+ Dead Locks";
                        std::cout << std::endl;
                        break;
                    case KEY_5_PAD://Astar h(n) + g(n)
                        m.astar(g);
                        std::cout << "-Astar ";
                        if(dead) std::cout << "+ Dead Locks";
                        std::cout << std::endl;
                        break;
                    case KEY_9_PAD: //Deadlocks on/off
                        m.reset(m.elemZero);
                        if(!dead){
                            m.deadLock();
                            dead= true;
                            std::cout << "Deadlocks enabled" << std::endl;
                        }else{
                            m.getDead().clear();
                            std::cout << "Deadlocks disabled" << std::endl;
                            dead= false;
                        }
                        break;
                }

                if (win) std::cout << "Win ! " << std::endl;
            }

            // Display on screen
            g.clear();
            m.draw(g);
            g.display(Coord::m_nb_col);

        }
    }

    // Free memory
    g.deinit();
    Console::deleteInstance();

    return 0;
}
END_OF_MAIN()
