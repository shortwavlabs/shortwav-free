#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
	pluginInstance = p;

	// Register modules
	p->addModel(modelRandomLfo);
	p->addModel(modelWaveshaper);
	p->addModel(modelDrift);
}
