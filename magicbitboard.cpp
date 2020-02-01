#include "magicbitboard.h"


Magic RookMagics[64];
Magic BishopMagics[64];

void init_magics(INT64U table[], Magic magics[], Direction directions[])
{
    // optimal pseudo random generator seeds to pick the correct magics in the shortest time, rank count -> 8
    int seeds[8] = { 728, 10316, 55013, 32803, 12281, 15100, 16645, 255 };

    INT64U occupancy[4096];
    INT64U reference[4096];
    INT64U edges;
    INT64U b;

    int epoch[4096] = {};
    int count = 0;
    int size = 0;

    for (INT8U square = 0; square < 64; ++square)
    {
        // Board edges are not considered in te relevant occupancies
        edges = ((Rank1 | Rank8) & ~RankMask[RANK(square)]) | ((FileA | FileH) & ~FileMask[FILE(square)]);

        /*
        * Given a square 's', the mask is the bitboard (the bits) of sliding attacks
        * from 's' computed on empty board. The index must be big enough to contain
        * all the attacks for each possible subset of the mask and so is 2 power
        * the number of 1s of mask (a maskban az 1-esek szama a 2 hatvanyanak kell lennie).
        * Hence we deduce the size of shift to apply to the 64 or 32 bits word to get the index
        */
        Magic& magic = magics[square];
        magic.Mask = sliding_attack(directions, square, 0) & ~edges;
        magic.Shift = 64 - POP_COUNT(magic.Mask);

        // Set the offset for the attack table of the square. We have individual
        // table sizes for each square with "Fancy Magic BitBoards".
        magic.Attacks = square == 0 ? table : magics[square - 1].Attacks + size;

        /*
         * Use Carry-Rippler trick to enumarate all subsets of mask[s] and 
         * store the corresponding sliding attack bitboard (bits) in reference[]
         * ez a do-while adja, hogy az osszes lehestseges tabla allast legeneralja,
         * persze ehhez csak azokat az allasokat hasznalja fel, ami tenylegesen szamit egy adott
         * pozicioban levo pld bishop eseten. Ez adja a lenyeget a magic bitboardnak:
         * egy adott pozioban levo atlos tamado max 12 mezot tamadhat, a tobbi mezo irrelevans 
         * a tamadasai szemszogebol. Pld a rook a1-en 12 mezot tamadhat, a szelek nem szamitanak.
         * A 12 pozicio kominaciot eleg figyelni, es ezt hash-eli el a magic bitboard.
         * Tehat a magic bitboard az Attacks To - t adja meg egy poziohoz. 
         * A tamadasok legeneralasaban itt a "b" szamitasa adja a trukkot,
         * ezzel lehet kiszamitani az osszes tamadast (occupancy-t, tabla lefedettseget, 
         * ami relevans a tamadast tekintve az adott poziciobol): b = (b - magic.Mask) & magic.Mask
         */
        b = 0;
        size = 0;

        do
        {
            occupancy[size] = b;
            reference[size] = sliding_attack(directions, square, b);

            size++;
            b = (b - magic.Mask) & magic.Mask;
        } while (b);

        PseudoRandomGenerator randomGenerator(seeds[RANK(square)]);

        /*
         * Find a magic for square picking up an (almost) random number
         * until we find the one taht passes the verification test
         */
        for (int i = 0; i < size;)
        {
            for (magic.MagicNumber = 0; POP_COUNT((magic.MagicNumber * magic.Mask) >> 56) < 6; )
            {
                magic.MagicNumber = randomGenerator.sparse_rand<INT64U>();
            }

            /*
                * A good magic must map every possiple occupancy to an index that
                * looks up the correct sliding attack in the atack[s] darabase.
                * Note that we build up the datebase for the square as a side 
                * effect of verifying the magic. Keep track of the attempt count
                * and save it in epoch[], little speed-up trick to avoid resetting
                * magic.Attacks[] after every failed attempt.
                */
            for (++count, i = 0; i < size; ++i)
            {
                UINT index = magic.index(occupancy[i]);

                if (epoch[index] < count)
                {
                    epoch[index] = count;
                    magic.Attacks[index] = reference[i];
                }
                else if (magic.Attacks[index] != reference[i])
                {
                    break;
                }
            }
        }
    }
}
