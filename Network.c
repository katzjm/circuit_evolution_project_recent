/*
* A Network represents a group of reactions that governs how different
* reagents interact with one another. Note that a Network can support
* MAX_NUM_REACTIONS at a time
*
* Author: Josh Katz
*/

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "nvector/nvector_serial.h"

#include "Network.h"
#include "Reaction.h"
#include "Configs.h"
#include "Cvode_Utils.h"

static void SetSourcesAndSinks(Network_Ptr network) {
  // Initially sources and sinks simply mark whether a species appears as a
  // reactant or product, respectively. Mark no species initially 
  network->sources = 0;
  network->sinks = 0;

  // Mark every species in every reaction
  for (int i = 0; i < network->num_reactions; i++) {
    Reaction_Ptr reaction = &network->reactions[i];

    network->sources |= (1 << reaction->reactant_1);
    network->sinks |= (1 << reaction->product_1);
    if (reaction->reactant_2 != NO_REAGENT) {
      network->sources |= (1 << reaction->reactant_2);
    }
    if (reaction->product_2 != NO_REAGENT) {
      network->sinks |= (1 << reaction->product_2);
    }
  }

  // If a species appears in both sources and sinks it is neither, so remove
  // the mark for that species in both sources and sinks
  int source_and_sink = network->sources & network->sinks;
  network->sources &= ~source_and_sink;
  network->sinks &= ~source_and_sink;
}

bool IsSource(Network_Ptr network, species_t species) {
  return (network->sources & (1 << species)) != 0;
}

bool IsSink(Network_Ptr network, species_t species) {
  return (network->sources & (1 << species)) != 0;
}

bool IsChanging(Network_Ptr network, species_t species) {
  return species != NO_REAGENT
         && !IsSource(network, species)
         && !IsSink(network, species);
}


void SetRandomNetwork(Network_Ptr network, Config_Ptr config) {
  int reaction_range = config->max_num_reactions - config->min_num_reactions;
  network->num_reactions = rand() % reaction_range + config->min_num_reactions;
  network->fitness = INFINITY;
  for (int i = 0; i < network->num_reactions; i++) {
    SetRandomReaction(&network->reactions[i], config);
  }
  SetSourcesAndSinks(network);
}

// Simple min function. Should not be used when either x or y need to be
// evaluated before comparison
#define min(x, y) (((x) <= (y)) ? (x) : (y))

void SetNetwork(Network_Ptr network,
                Reaction_Ptr reactions,
                double fitness,
                int num_reactions) {
  network->num_reactions = min(num_reactions, MAX_NUM_REACTIONS);
  network->fitness = fitness;
  for (int i = 0; i < network->num_reactions; i++) {
    network->reactions[i] = reactions[i];
  }
  SetSourcesAndSinks(network);
}

#undef min

void MutateNetwork(Network_Ptr network, Config_Ptr config) {
  float mutation = (float) rand() / RAND_MAX;
  bool network_changed = false;

  if ((mutation -= config->prob_add_reaction) <= 0) {
    network_changed = AddReaction(network, config);
  } else if ((mutation -= config->prob_remove_reaction) <= 0) {
    network_changed = RemoveReaction(network);
  }

  if (mutation > 0 || network_changed == false) {
    ModifyRateConstant(network, config);
  }
}

bool AddReaction(Network_Ptr network, Config_Ptr config) {
  if (network->num_reactions == MAX_NUM_REACTIONS) {
    return false;
  }

  SetRandomReaction(&network->reactions[network->num_reactions], config);
  network->num_reactions++;
  SetSourcesAndSinks(network);
  return true;
}

bool RemoveReaction(Network_Ptr network) {
  if (network->num_reactions <= 0) {
    return false;
  }
  network->num_reactions--;
  SetSourcesAndSinks(network);
  return true;
}

void ModifyRateConstant(Network_Ptr network, Config_Ptr config) {
  if (network->num_reactions < 1) {
    return;
  }

  int reaction_to_change = rand() % network->num_reactions;
  MutateRateConstant(&network->reactions[reaction_to_change], config);
}

N_Vector GetInitialConcentrations(Config_Ptr c) {
  N_Vector init_concentrations = N_VNew_Serial(c->num_species);
  realtype *init_concentrations_data = NV_DATA_S(init_concentrations);

  for (int i = 0; i < c->num_species; i++) {
    init_concentrations_data[i] = c->initial_concentrations;
  }

  return init_concentrations;
}

static int EvaluateNetworkVsTime(Network_Ptr network,
                                 Config_Ptr c,
                                 CvodeData_Ptr cvode_data) {
  SetUpCvodeNextRun(cvode_data);
  double species_fitness[c->num_species];
  memset(species_fitness, 0, sizeof(species_fitness[0]) * c->num_species);

  realtype t = 0;
  for (int i = 0; i < c->num_data_pts; i++) {
    realtype tout = c->inputs[i];
    if (!RunCvode(cvode_data, tout, &t)) {
      network->fitness = INFINITY;
      return 1;
    }

    for (int j = 0; j < c->num_species; j++) {
      if (!IsChanging(network, j)) {
        double found = NV_Ith_S(cvode_data->concentration_mem, j);
        species_fitness[j] += pow(found - c->outputs[i], 2);
      } else {
        species_fitness[j] = INFINITY;
      }
    }
  }

  network->fitness = INFINITY;
  for (int i = 0; i < c->num_species; i++) {
    if (species_fitness[i] < network->fitness) {
      network->fitness = species_fitness[i];
    }
  }
  return 0;
}

static int EvaluateNetworkVsConcentration(Network_Ptr network,
                                          Config_Ptr c,
                                          CvodeData_Ptr cvode_data) {
  realtype t_spacing = 2;
  realtype t_max = 50;
  realtype steady_state_threshold = 0.001;

  for (int i = 0; i < c->num_data_pts; i++) {
    SetUpCvodeNextRun(cvode_data);

    realtype t = 0;
    realtype tout = t_spacing;
    realtype steady_state_score;
    do {
      if (!RunCvode(cvode_data, tout, &t)) {
        network->fitness = INFINITY;
        return 1;
      }
      tout += t_spacing;

      steady_state_score = 0;
      if (fabs(t - t_spacing) > DBL_EPSILON) {
        
      } else {
        steady_state_score = INFINITY;
      }
    } while (steady_state_score > steady_state_threshold && t < t_max);


  }

}

int EvaluateNetwork(Network_Ptr network,
                    Config_Ptr c,
                    CvodeData_Ptr cvode_data) {
  if (c->time_based) {
    EvaluateNetworkVsTime(network, c, cvode_data);
  } else {

  }
}
