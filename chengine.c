#include <stdio.h>

// Define bitboard data type
#define U64 unsigned long long

// Set/get/pop macros
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? bitboard ^= (1ULL << square) : 0)

// Count bits within a bitboard
static inline int count_bits(U64 bitboard) {
    // Bit counter
    int count = 0;

    // Consecutively reset least significant 1st bit
    while (bitboard) {
        // Increment count
        count++;

        // Reset least significant 1st bit
        bitboard &= bitboard - 1;
    }

    // Return the count
    return count;
}

// Get least significant 1st bit index
static inline int get_ls1b_index(U64 bitboard) {
    // Make sure bitboard is not 0
    if (bitboard) {
        // Count trailing bits before LS1B
        return count_bits((bitboard & -bitboard) - 1);
    }
    else {
        // Return illegal index
        return -1;
    }
}

// Board squares
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1
};

// Side to move (colors)
enum { white, black };

// Bishop and rook
enum { rook, bishop };

const char *square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

/* ================================================================================ */
/* ============================== Random numbers ================================== */
/* ================================================================================ */

// Pseudo random number state
unsigned int state = 1804289383;

// Generate 32-bit pseudo legal numbers
unsigned int get_random_U32_number() {
    // Get current state
    unsigned int number = state;

    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    // Update random number state
    state = number;

    // Return random number
    return number;
}

// Generate 64-bit pseudo legal numbers
U64 get_random_U64_number() {
    // Define 4 random numbers
    U64 n1, n2, n3, n4;

    // Init random numbers
    n1 = (U64)(get_random_U32_number()) & 0xFFFF;
    n2 = (U64)(get_random_U32_number()) & 0xFFFF;
    n3 = (U64)(get_random_U32_number()) & 0xFFFF;
    n4 = (U64)(get_random_U32_number()) & 0xFFFF;

    // Return nranodm number
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// Generate magic number candidate
U64 generate_magic_number() {
    return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}

// Function: print bibtboard
void print_bitboard(U64 bitboard) {
    // Loop over board ranks
    for (int rank = 0; rank < 8; rank++) {
        // Loop over board files
        for (int file = 0; file < 8; file++) {
            // Convert file and rank into square index
            int square = rank * 8 + file;

            // Print ranks
            if (!file)
                printf("  %d ", 8 - rank);

            // Print bit state (either 1 or 0)
            printf(" %d ", get_bit(bitboard, square) ? 1 : 0);

        }

        // Print new line every rank
        printf("\n");
    }

    // Print board files
    printf("\n     a  b  c  d  e  f  g  h\n\n");

    // Print bitboard as unsigned decimal number
    printf("     Bitboard: %llud\n\n", bitboard);
}

/* ========================================================================= */
/* ============================== Attacks ================================== */
/* ========================================================================= */

// Not A file constant
const U64 not_A_file = 18374403900871474942ULL;

// Not H file constant
const U64 not_H_file = 9187201950435737471ULL;

// Not HG file constant
const U64 not_HG_file = 4557430888798830399ULL;

// Not AB file constant
const U64 not_AB_file = 18229723555195321596ULL;

// Bishop relevant occupancy bit count for every square on board
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

// Rook relevant occupancy bit count for every square on board
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

// Rook magic numbers
U64 rook_magic_numbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};

// Bishop magic numbers
U64 bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};

// Pawn attacks table [side][square]
U64 pawn_attacks[2][64];

// Knight attacks table [square]
U64 knight_attacks[64];

// King attacks table [square]
U64 king_attacks[64];

// Generate pawn attacks
U64 mask_pawn_attacks(int side, int square) {
    // Result attacks bitboard
    U64 attacks = 0ULL;

    // Piece bitboard
    U64 bitboard = 0ULL;

    // Set piece on board
    set_bit(bitboard, square);

    // White pawns
    if (!side) {
        // Generate pawn attacks for white
        if ((bitboard >> 7) & not_A_file) attacks |= (bitboard >> 7);
        if ((bitboard >> 9 ) & not_H_file) attacks |= (bitboard >> 9);
    } else {
        // Generate pawn attacks for black
        if ((bitboard << 7) & not_H_file) attacks |= (bitboard << 7);
        if ((bitboard << 9 ) & not_A_file) attacks |= (bitboard << 9);
    }

    return attacks;
}

// Generate knights attacks
U64 mask_knight_attacks (int square) {
    // Result attacks bitboard
    U64 attacks = 0ULL;

    // Piece bitboard
    U64 bitboard = 0ULL;

    // Set piece on board
    set_bit(bitboard, square);

    // Generate knight attacks
    if((bitboard >> 17) & not_H_file) attacks |= (bitboard >> 17);
    if((bitboard >> 15) & not_A_file) attacks |= (bitboard >> 15);
    if((bitboard >> 10) & not_HG_file) attacks |= (bitboard >> 10);
    if((bitboard >> 6) & not_AB_file) attacks |= (bitboard >> 6);
    if((bitboard << 17) & not_A_file) attacks |= (bitboard << 17);
    if((bitboard << 15) & not_H_file) attacks |= (bitboard << 15);
    if((bitboard << 10) & not_AB_file) attacks |= (bitboard << 10);
    if((bitboard << 6) & not_HG_file) attacks |= (bitboard << 6);

    // Return attack map
    return attacks;
}

// Generate king attacks
U64 mask_king_attacks (int square) {
    // Result attacks bitboard
    U64 attacks = 0ULL;

    // Piece bitboard
    U64 bitboard = 0ULL;

    // Set piece on board
    set_bit(bitboard, square);

    // Generate king attacks
    if (bitboard >> 8) attacks |= (bitboard >> 8);
    if ((bitboard >> 9) & not_H_file) attacks |= (bitboard >> 9);
    if ((bitboard >> 7) & not_A_file) attacks |= (bitboard >> 7);
    if ((bitboard >> 1) & not_H_file) attacks |= (bitboard >> 1);

    if (bitboard << 8) attacks |= (bitboard << 8);
    if ((bitboard << 9) & not_A_file) attacks |= (bitboard << 9);
    if ((bitboard << 7) & not_H_file) attacks |= (bitboard << 7);
    if ((bitboard << 1) & not_A_file) attacks |= (bitboard << 1);

    // Return attack map
    return attacks;
}

// Mask bishop attacks
U64 mask_bishop_attacks(int square) {
    // Result attacks bitboard
    U64 attacks = 0ULL;

    // Init ranks & files
    int r, f;

    // Init target rank & files
    int tr = square / 8;
    int tf = square % 8;

    // Mask relevant bishop occupancy squares
    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));


    return attacks;
}

// Mask rook attacks
U64 mask_rook_attacks(int square) {
    // Result attacks bitboard
    U64 attacks = 0ULL;

    // Init ranks & files
    int r, f;

    // Init target rank & files
    int tr = square / 8;
    int tf = square % 8;

    // Mask relevant rook occupancy squares
    for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
    for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
    for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
    for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));

    return attacks;
}

// Generate bishop attacks on the fly
U64 bishop_attacks_on_the_fly(int square, U64 block) {
    // Result attacks bitboard
    U64 attacks = 0ULL;

    // Init ranks & files
    int r, f;

    // Init target rank & files
    int tr = square / 8;
    int tf = square % 8;

    // Generate bishop attacks
    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }

    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }

    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }

    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }


    return attacks;
}

// Generate rook attacks on the fly
U64 rook_attacks_on_the_fly(int square, U64 block) {
    // Result attacks bitboard
    U64 attacks = 0ULL;

    // Init ranks & files
    int r, f;

    // Init target rank & files
    int tr = square / 8;
    int tf = square % 8;

    // Generate rook attacks
    for (r = tr + 1; r <= 7; r++) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }

    for (r = tr - 1; r >= 0; r--) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }

    for (f = tf + 1; f <= 7; f++) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }

    for (f = tf - 1; f >= 0; f--) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }

    // Return attack map
    return attacks;
}

// Init leaper pieces attacks
void init_leapers_attacks() {
    // Loop over 64 board squares
    for (int square = 0; square < 64; square++) {
        // Init pawn attacks
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);

        // Init knight attacks
        knight_attacks[square] = mask_knight_attacks(square);

        // Init king attacks
        king_attacks[square] = mask_king_attacks(square);
    }
}

// Set occipancies
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
    // Occupancy map
    U64 occupancy = 0ULL;

    // Loop over the range of bits within attack mask
    for (int count = 0; count < bits_in_mask; count++) {
        // Get LS1B index of the attacks mask
        int square = get_ls1b_index(attack_mask);

        // Pop LS1B in attack map
        pop_bit(attack_mask, square);

        // Make sure occupancy is on board
        if (index & (1 << count))
            // Populate occupancy map
            occupancy |= (1ULL << square);
    }

    // Return occupancy
    return occupancy;
}

/* ======================================================================== */
/* ============================== Magics ================================== */
/* ======================================================================== */

// Find appropiate magic number
U64 find_magic_number(int square, int relevant_bits, U64 bishop) {
    // Init occupancies
    U64 occupancies[4096];

    // Init attack tables
    U64 attacks[4096];

    // Init used attacks
    U64 used_attacks[4096];

    // Init attack mask for a current piece
    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

    // Init occupancy indices
    int occupancy_indices = 1 << relevant_bits;

    // Loop over occupancy indices
    for (int index = 0; index < occupancy_indices; index++) {
        // Init occupancies
        occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);

        // Init attacks
        attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies[index]) :
                                    rook_attacks_on_the_fly(square, occupancies[index]);
    }

    // Test magic numbers loop
    for (int random_count = 0; random_count < 1000000000; random_count++) {
        // Generate magic number candidate
        U64 magic_number = generate_magic_number();

        // Skip inappropiate magic numbers
        if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;

        // Init used attacks
        memset(used_attacks, 0ULL, sizeof(used_attacks));

        // Init index and fail flag
        int index, fail;
        
        // Test magic index loop
        for (index = 0, fail = 0; !fail && index < occupancy_indices; index++) {
            // Init magic index
            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));

            // If magic index works
            if (used_attacks[magic_index] == 0ULL) {
                // Init used attacks
                used_attacks[magic_index] = attacks[index];
            } else if (used_attacks[magic_index] != attacks[index]) {
                // Magic index doesn't work
                fail = 1;        
            }
        }

        // If magic number works
        if (!fail) {
            // Return it
            return magic_number;
        }

    }
    // If magic number doesn't work
    printf(" Magic number fails!");
    return 0ULL;
}

// Init magic numbers
void init_magic_numbers() {
    // Loop for 64 board squares
    for (int square = 0; square < 64; square++) {
        // Init rook magic numbers
        rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);
    }

    // Loop for 64 board squares
    for (int square = 0; square < 64; square++) {
        // Init bishop magic numbers
        bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);

    }
}


/* ========================================================================== */
/* ============================== Init all ================================== */
/* ========================================================================== */

// Init all variables
void init_all() {
    // Init magic numbers
    //init_magic_numbers();

    // Init leaper pieces attacks
    init_leapers_attacks();

}


/* ====================================================================== */
/* ============================== Main ================================== */
/* ====================================================================== */

int main() {
    // Init all
    init_all();
    

    return 0;
}