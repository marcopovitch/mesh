#include "const.h"

/*--------*/

/* modulo */

/*--------*/
float modulo(float num, int mod)
{
    float result;

    int tmp;

    tmp = (int) (num / (float) mod);
    result = num - (float) (tmp * mod);
    if (result < 0)
        result += mod;
    return (result);
}

/*-------------------------------------------------*/

/* modulo : if type = 0 return 0   if num = 0||360 */

/*          if type = 1 return 360 if num = 0||360 */

/*          else return the modulo                 */

/*-------------------------------------------------*/
float modulo360(float num, int type)
{
    float res;

    res = modulo(num, 360);
    if (res > -EPS && res < EPS) {
        switch (type) {
        case 0:
            return (0.);
        default:
            return (360.);
        }
    }
    return (res);
}
