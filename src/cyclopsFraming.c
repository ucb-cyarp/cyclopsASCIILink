//
// Created by Christopher Yarp on 10/15/19.
//

#include "cyclopsFraming.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

const uint8_t cyclopsPreambleSymbols[] =
        {1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
         0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0,
         1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0};

int createRawPreamble(TX_SYMBOL_DATATYPE *rawPreambleBuf, TX_MODTYPE_DATATYPE *preambleModulationBuf){
    static_assert(sizeof(TX_SYMBOL_DATATYPE)*8 >= MAX_BITS_PER_SYMBOL, "MAX_BITS_PER_SYMBOL must fit within TX_SYMBOL_DATATYPE");

    for(int i = 0; i<PREAMBLE_SYMBOL_LEN; i++){
        //Each symbol is placed in a TX_SYMBOL_DATATYPE unit TX_REPITIONS_PER_SYMBOL times
        for(int j = 0; j<TX_REPITIONS_PER_SYMBOL; j++){
            int ind = i*TX_REPITIONS_PER_SYMBOL+j;
            rawPreambleBuf[ind] = cyclopsPreambleSymbols[i];
            preambleModulationBuf[ind] = MOD_TYPE_BPSK; //The Preamble is BPSK
        }
    }

    return PREAMBLE_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL;
}

int createRawHeader(TX_SYMBOL_DATATYPE *rawHeaderBuf, TX_MODTYPE_DATATYPE *headerModulationBuf, uint8_t modType, uint8_t type, uint8_t src, uint8_t dst, uint16_t netID, uint16_t length){
    int headerInd = 0;

    //Mod Type
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, modType, sizeof(modType)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    //Type
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, type, sizeof(type)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    //Src
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, src, sizeof(src)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    //Dst
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, dst, sizeof(dst)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    //NET_ID
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, netID, sizeof(netID)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    //Length
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, length, sizeof(length)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    return headerInd;
}

int unpackToSymbols(TX_SYMBOL_DATATYPE *symbolBuf, TX_MODTYPE_DATATYPE *modulationBuf, uint64_t val, uint8_t bitsPerVal, uint8_t bitsPerSymbol, uint8_t symbolRepetitions){
    int symbolsPerVal = bitsPerVal/bitsPerSymbol;
    uint64_t valMask = (((uint64_t)1) << bitsPerSymbol)-1;

    int modType;
    switch(bitsPerSymbol){
        case 1:
            modType = MOD_TYPE_BPSK;
            break;
        case 2:
            modType = MOD_TYPE_QPSK;
            break;
        case 4:
            modType = MOD_TYPE_QAM16;
            break;
        default:
            modType = MOD_TYPE_BPSK;
            break;
    }

    for(int i = 0; i<symbolsPerVal; i++) {
        int64_t symbol = val & valMask;
        val = val >> bitsPerSymbol;
        for (int j = 0; j < symbolRepetitions; j++) {
            symbolBuf[i*symbolRepetitions+j] = (TX_SYMBOL_DATATYPE) symbol;
            modulationBuf[i*symbolRepetitions+j] = (TX_MODTYPE_DATATYPE) modType;
        }
    }

    return symbolsPerVal*symbolRepetitions;
}

int createRawASCIIPayload(TX_SYMBOL_DATATYPE *packetBuffer, TX_MODTYPE_DATATYPE *modeModeBuffer, const char* message, uint16_t payloadLenBytes, uint16_t bitsPerSymbol, int* msgBytesRead){
    int msgIndex = 0;
    int msgBytesReadLocal = -1;
    int arrayIndex = 0;
    bool foundEndOfStr = false;

    while(msgIndex<payloadLenBytes){
        if(!foundEndOfStr){
            if(message[msgIndex] == '\0'){
                msgBytesReadLocal = msgIndex;
                foundEndOfStr = true;
            }
        }

        if(foundEndOfStr){
            //Use padding byte
            uint8_t paddingByte;
            switch(bitsPerSymbol){
                case 1:
                    paddingByte = PADDING_BYTE_BPSK;
                    break;
                case 2:
                    paddingByte = PADDING_BYTE_QPSK;
                    break;
                case 4:
                    paddingByte = PADDING_BYTE_QAM16;
                    break;
                default:
                    paddingByte = PADDING_BYTE_BPSK;
                    break;
            }
            arrayIndex += unpackToSymbols(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, paddingByte, sizeof(*message)*8, bitsPerSymbol, TX_REPITIONS_PER_SYMBOL);
        }else{
            arrayIndex += unpackToSymbols(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, message[msgIndex], sizeof(*message)*8, bitsPerSymbol, TX_REPITIONS_PER_SYMBOL);
        }

        msgIndex++;
    }

    //If we did not encounter the nullPtr
    if(msgBytesReadLocal<0){
        msgBytesReadLocal = msgIndex;
    }

    *msgBytesRead = msgBytesReadLocal;
    return arrayIndex;
}

//TODO: Implement CRC.  For now, just adds padding
int createCRC(TX_SYMBOL_DATATYPE *packetBuffer, TX_MODTYPE_DATATYPE *modeModeBuffer, uint16_t bitsPerSymbol){
    int arrayIndex = 0;

    for(int i = 0; i<CRC_BYTES_LEN; i++){
        //Use padding byte
        uint8_t paddingByte;
        switch(bitsPerSymbol){
            case 1:
                paddingByte = PADDING_BYTE_BPSK;
                break;
            case 2:
                paddingByte = PADDING_BYTE_QPSK;
                break;
            case 4:
                paddingByte = PADDING_BYTE_QAM16;
                break;
            default:
                paddingByte = PADDING_BYTE_BPSK;
                break;
        }
        arrayIndex += unpackToSymbols(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, paddingByte, 8, bitsPerSymbol, TX_REPITIONS_PER_SYMBOL);
    }

    return arrayIndex;
}

int createRawCyclopsFrame(TX_SYMBOL_DATATYPE *packetBuffer, TX_MODTYPE_DATATYPE *modeModeBuffer, uint8_t src, uint8_t dst, uint16_t netID, int bitsPerPayloadSymbol, const char* message, int* msgBytesRead){
    int arrayIndex = 0;

    int modType;
    int msgLen;
    switch(bitsPerPayloadSymbol){
        case 1:
            modType = MOD_TYPE_BPSK;
            msgLen = BPSK_PAYLOAD_LEN_BYTES;
            break;
        case 2:
            modType = MOD_TYPE_QPSK;
            msgLen = QPSK_PAYLOAD_LEN_BYTES;
            break;
        case 4:
            modType = MOD_TYPE_QAM16;
            msgLen = QAM16_PAYLOAD_LEN_BYTES;
            break;
        default:
            modType = MOD_TYPE_BPSK;
            msgLen = BPSK_PAYLOAD_LEN_BYTES;
            break;
    }

    //Preamble
    // printf("Index: %d\n", arrayIndex);
    arrayIndex += createRawPreamble(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex);

    //Header
    // printf("Index: %d\n", arrayIndex);
    arrayIndex += createRawHeader(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, modType, 0, src, dst, netID, msgLen);

    //Payload
    // printf("Index: %d\n", arrayIndex);
    arrayIndex += createRawASCIIPayload(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, message, msgLen, bitsPerPayloadSymbol, msgBytesRead);

    //CRC
    // printf("Index: %d\n", arrayIndex);
    arrayIndex += createCRC(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, bitsPerPayloadSymbol);

    // printf("Index: %d\n", arrayIndex);

    return arrayIndex;
}

int filterRepackRxData(RX_PACKED_DATATYPE* resultPacked, RX_PACKED_LAST_DATATYPE* resultPackedLast, const RX_PACKED_DATATYPE* rawPacked, const RX_PACKED_LAST_DATATYPE* rawPackedLast, const RX_STROBE_DATATYPE* rawStrobe, const RX_PACKED_VALID_DATATYPE* rawValid, int rawLen, RX_PACKED_DATATYPE *remainingPacked, RX_PACKED_LAST_DATATYPE *remainingLast, int *remainingBits, int *phaseCounter){
    int dstInd = 0;

    static_assert(sizeof(RX_PACKED_DATATYPE)*8 % RX_PACKED_BITS == 0, "RX_PACKED_DATATYPE must be a multiple of RX_PACKED_BITS"); //Simplfies computation for now
    static_assert(sizeof(RX_PACKED_DATATYPE) == 1, "For now, restriciting RX_PACKED_DATATYPE to bytes for sanitization code"); //TODO: change

    int currentBits = *remainingBits;
    RX_PACKED_DATATYPE repackedLocal = *remainingPacked;
    RX_PACKED_LAST_DATATYPE repackedLastLocal = *remainingLast;

    int newShiftAmt = sizeof(RX_PACKED_DATATYPE)*8 - RX_PACKED_BITS;

    for(int i = 0; i<rawLen; i++){
        if(*phaseCounter == 0 && rawValid[i] && rawStrobe[i]){
            repackedLocal = (rawPacked[i] << newShiftAmt)|(repackedLocal >> RX_PACKED_BITS);
            repackedLastLocal = repackedLastLocal || rawPackedLast[i];
            currentBits += RX_PACKED_BITS;
            if(currentBits == sizeof(RX_PACKED_DATATYPE)*8){
                //Complete, store
                resultPacked[dstInd] = repackedLocal;
                resultPackedLast[dstInd] = repackedLastLocal;
                // printf("0b%d%d%d%d %d%d%d%d [%c] (%d)\n", (resultPacked[dstInd]>>7)&1, (resultPacked[dstInd]>>6)&1, (resultPacked[dstInd]>>5)&1, (resultPacked[dstInd]>>4)&1, (resultPacked[dstInd]>>3)&1, (resultPacked[dstInd]>>2)&1, (resultPacked[dstInd]>>1)&1, (resultPacked[dstInd])&1, resultPacked[dstInd], resultPackedLast[dstInd]);
                dstInd++;

                //Reset state
                currentBits = 0;
                repackedLocal = 0;
                repackedLastLocal = false;
            }
        }
        *phaseCounter = *phaseCounter < (RX_REPITITIONS_PER_OUTPUT-1) ? (*phaseCounter)+1 : 0;
    }

    *remainingBits = currentBits;
    *remainingPacked = repackedLocal;
    *remainingLast = repackedLastLocal;

    return dstInd;
}

void printPacket(const RX_PACKED_DATATYPE* packedFiltered, const RX_PACKED_LAST_DATATYPE* packedFilteredLast, int packedFilteredLen, int* byteInPacket, uint8_t* modMode, uint8_t* type, uint8_t* src, uint8_t *dst, int16_t *netID, int16_t *length, bool printDetails, int *rxCount){
    static_assert((HEADER_SYMBOL_LEN*BITS_PER_SYMBOL_HEADER/8)%sizeof(*packedFiltered) == 0, "For now, header must be a multiple of the size of RX_PACKED_DATATYPE");
    static_assert(BPSK_PAYLOAD_LEN_BYTES%sizeof(*packedFiltered) == 0, "For now, BPSK packet length must be a multiple of the size of RX_PACKED_DATATYPE");
    static_assert(QPSK_PAYLOAD_LEN_BYTES%sizeof(*packedFiltered) == 0, "For now, QPSK packet length must be a multiple of the size of RX_PACKED_DATATYPE");
    static_assert(QAM16_PAYLOAD_LEN_BYTES%sizeof(*packedFiltered) == 0, "For now, 16QAM packet length must be a multiple of the size of RX_PACKED_DATATYPE");
    static_assert(CRC_BYTES_LEN%sizeof(*packedFiltered) == 0, "For now, CRC Checksum length must be a multiple of the size of RX_PACKED_DATATYPE");

    int bytesPerPacked = sizeof(*packedFiltered);
    int packetFilteredByteLen = packedFilteredLen*bytesPerPacked;

    int byteInPacketLocal = *byteInPacket;
    uint8_t *packetFilteredBytes = (uint8_t*) packedFiltered;
    uint8_t *netIDBytes = (uint8_t*) netID;
    uint8_t *lengthBytes = (uint8_t*) length;

    int packedByteInd = 0;

    while(packedByteInd<packetFilteredByteLen) {

        if(byteInPacketLocal<(HEADER_SYMBOL_LEN*BITS_PER_SYMBOL_HEADER/8)) {
            //Parsing the header
            //We do this 1 byte at a time.
            switch(byteInPacketLocal){
                case 0:
                    *modMode = packetFilteredBytes[packedByteInd];
                    break;
                case 1:
                    *type = packetFilteredBytes[packedByteInd];
                    break;
                case 2:
                    *src = packetFilteredBytes[packedByteInd];
                    break;
                case 3:
                    *dst = packetFilteredBytes[packedByteInd];
                    break;
                case 4:
                    netIDBytes[0] = packetFilteredBytes[packedByteInd];
                    break;
                case 5:
                    netIDBytes[1] = packetFilteredBytes[packedByteInd];
                    break;
                case 6:
                    lengthBytes[0] = packetFilteredBytes[packedByteInd];
                    break;
                case 7:
                    lengthBytes[1] = packetFilteredBytes[packedByteInd];
                    break;
                default:
                    //Should never get here
                    printf("Parsing Error\n");
                    exit(1);
            }

            if(byteInPacketLocal==7){
                (*rxCount)++;
                printf("Packet Rx: #%d\n", *rxCount);
                if(printDetails){
                    char *modLabel;
                    switch(*modMode){
                        case MOD_TYPE_BPSK:
                            modLabel = "BPSK";
                            break;
                        case MOD_TYPE_QPSK:
                            modLabel = "QPSK";
                            break;
                        case MOD_TYPE_QAM16:
                            modLabel = "16QAM";
                            break;
                        default:
                            modLabel = "UNKNOWN";
                            break;
                    }

                    printf("Modulation Type = %s, Type = %d, Src = %d, Dst = %d, NET_ID = %d, LENGTH = %d\n", modLabel, *type, *src, *dst, *netID, *length);
                }

                int maxLen;
                bool lenError = false;

                switch(*modMode){
                    case MOD_TYPE_BPSK:
                        maxLen = BPSK_PAYLOAD_LEN_BYTES;
                        break;
                    case MOD_TYPE_QPSK:
                        maxLen = QPSK_PAYLOAD_LEN_BYTES;
                        break;
                    case MOD_TYPE_QAM16:
                        maxLen = QAM16_PAYLOAD_LEN_BYTES;
                        break;
                    default:
                        maxLen = QAM16_PAYLOAD_LEN_BYTES;
                        break;
                }

                if(*length > maxLen){
                    *length = maxLen;
                    lenError = true;
                }

                if(printDetails && lenError){
                    printf("Length Error\n");

                }
            }
            byteInPacketLocal++;
            packedByteInd++;
        }else if(byteInPacketLocal < *length){
            //TODO: Relies on header being a multiple of the packed length

            int packedInd = packedByteInd/bytesPerPacked;

            int packedIndInPayload = byteInPacketLocal/bytesPerPacked - HEADER_SYMBOL_LEN*BITS_PER_SYMBOL_HEADER/8/bytesPerPacked;
            int remainingPacked = (*length+(*length)%bytesPerPacked)/bytesPerPacked - packedIndInPayload;
            int maxPackedInd = packedInd+remainingPacked;
            if(maxPackedInd > packedFilteredLen){
                maxPackedInd = packedFilteredLen;
            }

            int lastInd = -1;
            for(int i = packedInd; i<maxPackedInd; i++){
                if(packedFilteredLast[i]){
                    lastInd = i;
                    break;
                }
            }

            //Print the payload
            if(lastInd < 0){
                //Did not find a last, print up to (but not including) the maxPacketInd
                int maxByteInd = maxPackedInd*bytesPerPacked;
                int byteLen = maxByteInd-packedByteInd;
                char* printStart = packetFilteredBytes+packedByteInd;
                for(int i = 0; i<byteLen; i++){//Sanitize
                    if(!isprint(printStart[i]) && printStart[i]!='\n'){
                        printStart[i] = '^';
                    }
                }
                fwrite(printStart, sizeof(char), byteLen, stdout);
                byteInPacketLocal += byteLen;
                packedByteInd += byteLen;
            }else{
                //print up to (including) the last point then set byteInPacketLocal back to 0
                int maxByteInd = (lastInd+1)*bytesPerPacked;
                int byteLen = maxByteInd-packedByteInd;
                char* printStart = packetFilteredBytes+packedByteInd;
                for(int i = 0; i<byteLen; i++){//Sanitize
                    if(!isprint(printStart[i]) && printStart[i]!='\n'){
                        printStart[i] = '^';
                    }
                }
                fwrite(printStart, sizeof(char), byteLen, stdout);
                printf("\n==== End of Packet ====\n");
                byteInPacketLocal = 0;
                packedByteInd += byteLen;
            }
        }else{
            //should be the CRC.  Search until a last is found
            int packedInd = packedByteInd/bytesPerPacked;

            int lastInd = -1;
            for(int i = packedInd; i<packedFilteredLen; i++){
                if(packedFilteredLast[i]){
                    lastInd = i;
                    break;
                }
            }

            if(lastInd < 0){
                //Did not find a last
                int maxByteInd = packedFilteredLen*bytesPerPacked;
                int byteLen = maxByteInd-packedByteInd;

                byteInPacketLocal += byteLen;
                packedByteInd += byteLen;
            }else{
                //Found last!  Reset for the next packet
                printf("\n==== End of Packet ====\n");
                int maxByteInd = (lastInd+1)*bytesPerPacked;
                int byteLen = maxByteInd-packedByteInd;
                if(byteInPacketLocal+byteLen != HEADER_SYMBOL_LEN*BITS_PER_SYMBOL_HEADER/8+(*length)+CRC_BYTES_LEN){
                    printf("Unexpected Position of Last Flag: got %d, expected %d\n", byteInPacketLocal+byteLen, HEADER_SYMBOL_LEN*BITS_PER_SYMBOL_HEADER/8+(*length)+CRC_BYTES_LEN);
                }

                byteInPacketLocal = 0;
                packedByteInd += byteLen;
            }
        }
        //Print until length or last is reached
    }

    *byteInPacket = byteInPacketLocal;
}

//TODO: Move


void parsePacket(const RX_PACKED_DATATYPE* packedFiltered, const RX_PACKED_LAST_DATATYPE* packedFilteredLast, int packedFilteredLen, rx_decoder_state_t *decoderState,
                 packet_buffer_state_t* packetBufferState){ 
    static_assert((HEADER_SYMBOL_LEN*BITS_PER_SYMBOL_HEADER/8)%sizeof(*packedFiltered) == 0, "For now, header must be a multiple of the size of RX_PACKED_DATATYPE");
    static_assert(BPSK_PAYLOAD_LEN_BYTES%sizeof(*packedFiltered) == 0, "For now, BPSK packet length must be a multiple of the size of RX_PACKED_DATATYPE");
    static_assert(QPSK_PAYLOAD_LEN_BYTES%sizeof(*packedFiltered) == 0, "For now, QPSK packet length must be a multiple of the size of RX_PACKED_DATATYPE");
    static_assert(QAM16_PAYLOAD_LEN_BYTES%sizeof(*packedFiltered) == 0, "For now, 16QAM packet length must be a multiple of the size of RX_PACKED_DATATYPE");
    static_assert(CRC_BYTES_LEN%sizeof(*packedFiltered) == 0, "For now, CRC Checksum length must be a multiple of the size of RX_PACKED_DATATYPE");

    int bytesPerPacked = sizeof(*packedFiltered);
    int packetFilteredByteLen = packedFilteredLen*bytesPerPacked;

    int byteInPacketLocal = decoderState->byteInPacket;
    uint8_t *packetFilteredBytes = (uint8_t*) packedFiltered;
    uint8_t *netIDBytes = (uint8_t*) &(decoderState->netID);
    uint8_t *lengthBytes = (uint8_t*) &(decoderState->length);

    int packedByteInd = 0;

    //Check if we are overfull.  We need to reserve one entry for the next reception

    while(packedByteInd<packetFilteredByteLen) {

        if(byteInPacketLocal<(HEADER_SYMBOL_LEN*BITS_PER_SYMBOL_HEADER/8)) {
            //Parsing the header
            //We do this 1 byte at a time.
            switch(byteInPacketLocal){
                case 0:
                    decoderState->modMode = packetFilteredBytes[packedByteInd];
                    break;
                case 1:
                    decoderState->type = packetFilteredBytes[packedByteInd];
                    break;
                case 2:
                    decoderState->src = packetFilteredBytes[packedByteInd];
                    break;
                case 3:
                    decoderState->dst = packetFilteredBytes[packedByteInd];
                    break;
                case 4:
                    netIDBytes[0] = packetFilteredBytes[packedByteInd];
                    break;
                case 5:
                    netIDBytes[1] = packetFilteredBytes[packedByteInd];
                    break;
                case 6:
                    lengthBytes[0] = packetFilteredBytes[packedByteInd];
                    break;
                case 7:
                    lengthBytes[1] = packetFilteredBytes[packedByteInd];
                    break;
                default:
                    //Should never get here
                    printf("Parsing Error\n");
                    exit(1);
            }

            if(byteInPacketLocal==7){
                (decoderState->rxCount)++;

                int maxLen;
                bool lenError = false;

                switch(decoderState->modMode){
                    case MOD_TYPE_BPSK:
                        maxLen = BPSK_PAYLOAD_LEN_BYTES;
                        break;
                    case MOD_TYPE_QPSK:
                        maxLen = QPSK_PAYLOAD_LEN_BYTES;
                        break;
                    case MOD_TYPE_QAM16:
                        maxLen = QAM16_PAYLOAD_LEN_BYTES;
                        break;
                    default:
                        maxLen = QAM16_PAYLOAD_LEN_BYTES;
                        break;
                }

                if(decoderState->length > maxLen){
                    decoderState->length = maxLen;
                    lenError = true;
                }

                if(lenError){
                    printf("Length Error\n");

                }

                //Check if there is a free entry to start writing
                if(packetBufferState->packetBufferOccupancy >= packetBufferState->packetBufferSize){
                    printf("Error, Overran Rx Packet Buffer\n");
                    exit(1);
                }

                //Set the current packet object to write to:
                decoderState->currentPacket = packetBufferState->packetBuffer+(((packetBufferState->packetBufferReadInd)+(packetBufferState->packetBufferOccupancy))%packetBufferState->packetBufferSize);
                decoderState->currentPacket->dataLen = 0;
            }
            byteInPacketLocal++;
            packedByteInd++;
        }else if(byteInPacketLocal < decoderState->length){
            //TODO: Relies on header being a multiple of the packed length

            int packedInd = packedByteInd/bytesPerPacked;

            int packedIndInPayload = byteInPacketLocal/bytesPerPacked - HEADER_SYMBOL_LEN*BITS_PER_SYMBOL_HEADER/8/bytesPerPacked;
            int remainingPacked = (decoderState->length+(decoderState->length)%bytesPerPacked)/bytesPerPacked - packedIndInPayload;
            int maxPackedInd = packedInd+remainingPacked;
            if(maxPackedInd > packedFilteredLen){
                maxPackedInd = packedFilteredLen;
            }

            int lastInd = -1;
            for(int i = packedInd; i<maxPackedInd; i++){
                if(packedFilteredLast[i]){
                    lastInd = i;
                    break;
                }
            }

            //Copy the payload
            if(lastInd < 0){
                //Did not find a last, print up to (but not including) the maxPacketInd
                int maxByteInd = maxPackedInd*bytesPerPacked;
                int byteLen = maxByteInd-packedByteInd;
                char* printStart = packetFilteredBytes+packedByteInd;
                for(int i = 0; i<byteLen; i++){//Sanitize
                    if(!isprint(printStart[i]) && printStart[i]!='\n'){
                        printStart[i] = '^';
                    }
                }

                memcpy(decoderState->currentPacket->data+decoderState->currentPacket->dataLen, printStart, byteLen);
                decoderState->currentPacket->dataLen += byteLen;

                byteInPacketLocal += byteLen;
                packedByteInd += byteLen;
            }else{
                //print up to (including) the last point then set byteInPacketLocal back to 0
                int maxByteInd = (lastInd+1)*bytesPerPacked;
                int byteLen = maxByteInd-packedByteInd;
                char* printStart = packetFilteredBytes+packedByteInd;

                for(int i = 0; i<byteLen; i++){//Sanitize
                    if(!isprint(printStart[i]) && printStart[i]!='\n'){
                        printStart[i] = '^';
                    }
                }

                memcpy(decoderState->currentPacket->data+decoderState->currentPacket->dataLen, printStart, byteLen);
                decoderState->currentPacket->dataLen += byteLen;
                byteInPacketLocal = 0;
                packedByteInd += byteLen;
            }
        }else{
            //should be the CRC.  Search until a last is found
            int packedInd = packedByteInd/bytesPerPacked;

            int lastInd = -1;
            for(int i = packedInd; i<packedFilteredLen; i++){
                if(packedFilteredLast[i]){
                    lastInd = i;
                    break;
                }
            }

            if(lastInd < 0){
                //Did not find a last
                int maxByteInd = packedFilteredLen*bytesPerPacked;
                int byteLen = maxByteInd-packedByteInd;

                byteInPacketLocal += byteLen;
                packedByteInd += byteLen;
            }else{
                //Found last!  Reset for the next packet
                int maxByteInd = (lastInd+1)*bytesPerPacked;
                int byteLen = maxByteInd-packedByteInd;
                if(byteInPacketLocal+byteLen != HEADER_SYMBOL_LEN*BITS_PER_SYMBOL_HEADER/8+(decoderState->length)+CRC_BYTES_LEN){
                    printf("Unexpected Position of Last Flag: got %d, expected %d\n", byteInPacketLocal+byteLen, HEADER_SYMBOL_LEN*BITS_PER_SYMBOL_HEADER/8+(decoderState->length)+CRC_BYTES_LEN);
                }

                //Decode the seq # using a rep3 code
                //Not a great code but dirt simple to implement
                int srcLoc = decoderState->src;
                int dstLoc = decoderState->dst;
                int netIDLoc = decoderState->netID;

                decoderState->currentPacket->id = (srcLoc & dstLoc) | (srcLoc & netIDLoc) | (dstLoc & netIDLoc);
                decoderState->currentPacket->modMode = decoderState->modMode;
                decoderState->currentPacket->packetType = decoderState->type;
                decoderState->currentPacket->rxCount = decoderState->rxCount;

                //Increment the occupancy of the packet buffer
                (packetBufferState->packetBufferOccupancy)++;

                //TODO: dump packet metadata struct
                //TODO: use repcode to get sequence number

                byteInPacketLocal = 0;
                packedByteInd += byteLen;
            }
        }
    }

    decoderState->byteInPacket = byteInPacketLocal;
}

void printPacketStruct(rx_packet_t* packet, int ch, bool printTitle, bool printDetails, bool printContent){
    if(printTitle){
        setColor(BG_COLOR, FG_COLOR_START+ch, RESET); //Invert FG/BG for title
        printf("Ch. %d Packet Rx: #%d\n", ch, packet->rxCount);
    }
    if(printDetails){
        char *modLabel;
        switch(packet->modMode){
            case MOD_TYPE_BPSK:
                modLabel = "BPSK";
                break;
            case MOD_TYPE_QPSK:
                modLabel = "QPSK";
                break;
            case MOD_TYPE_QAM16:
                modLabel = "16QAM";
                break;
            default:
                modLabel = "UNKNOWN";
                break;
        }

        setColor(BG_COLOR, FG_COLOR_START+ch, RESET); //Invert FG/BG for title
        printf("Modulation Type = %s, Type = %d, ID = %d, Length = %d\n", modLabel, packet->packetType, packet->id, packet->dataLen);
    }

    if(printContent){
        setColor(FG_COLOR_START+ch, BG_COLOR, RESET);
        fwrite(packet->data, sizeof(char), packet->dataLen, stdout);
    }

    if(printTitle){
        printf("\n");
        setColor(BG_COLOR, FG_COLOR_START+ch, RESET); //Invert FG/BG for title
        printf("==== End of Packet ====\n");
        printf("\n");
    }

    setColor(FG_COLOR_DEFAULT, BG_COLOR, RESET);
}

//Based on (https://www.linuxjournal.com/article/8603)
void setColor(int fg_color, int bg_color, int mode){
    printf("\e[%d;%d;%dm", mode, fg_color+30, bg_color+40);
}

/**
 * Current buffer is the buffer currently being watched
 * CurrentID is the ID we are currently looking for
 */
void processPackets(packet_buffer_state_t** buffers, int numBuffers, int* currentID, int maxID, int *currentBuffer, int *failureCount, int maxFailures, bool printTitle, bool printDetails, bool printContent){
    bool running = true;

    while(running){
        //Check the current buffer for occupancy >0
        packet_buffer_state_t* buffer = buffers[*currentBuffer];
        if(buffer->packetBufferOccupancy > 0){
            //There is a packet ready
            //Check the ID
            if(buffer->packetBuffer[buffer->packetBufferReadInd].id == *currentID){
                //This packet has the correct ID, print it
                printPacketStruct(buffer->packetBuffer+(buffer->packetBufferReadInd), *currentBuffer, printTitle, printDetails, printContent);
                //Advance the read pointer, decrement occupency
                buffer->packetBufferOccupancy--;
                buffer->packetBufferReadInd = (buffer->packetBufferReadInd < buffer->packetBufferSize-1) ? buffer->packetBufferReadInd+1 : 0;
                //Advance the currentID and currentBuffer
                *currentID = *currentID < maxID-1 ? *currentID+1 : 0;
                *currentBuffer = *currentBuffer < numBuffers-1 ? *currentBuffer+1 : 0;
                //Reset the failure counter
                *failureCount = 0;
            }else{
                bool pktIDGreater; //Because the ID loops, the logic to check if the ID is greater than the current ID or less than it requires some more logic
                if(*currentID<maxID/2){
                    pktIDGreater = buffer->packetBuffer[buffer->packetBufferReadInd].id < (*currentID+maxID/2) && buffer->packetBuffer[buffer->packetBufferReadInd].id > *currentID;
                }else{
                    pktIDGreater = !(buffer->packetBuffer[buffer->packetBufferReadInd].id > (*currentID-maxID/2) && buffer->packetBuffer[buffer->packetBufferReadInd].id < *currentID);
                }

                if(pktIDGreater){
                    //This packet is later than expected, assume a packet was lost
                    //Do not consume the packet but print the loss message
                    //Increment the ID and buffer
                    printf("Packet Lost Ch: %d\n", *currentBuffer);
                    *currentID = *currentID < maxID-1 ? *currentID+1 : 0;
                    *currentBuffer = *currentBuffer < numBuffers-1 ? *currentBuffer+1 : 0;
                    *failureCount++;
                }else{
                    //This packet was from the past.  Out of order?
                    //Report the out of order reception, dequeue it and increment the relevent state
                    printf("Old Packet Ch: %d\n", *currentBuffer);
                    buffer->packetBufferOccupancy--;
                    buffer->packetBufferReadInd = (buffer->packetBufferReadInd < buffer->packetBufferSize-1) ? buffer->packetBufferReadInd+1 : 0;
                    *currentID = *currentID < maxID-1 ? *currentID+1 : 0;
                    *currentBuffer = *currentBuffer < numBuffers-1 ? *currentBuffer+1 : 0;
                    *failureCount++;
                }
            }
        }else{
            //Buffer does not have any pending packets
            //Check if any of the other buffers have >=3 packets
            bool otherBufferWithHighOccupancy = false;
            for(int i = 0; i<numBuffers; i++){
                if(buffers[i]->packetBufferOccupancy>3){
                    otherBufferWithHighOccupancy=true;
                    break;
                }
            }

            //If so, assume packet lost
            if(otherBufferWithHighOccupancy){
                printf("Packet Lost Ch: %d\n", *currentBuffer);
                *currentID = *currentID < maxID-1 ? *currentID+1 : 0;
                *currentBuffer = *currentBuffer < numBuffers-1 ? *currentBuffer+1 : 0;
                *failureCount++;
            }else{
                //Else, nothing left to process, exit the loop
                running = false;
            }
        }

        if(*failureCount >= maxFailures){
            running = false;
        }
    }

    if(*failureCount >= maxFailures){
        //Clear all the buffers
        for(int i = 0; i<numBuffers; i++){
            int occupancy = buffers[i]->packetBufferOccupancy;
            int ind = buffers[i]->packetBufferReadInd;
            buffers[i]->packetBufferReadInd = (ind+occupancy)%buffers[i]->packetBufferSize;
            buffers[i]->packetBufferOccupancy = 0;
        }

        //Reset to waiting for ID 0 on Ch 0
        *currentID = 0;
        *currentBuffer = 0;

        printf("Error Cluster, Rx Processing Resetting...\n");
        *failureCount = 0;

    }
}