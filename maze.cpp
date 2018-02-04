/************************************************************
Sokoban project - Maze file
Copyright Florent DIEDLER
Date : 27/02/2016

Please do not remove this header, if you use this file !
************************************************************/

#include "maze.h"
#include "graphic.h"
#include "utils/console.h"
#include "utils/coord.h"
#include <fstream>
#include <iomanip>
#include <time.h>

Maze::Maze(const std::string& path)
    : m_lig(0), m_col(0), m_pos_player(0), m_dir_player(TOP), m_level_path(path), isDead(false)
{
}

Maze::~Maze()
{
}

bool Maze::init()
{
    bool res = this->_load(this->m_level_path);
    if (!res)
    {
        std::cerr << "Cannot load maze... Check file : " << this->m_level_path << std::endl;
        return false;
    }

    return res;
}

// Check if all boxes are on a goal
bool Maze::_isCompleted() const
{
    for (unsigned int i=0; i<this->m_pos_boxes.size(); ++i)
    {
        if (!this->isSquareBoxPlaced(this->m_pos_boxes[i]))
            return false;
    }
    return true;
}

// Check if we can push a box in a direction
// INPUT: position of the box to check, direction,
// OUTPUT : the position of the box after pushing
//      TRUE if all goes right otherwise FALSE
bool Maze::_canPushBox(unsigned short posBox, char dir) const
{
    // Check if this position is a box !
    if (!this->isSquareBox(posBox))
        return false;

    // Compute new position according to push direction
    unsigned short newPosBox = Coord::getDirPos(posBox, dir);

    // Can we push the box ?
    return this->isSquareWalkable(newPosBox);
}

// Load a maze from a file (DO NOT TOUCH)
bool Maze::_load(const std::string& path)
{
    std::vector<unsigned short> tmpPosBoxes;
    std::ifstream ifs(path.c_str());
    if (ifs)
    {
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(ifs, line))
        {
            lines.push_back(line);
            this->m_lig++;
            this->m_col = (this->m_col < line.size() ? line.size() : this->m_col);
        }
        ifs.close();

        if (this->m_col > NB_MAX_WIDTH || this->m_lig > NB_MAX_HEIGHT)
        {
            std::cerr << "Maze::load => Bad formatting in level data..." << std::endl;
            return false;
        }

        Coord::m_nb_col = this->m_col;
        for (unsigned int i=0; i<lines.size(); i++)
        {
            //LDebug << "Maze::load => Reading : " << lines[i];
            for (unsigned int j=0; j<this->m_col; j++)
            {
                if (j < lines[i].size())
                {
                    bool both = false;
                    unsigned short pos = Coord::coord1D(i, j);
                    unsigned char s = (unsigned char)(lines[i][j] - '0');

                    // Need to add a goal and a box ;)
                    if (s == SPRITE_BOX_PLACED)
                    {
                        both = true;
                    }

                    if (s == SPRITE_GOAL || both)
                    {
                        this->m_pos_goals.push_back(pos);
                    }
                    if (s == SPRITE_BOX || both)
                    {
                        tmpPosBoxes.push_back(pos);
                    }

                    // Assign player position
                    if (s == SPRITE_MARIO)
                    {
                        this->m_pos_player = pos;
                        //LDebug << "\tAdding player pos (" << pos << ")";
                        s = SPRITE_GROUND;
                    }

                    // Add this value in the field
                    this->m_field.push_back(s);
                }
                else
                {
                    // Here - Out of bound
                    this->m_field.push_back(SPRITE_GROUND);
                }
            }
        }

        // Copy box position
        this->m_pos_boxes.resize(tmpPosBoxes.size());
        for (unsigned int i=0; i<tmpPosBoxes.size(); ++i)
        {
            this->m_pos_boxes[i] = tmpPosBoxes[i];
        }

        //Sauvegarde de l'element initial
        this->elemZero= Elements(this->m_pos_player, this->m_field, this->m_pos_boxes, -1, 0, 0);

        return (this->m_pos_boxes.size() == this->m_pos_goals.size());
    }
    else
    {
        std::cerr << "Maze::load => File does not exist..." << std::endl;
    }

    return false;
}

// Inverse un vecteur
std::vector<unsigned short> Maze::invert(std::vector<unsigned short> a){

    int tmp=0;
    for (unsigned int i=0; i<a.size()/2; i++){
        tmp= a[i];
        a[i]= a[a.size()-1-i];
        a[a.size()-1-i]= tmp;
    }
    return a;
}

// Bouge le player dans la direction donnée
// Gestion de toutes les interactions
// suite à un mouvement du player
bool Maze::updatePlayer(char dir)
{
    if (dir < 0 || dir > MAX_DIR)
    {
        std::cerr << "Maze::updatePlayer => Direction not correct... " << +dir << std::endl;
        return false;
    }
    bool vic= false;
    unsigned short pos=0, newPosBox=0;
    this->isDead= false;
    //Mouvements du player

    //Position où on veut aller
    pos = Coord::getDirPos(getPosPlayer(), dir);
    //Position au delà d'où on veut aller
    newPosBox = Coord::getDirPos(pos, dir);

    //Mouvements des caisses
    if (_canPushBox(pos, dir))
    {
        //On cherche quelle caisse est solicité
        for (unsigned short int i=0; i<getPosBoxes().size() ; i++)
        {
            if (getPosBoxes()[i]==pos)
            {
                //On bouge la caisse
                setBoxesPos(i, newPosBox);

                //Mise a jour de la matrice
                //Pousse une caisse NOT PLACED, sur un GROUND ou DEAD
                if ((isSquareDead(newPosBox) || isSquareGround(newPosBox)) && !isSquareBoxPlaced(pos))
                {
                    if (isSquareDead(newPosBox)) this->isDead = true;
                    this->m_field[pos] = SPRITE_GROUND;
                    this->m_field[newPosBox] = SPRITE_BOX;
                }
                //Pousse une caisse NOT PLACED, sur un GOAL
                if (isSquareGoal(newPosBox) && !isSquareBoxPlaced(pos))
                {
                    this->m_field[pos] = SPRITE_GROUND;
                    this->m_field[newPosBox] = SPRITE_BOX_PLACED;
                }
                //Pousse une caisse PLACED, sur un GOAL
                if (isSquareGoal(newPosBox) && isSquareBoxPlaced(pos))
                {
                    this->m_field[pos] = SPRITE_GOAL;
                    this->m_field[newPosBox] = SPRITE_BOX_PLACED;
                }
                //Pousse une caisse PLACED, sur un GROUND ou DEAD
                if ((isSquareDead(newPosBox) || isSquareGround(newPosBox)) && isSquareBoxPlaced(pos))
                {
                    if (isSquareDead(newPosBox)) this->isDead = true;
                    this->m_field[pos] = SPRITE_GOAL;
                    this->m_field[newPosBox] = SPRITE_BOX;
                }
            }
        }
        if(_isCompleted()) vic= true;
    }

    //Mouvement du player
    if (isSquareWalkable(pos)) setPlayerPos(pos);

    //Direction du Player
    this->m_dir_player=dir;

    ///TEST BFS POIDS
    /*int poids = bfsPoids(Elements(getPosPlayer(), this->m_field, this->m_pos_boxes, -1, 0, 0));
    std::cout << poids << std::endl;*/



    return vic;
}

// Mouvements d'une caisse pour heuristique
bool Maze::updateBox (unsigned short numBox, char dir)
{
    bool s= false;
    unsigned short oldPos= this->m_pos_boxes[numBox], oppPos= Coord::getOppositeDirPos(oldPos, dir), newPos= Coord::getDirPos(oldPos, dir);

    if (!isSquareWall(newPos) && !isSquareWall(oppPos) && !isSquareDead(newPos))
    {

        if(this->m_field[newPos] == SPRITE_GOAL) s= true;
        else if (this->m_field[newPos] == SPRITE_GROUND)
        {
            this->m_field[newPos] = SPRITE_BOX;
            this->m_field[oldPos] = SPRITE_GROUND;
        }

        this->m_pos_boxes[numBox] = newPos;
    }
    return s;

}


// Tri par poids
bool Maze::sortByPoids(const Elements & a, const Elements & b)
{
    return a.m_poids < b.m_poids;
}

// Recherche du poids d'un noeuds (e)
int Maze::bfsPoids (Elements e)
{
    std::vector<Elements> save;
    std::queue<Elements> file;
    Elements temp;
    std::vector<unsigned short> a;
    std::vector<unsigned short> goalMarque;

    bool vic, s, tmp;
    int pred, poids= 0;

    //BFS pour toutes les caisses
    for(unsigned short i=0; i<e.m_pos_boxes.size(); i++)
    {

        //Si on a deja une box placee on ne fait pas de BFS dessus
        if(e.m_field[e.m_pos_boxes[i]] != SPRITE_BOX_PLACED){

            //Reset complet pour chaque BFS
            pred= 0;
            this->m_dir.clear();
            save.clear();
            save.push_back(Elements(e.m_pos_player, e.m_field, e.m_pos_boxes, -1, 0, 0));
            file.push(save[0]);
            vic= false;
            s= false;
            tmp= false;

            //On affiche seulement la box qu'on va bouger
            for(unsigned int j=0; j<getPosBoxes().size(); j++){
                if(j!=i) this->m_field[getPosBoxes()[j]] = SPRITE_GROUND;
            }

            while(!file.empty())
            {

                temp= file.front();
                file.pop();

                if(!vic) reset(temp);

                for(int j=0; j<4; j++)
                {
                    //On bouge la caisse
                    if(!vic) vic= updateBox(i, j);

                    //Si la victoire est sur un goal marqué, alors on a pas gagné
                    if(vic && !isGoalUnique(m_pos_boxes[i], goalMarque) && !tmp) vic= false;

                    if(!vic){
                        //Si l'element obtenu est unique, on le met dans le vecteur et file
                        s= true;
                        //Unique ou pas
                        for (unsigned int k=0; k<save.size(); k++){
                            if (getPosBoxes()[i] == save[k].m_pos_boxes[i]) s= false;
                        }
                        //Si unique, on met dans vecteur et file
                        if (s){
                            save.push_back(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, j, 0));
                            file.push(save.back());
                        }
                    }

                    //On remplit la derniere position dans le vecteur une seule fois apres victoire
                    if(vic && !tmp){
                        save.push_back(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, j, 0));
                        tmp= true;
                        //On marque le goal
                        goalMarque.push_back(m_pos_boxes[i]);
                    }

                    //On remet le niveau a TEMP
                    reset(temp);

                }
                //Predecesseurs
                pred++;
            }
            //Reviens à l'état initial
            reset(save[0]);

            //On affiche le chemin obtenu en fonction des pred
            int j= save.size()-1;
            //Remplissage du vecteur de direction
            while(save[j].m_pred != -1)
            {
                poids++;
                m_dir.push_back(save[j].m_dir);
                j= save[j].m_pred;
            }

            //On remet les box cachées
            for(unsigned int j=0; j<getPosBoxes().size(); j++){
                if(j!=i) this->m_field[getPosBoxes()[j]] = SPRITE_BOX;
            }
        if(save.size()==1) poids+=20;

        }else{
            //Si le box est déja placée, poids plus petit
            poids-= 10;
            //On marque le goal
            goalMarque.push_back(m_pos_boxes[i]);
        }
    }
    this->m_dir.clear();

    reset(e);

    return poids;

}


// Heuristique f(n) = h(n)
void Maze::bestFirstSearch(Graphic& g)
{
    //Temps pour résoudre
    float temps= clock();
    //Pour commencer l'algo, on reset tout
    reset(elemZero);

    //Vecteur save et save trié (qui remplace la file d'un BFS classique)
    std::vector<Elements> save;
    std::vector<Elements> saveSorted;
    save.push_back(elemZero);
    saveSorted.push_back(elemZero);

    //Deadlocks
    bool dead=false;

    bool vic=false, s=false, abort=false, tmp=false, tmp1=false;
    Elements temp;
    int pred=0, poidsNoeud=0;

    while(!vic && !abort)
    {

        //Pour cette boucle on fait des tests sur le poids le plus petit
        temp= saveSorted.front();
        //Predecesseur
        for(unsigned short i=0; i<save.size(); i++){
            if(temp.m_pos_player == save[i].m_pos_player && temp.m_field == save[i].m_field) pred= i;
        }
        saveSorted.erase(saveSorted.begin());

        //Reset tant qu'on gagne pas
        if(!vic) reset(temp);

        //Tests pour chaque direction
        for (int i=0; i<4; i++)
        {

            //Si on gagne pas, on bouge le player
            if(!vic) vic= updatePlayer(i);

            //Deadlock
            dead= this->isDead;
            //Si une caisse est sur dead, on passe a la suite
            if(!dead)
            {

                if(vic)
                {
                    if(!tmp1)
                    {
                        std::cout << "Win !" << std::endl;
                        tmp1= true;
                    }
                }
                else
                {
                    //Si l'element obtenu est unique, on le met dans le vecteur
                    s= true;
                    //Unique ou pas
                    for (unsigned int j=0; j<save.size(); j++)
                    {
                        if (getPosPlayer() == save[j].m_pos_player && (getField() == save[j].m_field)) s= false;
                    }
                    //Si unique, on met dans vecteur
                    if (s)
                    {
                        //On calcul le poids de chaque noeud unique puis on le met dans save
                        poidsNoeud= bfsPoids(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, i, 0));
                        //On le met dans save et saveSorted
                        save.push_back(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, i, poidsNoeud));
                        saveSorted.push_back(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, i, poidsNoeud));
                        //On tri le vecteur saveSorted en fonction du poids
                        std::sort(saveSorted.begin(), saveSorted.end(), sortByPoids);

                        //Affichage
                        std::cout << pred << " " << i << " " << save.back().m_pos_player << " " << poidsNoeud << " " << saveSorted.size() << std::endl;
                    }
                }
            }

            //On remplit la derniere position dans le vecteur une seule fois apres victoire
            if(vic && !tmp)
            {
                save.push_back(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, i, 0));
                tmp= true;
            }
            //On reset pour chaque direction
            if(!vic) reset(temp);

        }
        //Stop manuel
        if(g.keyPressed())
        {
            if (g.keyRead()==KEY_BACKSPACE) abort= true;
        }

    }

    reset(elemZero);

    if(!abort)
    {

        //On affiche le chemin obtenu en fonction des pred
        int j= save.size()-1;
        //Remplissage du vecteur de direction
        while(save[j].m_pred != -1)
        {
            m_dir.push_back(save[j].m_dir);
            j= save[j].m_pred;
        }
        //On inverse le vecteur
        this->m_dir= invert(this->m_dir);
       // this->m_dir.pop_back();
        //Affichage du vecteur inversé
        std::cout << "Vecteur : ";
        for(unsigned int i=0; i<this->m_dir.size(); i++) std::cout << this->m_dir[i] << " ";
        //Affichage du nombre d'éléments uniques
        std::cout << std::endl << "Nombre d'elements uniques : " << save.size() << std::endl;
        //Affichage du nombre de mouvements
        std::cout << "Nombre de mouvements : " << m_dir.size() << std::endl;

        //Affichage du temps de resolution
        std::cout << "TEMPS : " << (clock() - temps)/1000 << " secondes" << std::endl;

        //Animation
        afficherSolution(g);

    }else std::cout << "BestFirstSearch CANCELLED" << std::endl;

}

// Astar f(n) = h(n) + g(n)
void Maze::astar(Graphic& g)
{
    //Temps pour résoudre
    float temps= clock();
    //Pour commencer l'algo, on reset tout
    reset(elemZero);

    //Astar
    int o=0;
    std::vector<unsigned short> O;

    //Vecteur save et save trié (qui remplace la file d'un BFS classique)
    std::vector<Elements> save;
    std::vector<Elements> saveSorted;
    save.push_back(elemZero);
    saveSorted.push_back(elemZero);

    //Deadlocks
    bool dead=false;

    bool vic=false, s=false, abort=false, tmp=false, tmp1=false;
    Elements temp;
    int pred=0, poidsNoeud=0;

    while(!vic && !abort)
    {

        //Pour cette boucle on fait des tests sur le poids le plus petit
        temp= saveSorted.front();
        //Predecesseur
        for(unsigned short i=0; i<save.size(); i++){
            if(temp.m_pos_player == save[i].m_pos_player && temp.m_field == save[i].m_field) pred= i;
        }
        saveSorted.erase(saveSorted.begin());

        //Reset tant qu'on gagne pas
        if(!vic) reset(temp);

        //Tests pour chaque direction
        for (int i=0; i<4; i++)
        {

            //Si on gagne pas, on bouge le player
            if(!vic) vic= updatePlayer(i);

            //Deadlock
            dead= this->isDead;
            //Si une caisse est sur dead, on passe a la suite
            if(!dead)
            {

                if(vic)
                {
                    if(!tmp1)
                    {
                        std::cout << "Win !" << std::endl;
                        tmp1= true;
                    }
                }
                else
                {
                    //Si l'element obtenu est unique, on le met dans le vecteur
                    s= true;
                    //Unique ou pas
                    for (unsigned int j=0; j<save.size(); j++)
                    {
                        if (getPosPlayer() == save[j].m_pos_player && (getField() == save[j].m_field)) s= false;
                    }
                    //Si unique, on met dans vecteur
                    if (s)
                    {
                        //On calcul le poids de chaque noeud unique
                        poidsNoeud= bfsPoids(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, i, 0));

                        //On ajoute à ce poids, le nombre de coups qu'il faut pour arriver au noeud
                        o= save.size()-1;
                        //Remplissage du vecteur de direction
                        while(save[o].m_pred != -1)
                        {
                            O.push_back(save[o].m_dir);
                            o= save[o].m_pred;
                        }
                        //On rajoute la taille du vecteur des directions (+1 car on a toujours pas mis le noeud actuel)
                        poidsNoeud+= O.size()+1;
                        O.clear();

                        //On le met dans save et saveSorted
                        save.push_back(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, i, poidsNoeud));
                        saveSorted.push_back(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, i, poidsNoeud));
                        //On tri le vecteur saveSorted en fonction du poids
                        std::sort(saveSorted.begin(), saveSorted.end(), sortByPoids);

                        //Affichage
                        //std::cout << pred << " " << i << " " << save.back().m_pos_player << " " << poidsNoeud << " " << saveSorted.size() << std::endl;
                    }
                }
            }

            //On remplit la derniere position dans le vecteur une seule fois apres victoire
            if(vic && !tmp)
            {
                save.push_back(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, i, 0));
                tmp= true;
            }
            //On reset pour chaque direction
            if(!vic) reset(temp);

        }
        //Stop manuel
        if(g.keyPressed())
        {
            if (g.keyRead()==KEY_BACKSPACE) abort= true;
        }

    }

    reset(elemZero);

    if(!abort)
    {

        //On affiche le chemin obtenu en fonction des pred
        int j= save.size()-1;
        //Remplissage du vecteur de direction
        while(save[j].m_pred != -1)
        {
            m_dir.push_back(save[j].m_dir);
            j= save[j].m_pred;
        }
        //On inverse le vecteur
        this->m_dir= invert(this->m_dir);
       // this->m_dir.pop_back();
        //Affichage du vecteur inversé
        std::cout << "Vecteur : ";
        for(unsigned int i=0; i<this->m_dir.size(); i++) std::cout << this->m_dir[i] << " ";
        //Affichage du nombre d'éléments uniques
        std::cout << std::endl << "Nombre d'elements uniques : " << save.size() << std::endl;
        //Affichage du nombre de mouvements
        std::cout << "Nombre de mouvements : " << m_dir.size() << std::endl;

        //Affichage du temps de resolution
        std::cout << "TEMPS : " << (clock() - temps)/1000 << " secondes" << std::endl;

        //Animation
        afficherSolution(g);

    }else std::cout << "ASTAR CANCELLED" << std::endl;

}


// Verifie si un nombre entré est déja dans le vecteur entré
// TRUE quand unique, FALSE sinon
bool Maze::isGoalUnique (int pos, std::vector<unsigned short> a){
    for (unsigned short i=0; i<a.size(); i++){
        if(a[i]== pos) return false;
    }
    return true;
}


// Display maze on screen with Allegro
void Maze::draw(const Graphic& g)
{

    for(unsigned int i=0; i<this->getSize(); i++)
    {
        unsigned int l = 0, c = 0;
        Coord::coord2D(i, l, c);

        if (i == this->m_pos_player)
        {
            g.drawT(g.getSpritePlayer(this->m_dir_player), c, l);
        }
        else
        {
            //Si c'est un deadlock sans caisse dessus (caisse à la priorité sur le deadlock)
            if(isSquareDead(i) && !isSquareBox(i)) this->m_field[i] = SPRITE_DEADSQUARE;
            g.drawT(g.getSprite(this->m_field[i]), c, l);
        }
    }

}

// Algorithme bruteforce
// pour résoudre un niveau + affichage
void Maze::bruteForce(Graphic& g)
{
    //Temps de resolution
    float temps= clock();
    //Pour commencer l'algo, on reset tout
    reset(elemZero);

    //Position initiale des caisses
    std::vector<unsigned short> c = getPosBoxes();
    //Field initial
    std::vector<unsigned char> f = getField();
    //Position initiale du joueur
    unsigned short p = getPosPlayer();

    this->m_dir.clear();
    this->m_dir.push_back(0);
    bool vic=false, abort=false;
    int taille=0;

    while(!abort && !vic)
    {
        //Remplissage du vecteur des directions
        //Quand la derniere valeur est de 4, on check tout
        if(m_dir.back() == 4)
        {
            for(int i=m_dir.size() ; i>0 ; i--)
            {
                if(m_dir[i]==4)
                {
                    m_dir[i]= 0;
                    m_dir[i-1]++;
                }
                //Quand la premiere valeur passe a 4, on rajoute une case dans le vecteur
                if(m_dir.front()==4)
                {
                    m_dir.front()= 0;
                    m_dir.push_back(0);
                }
            }
        }
        //Mouvements du joueur
        for (unsigned short i=0; i<m_dir.size(); i++)
        {
            vic= updatePlayer(m_dir[i]);
        }
        //Affichage emplacement caisse
        for (unsigned short i=0; i<getPosBoxes().size(); i++)
        {
            std::cout << getPosBoxes()[i] << "   " ;
        }
        //Afficher le vecteur
        for (unsigned short i=0; i<m_dir.size(); i++)
        {
            std::cout << m_dir[i] << " ";
        }
        //Affichage position du joueur
        std::cout << "             "<< getPosPlayer();
        std::cout << std::endl;

        //On reset pas le niveau
        for(unsigned short i=0; i<c.size(); i++)
        {
            setBoxesPos(i, c[i]);
        }
        setPlayerPos(p);
        m_field = f;

        //Compteur
        if(!vic) m_dir.back()++;

        //Stop manuel
        if(g.keyPressed())
        {
            if (g.keyRead()==KEY_BACKSPACE) abort= true;
        }
        taille++;
    }

    if(!abort)
    {
        //Victoire
        std::cout << "Win !" << std::endl;
        //Afficher la taille du vecteur
        std::cout << "Nombre de deplacements : " << m_dir.size() << std::endl ;
        //Affichage du nombre d'éléments uniques
        std::cout << "Nombre d'elements uniques : " << taille << std::endl;
        //Affichage du temps de resolution
        std::cout << "TEMPS : " << (clock() - temps)/1000 << " secondes" << std::endl;

        //Animation
        afficherSolution(g);
    }
    else std::cout << "BRUTEFORCE CANCELLED" << std::endl;

}

// Algorithme Breadth First Search
// pour résoudre un niveau + affichage
void Maze::bfs(Graphic& g)
{
    //Temps pour résoudre
    float temps= clock();
    //Pour commencer l'algo, on reset tout
    reset(elemZero);

    std::vector<Elements> save;
    save.push_back(elemZero);
    //File
    std::queue<Elements> file;
    file.push(save[0]);

    //Deadlocks
    bool dead=false;

    bool vic=false, s=false, abort=false, tmp=false, tmp1=false;
    Elements temp;
    int pred=0;

    while(!file.empty() && !abort)
    {

        //Pour cette boucle on fait des tests sur le dernier de la file
        temp= file.front();
        file.pop();

        //Reset tant qu'on gagne pas
        if(!vic) reset(temp);

        //Tests pour chaque direction
        for (int i=0; i<4; i++)
        {

            //Si on gagne pas, on bouge le player
            if(!vic) vic= updatePlayer(i);

            //Deadlock
            dead= this->isDead;
            //Si une caisse est sur dead, on passe a la suite
            if(!dead)
            {

                if(vic)
                {
                    if(!tmp1)
                    {
                        std::cout << "Win !" << std::endl;
                        tmp1= true;
                    }
                }
                else
                {
                    //Si l'element obtenu est unique, on le met dans le vecteur et file
                    s= true;
                    //Unique ou pas
                    for (unsigned int j=0; j<save.size(); j++)
                    {
                        if (getPosPlayer() == save[j].m_pos_player && (getField() == save[j].m_field)) s= false;
                    }
                    //Si unique, on met dans vecteur et file
                    if (s)
                    {
                        save.push_back(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, i, 0));
                        file.push(save.back());
                        std::cout << pred << " " << i << " " << save.back().m_pos_player << std::endl;
                    }
                }
            }
            //On reset pour tester chaque direction
            reset(temp);

            //On remplit la derniere position dans le vecteur une seule fois apres victoire
            if(vic && !tmp)
            {
                save.push_back(Elements(getPosPlayer(), this->m_field, getPosBoxes(), pred, i, 0));
                tmp= true;
            }

        }
        //L'indice du pred +1
        pred++;
        //Stop manuel
        if(g.keyPressed())
        {
            if (g.keyRead()==KEY_BACKSPACE) abort= true;
        }

    }

    reset(elemZero);

    if(!abort)
    {
        //On affiche le chemin obtenu en fonction des pred
        int j= save.size()-1;
        //Remplissage du vecteur de direction
        while(save[j].m_pred != -1)
        {
            m_dir.push_back(save[j].m_dir);
            j= save[j].m_pred;
        }
        //On inverse le vecteur
        this->m_dir= invert(this->m_dir);
        //Affichage du vecteur inversé
        std::cout << "Vecteur : ";
        for(unsigned int i=0; i<this->m_dir.size(); i++) std::cout << this->m_dir[i] << " ";
        //Affichage du nombre d'éléments uniques
        std::cout << std::endl << "Nombre d'elements uniques : " << save.size() << std::endl;
        //Affichage du nombre de mouvements
        std::cout << "Nombre de mouvements : " << m_dir.size() << std::endl;

        //Affichage du temps de resolution
        std::cout << "TEMPS : " << (clock() - temps)/1000 << " secondes" << std::endl;

        //Animation
        afficherSolution(g);
    }
    else std::cout << "BFS CANCELLED" << std::endl;

}

// Algorithme Depth First Search
// pour résoudre un niveau + affichage
void Maze::dfs(Graphic& g)
{
    //Temps pour résoudre
    float temps= clock();

    std::vector<Elements> save;
    save.push_back(elemZero);
    //Stack
    std::stack<Elements> pile;
    pile.push(save[0]);

    //Deadlocks
    bool dead=false;

    Elements temp;

    bool abort=false, vic=false, s=false;
    unsigned int tmp=0;

    while(!vic && !abort)
    {
        //Pop quand on a pas ajouté d'élément
        if(tmp== pile.size()) pile.pop();
        temp= pile.top();
        reset(temp);

        for(unsigned int i=0; i<4; i++)
        {
            if(!s)
            {
                vic= updatePlayer(i);
                //Deadlock
                dead= this->isDead;
                //Si une caisse est sur dead, on passe a la suite
                if(!dead)
                {
                    if(!vic)
                    {
                        s= true;
                        tmp= pile.size();

                        //On vérifie que l'élément est unique
                        for (unsigned int j=0; j<save.size(); j++)
                        {
                            if ((getPosPlayer() == save[j].m_pos_player) && (getField() == save[j].m_field)) s= false;
                        }
                        //On le met dans save et pile
                        if(s)
                        {
                            save.push_back(Elements(getPosPlayer(), this->m_field, getPosBoxes(), save.size()-1, i, 0));
                            pile.push(save.back());
                        }
                        else reset(temp);


                    }
                    else if(vic)
                    {
                        //Si on gagne, pour ne pas finir la boucle for
                        s= true;
                        pile.push(Elements(getPosPlayer(), this->m_field, getPosBoxes(), save.size()-1, i, 0));
                        reset(elemZero);
                    }

                }
                else reset(temp);
            }
        }
        std::cout << pile.size() << " " << pile.top().m_dir << " " << pile.top().m_pos_player << std::endl;
        s= false;
        //Stop manuel
        if(g.keyPressed())
        {
            if (g.keyRead()==KEY_BACKSPACE) abort= true;
        }
    }

    if(vic)
    {
        std::cout << "Win !" << std::endl;
        //On remplit le vecteur de direction pour l'affichage (on ne prend pas la toute 1ere direction de l'élément zero)
        while(pile.size() != 1)
        {
            this->m_dir.push_back(pile.top().m_dir);
            pile.pop();
        }
        //On inverse le vecteur
        this->m_dir= invert(this->m_dir);
        //Affichage du vecteur inversé
        std::cout << "Vecteur : ";
        for(unsigned int i=0; i<this->m_dir.size(); i++) std::cout << this->m_dir[i] << " " ;
        //Affichage du nombre d'éléments uniques
        std::cout << std::endl << "Nombre d'elements uniques : " << save.size() << std::endl;
        //Affichage du nombre de mouvements
        std::cout << "Nombre de mouvements : " << m_dir.size() << std::endl;
        //Affichage du temps de resolution
        std::cout << "TEMPS : " << (clock() - temps)/1000 << " secondes" << std::endl;

        //Animation
        afficherSolution(g);

    }
    else if(abort) std::cout << "DFS CANCELLED" << std::endl;

}

// Met le field à l'état (noeud) entré
void Maze::reset(Elements e)
{
    setPlayerPos(e.m_pos_player);
    this->m_field= e.m_field;
    this->m_pos_boxes= e.m_pos_boxes;
}

// Affichage de la solution
// A partir du vecteur m_dir du header
// Destruction du vecteur a la fin de l'affichage
void Maze::afficherSolution(Graphic& g)
{
    bool abort= false;
    unsigned short i=0;
    while(i<this->m_dir.size() && !abort)
    {
        //Timer
        int a= clock();
        while(clock() - a < 50) {}
        //On bouge le player
        updatePlayer(m_dir[i]);
        //On affiche
        g.clear();
        draw(g);
        g.display(Coord::m_nb_col);
        i++;
        //Stop manuel
        if(g.keyPressed())
        {
            if (g.keyRead()==KEY_BACKSPACE) abort= true;
        }
    }
    if(abort) std::cout << "AFFICHAGE CANCELLED" << std::endl;
    m_dir.clear();
}

// Remplissage du vecteur avec
// les position des deadlocks
void Maze::deadLock()
{
    unsigned int x=0, y=0;
    bool tmp= false;

    //Coins
    for(unsigned short pos=0; pos<this->getSize(); pos++)
    {
        Coord::coord2D(pos, x, y);
        if (x!=0 && y!=0 && x!=24 && y!=24 && (this->m_field[pos]!= SPRITE_GOAL) && (this->m_field[pos]!= SPRITE_BOX_PLACED) && (this->m_field[pos]!= SPRITE_WALL))
        {
            if (this->m_field[Coord::getDirPos(pos, TOP)] == SPRITE_WALL)
            {
                if ((this->m_field[Coord::getDirPos(pos, LEFT)] == SPRITE_WALL) || (this->m_field[Coord::getDirPos(pos, RIGHT)] == SPRITE_WALL)) tmp= true;
            }
            if (this->m_field[Coord::getDirPos(pos, BOTTOM)] == SPRITE_WALL)
            {
                if ((this->m_field[Coord::getDirPos(pos, LEFT)] == SPRITE_WALL) || (this->m_field[Coord::getDirPos(pos, RIGHT)] == SPRITE_WALL)) tmp= true;
            }
            //Remplissage de la matrice
            if (tmp) this->m_pos_dead.push_back(pos);
        }
        tmp= false;
    }

    //Lignes
    if(this->m_pos_dead.size() > 2)
    {
        unsigned int a=0, b=0, dirTmp1=0 ,dirTmp2=0;
        bool mur=true;

        for(unsigned int i=0; i<this->m_pos_dead.size(); i++)
        {
            for(unsigned int j=i+1; j<this->m_pos_dead.size(); j++)
            {
                //On convertis les coordonnées des coins dead en 2D
                Coord::coord2D(this->m_pos_dead[i], x, y);
                Coord::coord2D(this->m_pos_dead[j], a, b);

                //Si même ligne
                if(x==a)
                {
                    if(y<b)
                    {
                        dirTmp1=2;
                        dirTmp2=3;
                    }
                    else
                    {
                        dirTmp1=3;
                        dirTmp2=2;
                    }
                    //Si les extrémité du mur a chercher sont des wall
                    if(isSquareWall(Coord::getDirPos(this->m_pos_dead[i], dirTmp1)) && isSquareWall(Coord::getDirPos(this->m_pos_dead[j], dirTmp2)))
                    {
                        //Direction HAUT et BAS
                        for(unsigned short k=0; k<2; k++)
                        {
                            mur=true;
                            for(unsigned int l=std::min(y, b); l<=std::max(y, b); l++)
                            {
                                if(mur)
                                {
                                    mur= (isSquareWall(Coord::getDirPos(Coord::coord1D(x, l), k)) && isSquareGround(Coord::coord1D(x,l)));
                                }
                            }
                            if(mur)
                            {
                                for(unsigned int l=std::min(y, b)+1; l<std::max(y, b); l++) this->m_pos_dead.push_back(Coord::coord1D(x, l));
                            }
                        }
                    }
                }
                //Si même colonne
                if(y==b)
                {
                    if(x<a)
                    {
                        dirTmp1=0;
                        dirTmp2=1;
                    }
                    else
                    {
                        dirTmp1=1;
                        dirTmp2=0;
                    }
                    //Si les extrémité du mur a chercher sont des wall
                    if(isSquareWall(Coord::getDirPos(this->m_pos_dead[i], dirTmp1)) && isSquareWall(Coord::getDirPos(this->m_pos_dead[j], dirTmp2)))
                    {
                        //Direction GAUCHE et DROITE
                        for(unsigned short k=2; k<4; k++)
                        {
                            mur=true;
                            for(unsigned int l=std::min(x, a); l<=std::max(x, a); l++)
                            {
                                if(mur)
                                {
                                    mur= (isSquareWall(Coord::getDirPos(Coord::coord1D(l, y), k)) && isSquareGround(Coord::coord1D(l, y)));
                                }
                            }
                            if(mur)
                            {
                                for(unsigned int l=std::min(x, a)+1; l<std::max(x, a); l++) this->m_pos_dead.push_back(Coord::coord1D(l, y));
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << "Emplacement Deadlocks : ";
    for (unsigned short i=0; i<this->m_pos_dead.size(); i++)
    {
        std::cout << this->m_pos_dead[i] << " ";
    }
    std::cout << std::endl;
}

// DO NOT TOUCH !
// Overload function for displaying debug information
// about Maze class
std::ostream& operator << (std::ostream& O, const Maze& m)
{
    unsigned int l, c;
    int i = 0;
    Coord::coord2D(m.m_pos_player, l, c);
    O << "Player position " << m.m_pos_player << " (" << l << "," << c << ")" << std::endl;
    O << "Field Size " << +m.m_lig << " x " << +m.m_col << " = " << m.getSize() << std::endl;
    O << "Field Vector capacity : " << m.m_field.capacity() << std::endl;
    O << "Field array : " << std::endl << std::endl;
    for(unsigned int l=0; l<m.getSize(); l++)
    {
        if (l == m.m_pos_player) Console::getInstance()->setColor(_COLOR_YELLOW);
        else if (m.isSquareWall(l)) Console::getInstance()->setColor(_COLOR_PURPLE);
        else if (m.isSquareBoxPlaced(l) || m.isSquareGoal(l)) Console::getInstance()->setColor(_COLOR_GREEN);
        else if (m.isSquareBox(l)) Console::getInstance()->setColor(_COLOR_BLUE);
        else if (m.m_field[l] == SPRITE_DEADSQUARE) Console::getInstance()->setColor(_COLOR_RED);
        else Console::getInstance()->setColor(_COLOR_WHITE);

        O << std::setw(2) << +m.m_field[l] << " "; // + => print as "int"

        if ((l+1) % m.m_col == 0)
        {
            O << std::endl;
        }
    }
    Console::getInstance()->setColor(_COLOR_DEFAULT);

    O << std::endl;
    O << "Box position : " << std::endl;
    for (unsigned int i=0; i<m.m_pos_boxes.size(); i++)
    {
        Coord::coord2D(m.m_pos_boxes[i], l, c);
        O << "\t" << "Box #" << i << " => " << std::setw(3) << m.m_pos_boxes[i] << std::setw(2) << " (" << l << "," << c << ")" << std::endl;
    }

    O << std::endl;
    O << "Goal position : " << std::endl;
    for (const auto& goal : m.m_pos_goals)
    {
        unsigned int l, c;
        Coord::coord2D(goal, l, c);
        if (m.isSquareBoxPlaced(goal)) Console::getInstance()->setColor(_COLOR_GREEN);
        O << "\t" << "Goal #" << i << " => " << std::setw(3) << goal << std::setw(2) << " (" << l << "," << c << ")" << std::endl;
        if (m.isSquareBoxPlaced(goal)) Console::getInstance()->setColor(_COLOR_DEFAULT);
        i++;
    }

    return O;
}

