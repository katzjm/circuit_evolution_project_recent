/*
 * Describes helper structs and methods used to configure Networks, Reactions
 * and the program in general
 *
 * Author: Josh Katz
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct reaction_config_str {
    int num_reagents;
    float max_rate_constant;
    float min_rate_constant;
    float max_percent_rate_change;
} RConfig, *RConfig_Ptr;

typedef struct network_config_str {
    int max_num_reactions;
    int min_num_reactions;
} NConfig, *NConfig_Ptr;

typedef struct program_config_str {

} PConfig, *PConfig_Ptr;

#endif // _CONFIG_H_
