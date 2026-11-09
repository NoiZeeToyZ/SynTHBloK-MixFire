// SynTHBloK-MixFire: registers this plugin's modules.
#include "plugin.hpp"

Plugin* pluginInstance;

extern Model* modelSynTHBloKMix24;
extern Model* modelSynTHBloKMix8;
extern Model* modelSynTHBloKMix12;

void init(Plugin* p) {
    pluginInstance = p;
    p->addModel(modelSynTHBloKMix24);
    p->addModel(modelSynTHBloKMix8);
    p->addModel(modelSynTHBloKMix12);
}
