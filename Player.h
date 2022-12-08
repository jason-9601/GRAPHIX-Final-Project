// The Player ship can be controlled using WASDQE 

//  W / S – Forward / Back (z)
//  A / D – Turn Left / Right (x)
//  Q / E – Ascend / Descend (y -- check note below abt sub and y-axis)

// Sub can only be controlled when in 1st / 3rd Person view
// Sub cannot go above 0 in the Y axis
// Print out the current depth the sub is in the console window using cout

// DUMPING OF IDEAS 
// key movement control -- WASDQE CODITIONAL IF 1ST/3RD PERSON ON MAIN MODEL OBJ
// (note: sub cannot go above 0 in Y Axis): start position y-axis negative 
// -- check other classes to ensure no overlap ++ make either (1) condition that if y-axis reach 0 not move OR (2) make world/cam extend only up to y-axis 0 ??? 2x check this

// PRINT CURRENT COORD -- Search for similar concept i found on youtube before regardin