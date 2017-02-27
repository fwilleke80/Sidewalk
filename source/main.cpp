#include "c4d.h"
#include "c4d_symbols.h"
#include "main.h"


// Some string defines
#define PLUGIN_VERSION String("Sidewalk 1.0.6")


Bool PluginStart()
{
	// Try to register the plugin
	if (!RegisterSidewalkObject())
		return false;

	GePrint(PLUGIN_VERSION);

	return true;
}


void PluginEnd()
{}


Bool PluginMessage(Int32 id, void *data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
			return resource.Init();	// Don't start plugin without resource
	}

	return false;
}
