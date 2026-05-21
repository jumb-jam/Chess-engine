#include "book.h"

static void playMove(
    Board& b,
    int fr,
    int fc,
    int tr,
    int tc)
{
    Move m;

    m.fromRow=fr;
    m.fromCol=fc;

    m.toRow=tr;
    m.toCol=tc;

    Undo u;

    b.make_move(m,u);
}

void OpeningBook::init(){

    //
    // Starting position
    //

    Board start;
    start.init_board();

    //
    // First move choices
    //

    add(start.get_hash(),6,4,4,4); // e4
    add(start.get_hash(),6,3,4,3); // d4

    //
    // e4 branch
    //

    {
        Board b=start;

        playMove(b,6,4,4,4); // e4

        add(b.get_hash(),1,4,3,4); // e5
        add(b.get_hash(),1,2,3,2); // c5 Sicilian
        add(b.get_hash(),1,4,2,4); // e6 French
        add(b.get_hash(),1,2,2,2); // c6 Caro-Kann

        //
        // e4 e5
        //

        Board e4e5=b;

        playMove(e4e5,1,4,3,4);

        add(e4e5.get_hash(),7,6,5,5); // Nf3

        //
        // e4 e5 Nf3
        //

        Board nf3=e4e5;

        playMove(nf3,7,6,5,5);

        add(nf3.get_hash(),0,1,2,2); // Nc6

        //
        // Italian
        //

        Board italian=nf3;

        playMove(italian,0,1,2,2);

        add(italian.get_hash(),7,5,4,2); // Bc4

        Board bc4=italian;

        playMove(bc4,7,5,4,2);

        add(bc4.get_hash(),0,5,3,2); // Bc5

        Board bc5=bc4;

        playMove(bc5,0,5,3,2);

        add(bc5.get_hash(),6,3,5,3); // d3
        add(bc5.get_hash(),7,4,7,6); // O-O

        //
        // Ruy Lopez
        //

        Board ruy=nf3;

        playMove(ruy,0,1,2,2);

        add(ruy.get_hash(),7,5,3,1); // Bb5

        Board bb5=ruy;

        playMove(bb5,7,5,3,1);

        add(bb5.get_hash(),0,6,2,5); // Nf6 Berlin
    }

    //
    // d4 branch (London/KID)
    //

    {
        Board b=start;

        playMove(b,6,3,4,3); // d4

        add(b.get_hash(),1,3,3,3); // d5
        add(b.get_hash(),0,6,2,5); // Nf6

        //
        // London
        //

        Board london=b;

        playMove(london,1,3,3,3);

        add(london.get_hash(),7,6,5,5); // Nf3

        Board nf3=london;

        playMove(nf3,7,6,5,5);

        add(nf3.get_hash(),0,1,2,2); // Nc6

        Board nc6=nf3;

        playMove(nc6,0,1,2,2);

        add(nc6.get_hash(),7,5,6,4); // Bf4

        Board bf4=nc6;

        playMove(bf4,7,5,6,4);

        add(bf4.get_hash(),1,4,2,4); // e6
    }

    //
    // Sicilian
    //

    {
        Board b=start;

        playMove(b,6,4,4,4); // e4
        playMove(b,1,2,3,2); // c5

        add(b.get_hash(),7,6,5,5); // Nf3

        Board nf3=b;

        playMove(nf3,7,6,5,5);

        add(nf3.get_hash(),1,3,2,3); // d6

        Board d6=nf3;

        playMove(d6,1,3,2,3);

        add(d6.get_hash(),6,3,4,3); // d4
    }

    //
    // King's Indian
    //

    {
        Board b=start;

        playMove(b,6,3,4,3); // d4
        playMove(b,0,6,2,5); // Nf6

        add(b.get_hash(),6,2,4,2); // c4

        Board c4=b;

        playMove(c4,6,2,4,2);

        add(c4.get_hash(),1,6,3,6); // g6

        Board g6=c4;

        playMove(g6,1,6,3,6);

        add(g6.get_hash(),7,1,5,2); // Nc3
    }

}