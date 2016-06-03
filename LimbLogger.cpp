#include "LimbLogger.h"


LimbLogger::LimbLogger()
		{
		setReadOnly( true );
		setMultiLine( true, true );
		}

void LimbLogger::logMessage( const String &message )
	{
	Logger::outputDebugString( message );
	insertTextAtCaret( String( "\n" ) + message  );
	}
