#ifndef SUNCLOCK_ASTRO_H
#define SUNCLOCK_ASTRO_H

#include <time.h>

typedef struct {
    double lon;  // longitude in degrees
    double lat;  // latitude in degrees
} AstroPoint;

double astro_gm_time_to_julian(struct tm* t);
long astro_gm_time_to_julian_astro(struct tm* t);
double astro_kepler(double m, double ecc);
void astro_sun_position(double jd, int apparent, double* ra, double* dec,
                        double* rv, double* slong);
int astro_terminator_points(AstroPoint* points, int max_points, double dec);

#endif
