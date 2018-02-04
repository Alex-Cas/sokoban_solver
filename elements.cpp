#include "elements.h"

Elements::Elements(){
}

Elements::Elements(unsigned short pos_player,
                   std::vector<unsigned char>field,
                   std::vector<unsigned short>pos_boxes,
                   int pred, int dir, int poids)
{
    m_pos_player = pos_player;
    m_field = field;
    m_pos_boxes = pos_boxes;
    m_pred = pred;
    m_dir = dir;
    m_poids = poids;
}

Elements::~Elements(){
}
