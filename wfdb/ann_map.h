/*
 * ann_map.h
 *
 * Created: 7/22/2015 11:16:26 AM
 *  Author: jsigrist
 */ 


#ifndef ANN_MAP_H
#define ANN_MAP_H

#include <stdint.h>
#include "ecgcodes.h"

// Converts from an integer code to the ascii code.
uint8_t to_ann(uint8_t code);

// Converts from an ascii label to an integer code.
uint8_t from_ann(uint8_t label);


#endif /* ANN_MAP_H */