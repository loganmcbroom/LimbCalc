#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Session : public Component
	{
public:
	Session( const String &name, const String &init = "" );
	Session( const Session& s );
	Session& operator=( const Session & );

	void paint( Graphics & g ) override;
	void resized() override;
	void recalculate();
	double const getLinVol ()  const { return linearVolume;			}
	double const getQuadVol()  const { return quadraticVolume;		}
	double const getRelative() const { return relativeToCurrent;	}
	TextEditor & getData() { return data; }

	void updateRelative( const Session & );
	

private:
	TextEditor data;
	double linearVolume = 0, quadraticVolume = 0, relativeToCurrent = 0;
	std::vector< double > c; 
	};