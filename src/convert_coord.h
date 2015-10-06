#ifndef _CONVERT_H_
#define _CONVERT_H_

#ifndef _COORD_3D_H_
#define _COORD_3D_H_
struct point3d_t {

    float lat;           /**< latitude */

    float lon;           /**< longitude */

    float prf;           /**< depth */
};
#endif

#ifndef _COORD_GEO_H_
#define _COORD_GEO_H_
struct coord_geo_t {

    double lat;    /**< latitude   */

    double lon;    /**< longitude (in rad or degree ?) */

    double prof;   /**< depth (in km)*/
};
#endif

#ifndef _COORD_CART_H_
#define _COORD_CART_H_
struct coord_cart_t {

    double x;      /**< plan xy axe equateur */

    double y;      /**< plan xz axe meridien origine */

    double z;      /**< depth (in kms) */
};
#endif

struct coord_cart_t *point3d2cart(const struct point3d_t *point3d);

struct coord_geo_t *point3d2geo(struct point3d_t *p);

#endif
