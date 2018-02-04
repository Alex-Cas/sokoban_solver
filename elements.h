#include <vector>
#include <iostream>


class Elements
{
    public:
        unsigned short m_pos_player;
        std::vector<unsigned char> m_field; // field
        std::vector<unsigned short> m_pos_boxes; // box positions
        int m_pred;
        int m_dir;
        int m_poids;

        Elements();
        Elements(unsigned short pos_player,
                   std::vector<unsigned char>field,
                   std::vector<unsigned short>pos_boxes,
                   int pred, int dir, int poids);
        ~Elements();

};
