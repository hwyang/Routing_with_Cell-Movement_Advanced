/*****************************/
/*  User-Defined Parameters  */
/*****************************/
#define FLUTE_ROUTING 1 // 1 to construct routing, 0 to estimate WL only
#define FLUTE_LOCAL_REFINEMENT 1     // Suggestion: Set to 1 if ACCURACY >= 5
#define FLUTE_REMOVE_DUPLICATE_PIN 0 // Remove dup. pin for flute_wl() & flute()

#define FLUTE_POWVFILE "POWV9.dat" // LUT for POWV (Wirelength Vector)
#define FLUTE_POSTFILE "POST9.dat" // LUT for POST (Steiner Tree)
#define FLUTE_D 9                  // LUT is used for d <= FLUTE_D, FLUTE_D <= 9