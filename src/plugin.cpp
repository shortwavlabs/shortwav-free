#include "plugin.hpp"
#include "RandomLfo.hpp"
#include "Waveshaper.hpp"
#include "Drift.hpp"

Plugin *pluginInstance;

extern Model *modelRandomlfo;
extern Model *modelWaveshaper;
extern Model *modelDrift;

void init(Plugin *p)
{
	pluginInstance = p;

	// Register modules
	p->addModel(modelRandomlfo);
	p->addModel(modelWaveshaper);
	p->addModel(modelDrift);
}
