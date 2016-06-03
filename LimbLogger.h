#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct LimbLogger : public Logger
				  , public TextEditor
	{
	LimbLogger();
	void logMessage( const String &message ) override;
	};