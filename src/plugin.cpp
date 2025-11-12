#include "plugin.hpp"
#include "RandomLfo.hpp"
#include "Waveshaper.hpp"

Plugin *pluginInstance;

extern Model *modelRandomlfo;
extern Model *modelWaveshaper;

void init(Plugin *p) {
	pluginInstance = p;

	// Register modules
	p->addModel(modelRandomlfo);
	p->addModel(modelWaveshaper);
}
