/*
* ann_map.c
*
* Created: 7/22/2015 11:16:17 AM
*  Author: jsigrist
*/

#include "ann_map.h"

uint8_t to_ann(uint8_t code)
{
  switch (code)
  {
    case NORMAL:
        return 'N';
    case PVC:
        return 'V';
    default:
        return 'Q';
  }
  return 'Q';
}

uint8_t from_ann(uint8_t label)
{
  switch (label)
  {
    case 'N':
        return NORMAL;
    case 'V':
        return PVC;
    default:
        return UNKNOWN;
  }
  return UNKNOWN;
}


