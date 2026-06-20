#ifndef OCEAN_FLOOR_H
#define OCEAN_FLOOR_H

#include <cmath>

// Zwraca wysokość dna (Y) dla pozycji x,z
float oceanFloorHeight(float x, float z);

// Rysuje podłogę/ocean floor (wywoływane z main)
void generate_ocean_floor();

#endif // OCEAN_FLOOR_H
