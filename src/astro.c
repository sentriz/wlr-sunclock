#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265358979323846
#define E 2.7182818284590452354
#define EPSILON 1E-6

#define abs(x) ((x) < 0 ? (-(x)) : x)                  /* absolute value */
#define sgn(x) (((x) < 0) ? -1 : ((x) > 0 ? 1 : 0))    /* extract sign */
#define dtr(x) ((x) * (PI / 180.0))                    /* degree -> radian */
#define rtd(x) ((x) / (PI / 180.0))                    /* radian -> degree */
#define fixangle(a) ((a)-360.0 * (floor((a) / 360.0))) /* fix angle */

#define TERMINC 100       /* circle segments for terminator */
#define PROJINT (60 * 10) /* frequency of seasonal recalculation */

/* convert internal GMT date and time to Julian day
   and fraction */
double astro_gm_time_to_julian(struct tm* t) {
    double y = t->tm_year + 1900;
    double m = t->tm_mon + 1;
    if (m > 2)
        m = m - 3;
    else {
        m = m + 9;
        y--;
    }
    /* compute century */
    double c = y / 100L;
    y -= 100L * c;
    return t->tm_mday + (c * 146097L) / 4 + (y * 1461L) / 4 +
           (m * 153L + 2) / 5 + 1721119L;
}

/* convert internal GMT  date  and     time  to  astronomical
   Julian  time  (i.e.   Julian  date  plus  day fraction,
   expressed as a double) */
long astro_gm_time_to_julian_astro(struct tm* t) {
    return (astro_gm_time_to_julian(t) - 0.5) +
           (((long)t->tm_sec) + 60L * (t->tm_min + 60L * t->tm_hour)) / 86400.0;
}

double astro_kepler(double m, double ecc) {
    double delta;
    double e = m = dtr(m);
    do {
        delta = e - ecc * sin(e) - m;
        e -= delta / (1 - ecc * cos(e));
    } while (abs(delta) > EPSILON);
    return e;
}

/*  calculate position of the Sun.	JD is the Julian  date
    of  the  instant for which the position is desired and
    APPARENT should be nonzero if  the  apparent  position
    (corrected  for  nutation  and aberration) is desired.
    The Sun's co-ordinates are returned  in  RA  and  DEC,
    both  specified  in degrees (divide RA by 15 to obtain
    hours).  The radius vector to the Sun in  astronomical
    units  is returned in RV and the Sun's longitude (true
    or apparent, as desired) is  returned  as  degrees  in
    SLONG */
void astro_sun_position(double jd, int apparent, double* ra, double* dec,
                        double* rv, double* slong) {
    /* time in Julian centuries of 36525 ephemeris days,
       measured from the epoch 1900 January 0.5 ET */
    double t = (jd - 2415020.0) / 36525.0;
    double t2 = t * t;
    double t3 = t2 * t;

    /* geometric mean longitude of the Sun, referred to the
       mean equinox of the date */
    double l = fixangle(279.69668 + 36000.76892 * t + 0.0003025 * t2);

    /* sun's mean anomaly */
    double m =
        fixangle(358.47583 + 35999.04975 * t - 0.000150 * t2 - 0.0000033 * t3);

    /* eccentricity of the Earth's orbit */
    double e = 0.01675104 - 0.0000418 * t - 0.000000126 * t2;

    /* eccentric anomaly */
    double ea = astro_kepler(m, e);

    /* true anomaly */
    double v = fixangle(2 * rtd(atan(sqrt((1 + e) / (1 - e)) * tan(ea / 2))));

    /* sun's true longitude */
    double theta = l + v - m;

    /* obliquity of the ecliptic. */
    double eps = 23.452294 - 0.0130125 * t - 0.00000164 * t2 + 0.000000503 * t3;

    /* corrections for Sun's apparent longitude, if desired */
    if (apparent) {
        double omega = fixangle(259.18 - 1934.142 * t);
        theta = theta - 0.00569 - 0.00479 * sin(dtr(omega));
        eps += 0.00256 * cos(dtr(omega));
    }

    /* return Sun's longitude and radius vector */
    *slong = theta;
    *rv = (1.0000002 * (1 - e * e)) / (1 + e * cos(dtr(v)));

    /* determine solar co-ordinates */
    *ra =
        fixangle(rtd(atan2(cos(dtr(eps)) * sin(dtr(theta)), cos(dtr(theta)))));
    *dec = rtd(asin(sin(dtr(eps)) * sin(dtr(theta))));
}

/* greenwich Mean Siderial Time for a given
   instant expressed as a Julian date and fraction */
double astro_gm_siderial_time(double jd) {
    /* time, in Julian centuries of 36525 ephemeris days,
       measured from the epoch 1900 January 0.5 ET */
    double t = ((floor(jd + 0.5) - 0.5) - 2415020.0) / 36525.0;
    double theta0 = 6.6460656 + 2400.051262 * t + 0.00002581 * t * t;

    t = (jd + 0.5) - (floor(jd + 0.5));
    theta0 += (t * 24.0) * 1.002737908;
    theta0 = (theta0 - 24.0 * (floor(theta0 / 24.0)));
    return theta0;
}

/* project illuminated area on the map */
void astro_project_illum(short* wtab, int xdots, int ydots, double dec) {
    int i, ftf = 1, ilon, ilat, lilon = 0, lilat = 0, xt;
    double m, x, y, z, th, lon, lat;

    /* clear unoccupied cells in width table */
    for (i = 0; i < ydots; i++)
        wtab[i] = -1;

    /* build transformation for declination */
    double s = sin(-dtr(dec));
    double c = cos(-dtr(dec));

    /* increment over a semicircle of illumination */
    for (th = -(PI / 2); th <= PI / 2 + 0.001; th += PI / TERMINC) {
        /* transform the point through the declination rotation */
        x = -s * sin(th);
        y = cos(th);
        z = c * sin(th);

        /* transform the resulting co-ordinate through the
           map projection to obtain screen co-ordinates */
        lon = (y == 0 && x == 0) ? 0.0 : rtd(atan2(y, x));
        lat = rtd(asin(z));

        ilat = ydots - (lat + 90) * (ydots / 180.0);
        ilon = lon * (xdots / 360.0);

        if (ftf) {
            /* first time.  just save start co-ordinate */
            lilon = ilon;
            lilat = ilat;
            ftf = 0;
            continue;
        }

        /* trace out the line and set the width table */
        if (lilat == ilat) {
            wtab[(ydots - 1) - ilat] = ilon == 0 ? 1 : ilon;
        } else {
            m = ((double)(ilon - lilon)) / (ilat - lilat);
            for (i = lilat; i != ilat; i += sgn(ilat - lilat)) {
                xt = lilon + floor((m * (i - lilat)) + 0.5);
                wtab[(ydots - 1) - i] = xt == 0 ? 1 : xt;
            }
        }
        lilon = ilon;
        lilat = ilat;
    }

    /* now tweak the widths to generate full illumination for the correct pole
     */
    if (dec < 0.0) {
        ilat = ydots - 1;
        lilat = -1;
    } else {
        ilat = 0;
        lilat = 1;
    }

    for (i = ilat; i != ydots / 2; i += lilat) {
        if (wtab[i] != -1) {
            while (1) {
                wtab[i] = xdots / 2;
                if (i == ilat)
                    break;
                i -= lilat;
            }
            break;
        }
    }
}
