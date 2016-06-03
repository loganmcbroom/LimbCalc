#include "Session.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "MainComponent.h"

#define MARGIN 10.0f

//
Session::Session( const String & name, const String & init )
	{
	data.setMultiLine( true, false );
	data.setReturnKeyStartsNewLine( true );
	setName( name );
	data.setText( init, true );
	addAndMakeVisible( data );
	}

//
Session::Session( const Session& s )
	: linearVolume		( s.linearVolume		)
	, quadraticVolume	( s.quadraticVolume		)
	, relativeToCurrent	( s.relativeToCurrent	)
	{
	data.setMultiLine( true, false );
	data.setReturnKeyStartsNewLine( true );
	setName( s.getName() );
	data.setText( s.data.getText(), true );
	addAndMakeVisible( data );
	s.getParentComponent()->addAndMakeVisible( this );
	data.addListener( static_cast<MainContentComponent *>( s.getParentComponent() ) );
	}

//
Session& Session::operator=( const Session& s ) 
	{
    linearVolume		= s.linearVolume;
    quadraticVolume		= s.quadraticVolume;
    relativeToCurrent	= s.relativeToCurrent;
	data.setMultiLine( true, false );
	data.setReturnKeyStartsNewLine( true );
	setName( s.getName() );
	data.setText( s.data.getText(), true );
	addAndMakeVisible( data );
	s.getParentComponent()->addAndMakeVisible( this );
	data.addListener( static_cast<MainContentComponent *>( s.getParentComponent() ) );

    return *this;
	}

//
void Session::paint( Graphics & g )
	{
	Rectangle< float > linVol  = Rectangle<float>( MARGIN + 100, 0,  ( getWidth() - ( MARGIN + 100 ) ) / 2.0, 55 );//.reduced( 5 );
	Rectangle< float > quadVol = Rectangle<float>( MARGIN * 2 + 100 + linVol.getWidth(), 0, ( getWidth() - ( MARGIN + 100 ) ) / 2.0, 55 );//.reduced( 5 );
	Colour linColour  = Colour::fromHSV(  0, .8, .8, .5 );
	Colour quadColour = Colour::fromHSV( .5, .8, .8, .5 );
	Path linPath, quadPath;


	g.setColour( Colours::white );
	g.fillRect( linVol .withHeight( 35 ).translated( 0, 20 ) );
	g.fillRect( quadVol.withHeight( 35 ).translated( 0, 20 ) );

	g.setFont( Font( 16.0f ) );
	g.drawText( "Linear Volume",	linVol .withHeight( 20 ), Justification::centred );
	g.drawText( "Quadratic Volume", quadVol.withHeight( 20 ), Justification::centred );
	
	g.setColour( Colours::black );
	g.setFont( Font( 24.0f ) );
	g.drawText( String( linearVolume ), linVol.withHeight( 35 ).translated( 0, 20 ),  Justification::centred );
	g.drawText( String( quadraticVolume ), quadVol.withHeight( 35 ).translated( 0, 20 ), Justification::centred );
		
	//Draw graph
	
	Rectangle<float> linGraph = Rectangle<float>( MARGIN + 100, MARGIN + 65,  
		getWidth() - ( MARGIN + 100 ), ( getWidth() - ( MARGIN + 100 ) ) / 2.0  ).reduced( 5 );
	g.setColour( Colour::fromHSV( 0, 0, .25, 1 ) );
	g.fillRect( linGraph.expanded( 5 ) );
	g.setColour( Colour::fromHSV(0,0,1,1) );
	Line<float> xAxis( 
		linGraph.getTopLeft()  + Point<float>( 20, linGraph.getHeight() / 2.0 ), 
		linGraph.getTopRight() + Point<float>( -20, linGraph.getHeight() / 2.0 ) 
		);
	g.drawLine( xAxis, 2 );
		

	if( c.size() <= 1 ) return;
	int deltaX = ( xAxis.getEndX() - xAxis.getStartX() ) / ( c.size() - 1 );
	double largest = *std::max_element( c.begin(), c.end() );
	if( largest <= 0 ) return;
	
	Point< float > previousEnd( xAxis.getStartX(), xAxis.getEndY() - ( c[0] / largest ) * linGraph.getHeight() / 2.0 );
	Point< float > start, end;
	linPath.startNewSubPath( previousEnd );
	quadPath.startNewSubPath( previousEnd );
	for( int i = 0; i < c.size(); ++i )
		{
		start = Point<float>( xAxis.getStartX() + i * deltaX,  xAxis.getEndY() );
		end   = Point<float>( xAxis.getStartX() + i * deltaX, 
							  xAxis.getEndY() - ( c[i] / largest ) * linGraph.getHeight() / 2.0 );
		g.setColour( Colours::white );
		g.drawLine( Line<float>( start, end ), 2 );

		double e = ( ( double( i ) - double( c.size() - 1 ) / 2.0 ) * 2.0 / double( c.size() ) ) * 10.0; //width / 2
		g.setColour( Colour::fromHSV( 0, 0, .6, .6 ) );
		g.drawEllipse( end.getX() - e, end.getY(), e * 2, ( c[i] / largest ) * linGraph.getHeight(), 1 );

		linPath.lineTo( end );
		if( i % 2 == 0 ) quadPath.quadraticTo( previousEnd, end );
		previousEnd = end;
		}
	if( c.size() % 2 == 0 ) quadPath.lineTo( end );
	g.setColour( quadColour );
	g.strokePath( quadPath, PathStrokeType( 2 ) );
	g.setColour( linColour );
	g.strokePath( linPath, PathStrokeType( 2 ) );
	}

void Session::resized()
	{
	data.setBounds( 0, 0, 100, getHeight() );
	}

//
void Session::recalculate()
	{
	//Convert TextEditor data into an array of doubles
	std::vector< double > _c;
	size_t startIndex = 0, endIndex = 0;
	String & dataString = data.getText();
	

	//Remove any zany end values
	if( dataString.length() == 0 ) return;
	int lastNumber = dataString.length();
	while( ! ( '0' <= dataString[lastNumber] && dataString[lastNumber] <= '9' ) )
		{
		--lastNumber;
		if( lastNumber <= 0 ) 
			{
			return;
			}
		}
	dataString = dataString.substring( 0, lastNumber + 1 );

	//Safe to do the actual conversion now
	while( ( endIndex = dataString.indexOfChar( endIndex + 1, '\n' ) ) != -1 )
		{
		_c.push_back( std::max( dataString.substring( startIndex, endIndex ).getDoubleValue(), 0.0 ) );
		startIndex = endIndex;
		}
	_c.push_back( std::max( dataString.substring( startIndex, dataString.length() ).getDoubleValue(), 0.0 ) );
	c = _c;

	if( c.size() < 3 ) return;

	//Calculate linear
		{
		//4.0 = delta x, dividing by 4pi accounts for the conversion for circumference to radius
		const double factor = ( 4.0 / 2.0 ) / ( M_PI * 4.0 );

		double result = 0;
		result += std::pow( c.front(), 2 );
		result += std::pow( c.back(),  2 );
		for( int i = 1; i < c.size() - 1; ++i )
			{
			result += std::pow( c[i], 2 ) * 2.0f;
			}
		linearVolume = result * factor;
		}

	//Calculate quadratic
		{
		//4.0 = delta x, dividing by 4pi accounts for the conversion for circumference to radius
		const double factor = ( 4.0 / 3.0 ) / ( M_PI * 4.0 );

		double result = 0;
		int cSize = c.size();
		//We can't do quadratic approximation with an even number of entries
		//If we get an even number we'll do linear approximation for the last section
		if( c.size() % 2 == 0 )
			{
			double c1 = c.back();
			double c2 = c[ c.size() - 2 ];
			result += c1*c1 + c2*c2 + c1*c2;
			--cSize;
			}
	
		//First and last entries are accounted for once
		result += std::pow( c.front(), 2 );
		result += std::pow( c.back(),  2 );
		for( int i = 1; i < cSize - 1; ++i )
			{
			//if we're at an even number (not index) 16, otherwise 4, times the square of the measurement
			result += ( std::pow( c[i], 2 ) * ( ( i % 2 == 0 )? 2.0f : 4.0f ) );
			}
		quadraticVolume = result * factor;
		}
	}

void Session::updateRelative( const Session &s )
	{
	relativeToCurrent = s.getQuadVol() == 0 ? -1 : quadraticVolume / s.getQuadVol();
	}