#include "MainComponent.h"

#define MARGIN 10.0f

//Constructor
MainContentComponent::MainContentComponent()
	: openButton( "Open" )
	, newButton ( "New"  )
	, saveButton( "Save" )
	, addButton ( "Add" )
	, removeButton( "Remove" )
	{
	Logger::setCurrentLogger( &log );
	addAndMakeVisible( log );
	Logger::writeToLog( "Logger Active" );
	Logger::writeToLog( String( "Running LimbCalc from: ") + File::getCurrentWorkingDirectory().getFullPathName() );

	sessionsList.setModel( this );
	
    setSize (MARGIN * 7 + 250 + 150 + 150, 500);
	addAndMakeVisible( sessionsList );
	addAndMakeVisible( openButton   );
	addAndMakeVisible( newButton    );
	addAndMakeVisible( saveButton   );
	addAndMakeVisible( addButton    );
	addAndMakeVisible( removeButton );

	openButton  .addListener( this );
	newButton   .addListener( this );
	saveButton  .addListener( this );
	addButton   .addListener( this );
	removeButton.addListener( this );
	}

//Destructor
MainContentComponent::~MainContentComponent()
	{
	Logger::setCurrentLogger( nullptr );
	if( changes )
		{
		if( AlertWindow::showOkCancelBox( AlertWindow::AlertIconType::WarningIcon, "Save Changes?",
			"Do you want to save first?", "Yes", "No" ) )
			saveButtonPressed();
		}
	}

//Paint
void MainContentComponent::paint( Graphics& g )
	{
    g.fillAll ( Colour::fromHSV( 0, 0, .18, 1 ) );
	}

//Resized
void MainContentComponent::resized()
	{
	sessionsList.setBounds( MARGIN, MARGIN * 3 + 50, 150 + MARGIN * 2, getHeight() - MARGIN * 4 - 50 );

	openButton.setBounds( MARGIN, MARGIN, 50, 25 );
	saveButton.setBounds( MARGIN * 2 + 50, MARGIN, 50, 25 );
	newButton.setBounds( MARGIN * 3 + 100, MARGIN, 50, 25 );

	addButton.setBounds( MARGIN, MARGIN * 2 + 25, 75 + MARGIN * .5, 25 );
	removeButton.setBounds( 75 + MARGIN * (2.5), MARGIN * 2 + 25, 75 + MARGIN * .5, 25 );

	log.setBounds( MARGIN * 4 + 150, getHeight() - MARGIN - 100, getWidth() - ( MARGIN * 4 + 150 ), 100 );

	for( auto &&i : sessions )
		{
		i.setBounds( MARGIN * 4 + 150, MARGIN, getWidth() - ( MARGIN * 5 + 150 ), getHeight() - MARGIN * 3 - 100 ); 
		}
	}

//Typing happened
void MainContentComponent::textEditorTextChanged( TextEditor &editor )
	{
	//update session volume automatically
	calculateVolume();
	repaint();
	changes = true;
	}

//
void MainContentComponent::paintListBoxItem( int rowNumber, Graphics &g, int width, int height, bool rowSelected )
	{
	if( rowNumber >= sessions.size() ) return;
	g.fillAll( Colour::fromHSV( 0, 0, rowSelected? .65 : .82, 1 ) );

	g.setColour( Colours::black );
	g.drawLine( 0, height, width, height, 1.0f );

	g.setFont( Font( 14.0f ) );
	g.setColour( Colours::black );
	g.drawText( sessions[rowNumber].getName(), Rectangle<int>(0,0,width,height).reduced( 2 ), Justification::left );
	g.drawText( String( sessions[rowNumber].getRelative() * 100 ) + String( "%" ), Rectangle<int>(0,0,width,height).reduced( 2 ), Justification::right );
	}

//
void MainContentComponent::selectedRowsChanged( int row ) 
	{
	if( row == -1 ) 
		{
		currentRow = row;
		return;
		}
	calculateRelativeVolumes();
	sessions[row].setVisible( true );
	if( currentRow != -1 ) sessions[currentRow].setVisible( false );
	currentRow = row;
	}

//Calculates the linear and quadratic volume approximation for the currently selected session
void MainContentComponent::calculateVolume()
	{
	Session & currentSession = sessions[ sessionsList.getSelectedRow() ];
	//calculate volume for current session
	currentSession.recalculate();
	currentSession.repaint();

	calculateRelativeVolumes();
	}

//See how other sessions compare against selected
void MainContentComponent::calculateRelativeVolumes()
	{
	for( auto &&i : sessions )
		i.updateRelative( sessions[ sessionsList.getSelectedRow() ] );
	sessionsList.repaint();
	}

//Delegates button pressed to individual functions
void MainContentComponent::buttonClicked( Button *b )
	{
		 if( b == &openButton   ) openButtonPressed  ();
	else if( b == &saveButton   ) saveButtonPressed  ();
	else if( b == &newButton    ) newButtonPressed   ();
	else if( b == &addButton    ) addButtonPressed   ();
	else if( b == &removeButton ) removeButtonPressed();
	}

//
void MainContentComponent::openButtonPressed()
	{
	//Pick a file any file
	FileChooser chooser( "Select Patient", File::nonexistent, "*.json" );
	if( ! chooser.browseForFileToOpen() ) return;

	//Out with the old
	if( changes )
		if( AlertWindow::showOkCancelBox( AlertWindow::AlertIconType::WarningIcon, "Save Changes?",
											"Do you want to save the current data?", "Yes", "No" ) )
			saveButtonPressed();
		
	sessions.clear();
	patient = File();

	//In with the new
	var json = JSON::parse( chooser.getResult() );
	if( json == var::null )
		{
		Logger::writeToLog( "Error parsing patient file" );
		return;
		}

	size_t numSessions = json["Sessions"].size();
	sessions.reserve( numSessions );
	for( int i = 0; i < numSessions; ++i )
		{
		const var &s = json["Sessions"][i];
		sessions.emplace_back( s["Name"], s["Data"] );
		addAndMakeVisible( sessions.back() );
		sessions.back().getData().addListener( this );
		sessions.back().recalculate();
		}
	sessionsList.updateContent();
	resized();
	sessionsList.selectRow( sessions.size() - 1 );

	setPatient( chooser.getResult() );
	}

File createFile()
	{
	FileChooser chooser( "Type patient name", File::nonexistent, "*.json" ); 
	if( ! chooser.browseForFileToSave( true ) ) return File();
	File & f = chooser.getResult();
	if( ! f.exists() )
		{
		if( f.create().failed() )
			{
			Logger::writeToLog( "Error creating new file" );
			}
		}
	return f;
	}

//
void MainContentComponent::saveButtonPressed()
	{
	if( ! patient.exists() ) 
		{
		File f = createFile();
		if( ! f.exists() ) return;
		setPatient( f );
		}

	//Create the json structure
	DynamicObject * json =  new DynamicObject();
	var Sessions;
	for( int i = 0; i < sessions.size(); ++i )
		{
		DynamicObject * entry = new DynamicObject();
		entry->setProperty( "Name", sessions[i].getName() );
		entry->setProperty( "Data", sessions[i].getData().getText() );
		Sessions.append( entry );
		//fill Sessions with data
		//[ {
		//"name" : "the date"
		//"Data" : "0 1 2 3"
		//} ]
		}
	json->setProperty( "Sessions", Sessions );

	//create tempfile and fill with current data
	TemporaryFile tempFile( patient );
	tempFile.getFile().appendText( JSON::toString( json ) );

	//save it over previous file
	if( tempFile.overwriteTargetFileWithTemporary() )
		{
		//Logger::writeToLog( "Juce reporting save error, probably nothing" );
		//return;
		}

	changes = false;
	}

//
void MainContentComponent::newButtonPressed()
	{
	//Give the new file a name and escape if nothing is given
	File f = createFile();
	if( ! f.exists() ) return;
	setPatient( f );
	sessions.clear();
	sessionsList.updateContent();
	}

//
void MainContentComponent::addButtonPressed()
	{
	sessions.emplace_back( Time::getCurrentTime().formatted( "%b %d, %Y" ), "" );
	sessionsList.updateContent();
	addAndMakeVisible( sessions.back() );
	resized();
	sessions.back().getData().addListener( this );
	sessionsList.selectRow( sessions.size() - 1 );
	changes = true;
	}

//
void MainContentComponent::removeButtonPressed()
	{
	if( sessionsList.getSelectedRow() == -1 ) 
		{
		Logger::writeToLog( "Select a row to remove first" );
		return;
		}
	if( AlertWindow::showOkCancelBox( AlertWindow::AlertIconType::WarningIcon, "Are You Sure?",
		"Are you sure you want to delete this data?", "Yes", "No" ) )
		{
		sessions.erase( sessions.begin() + sessionsList.getSelectedRow( 0 ) );
		sessionsList.updateContent();
		changes = true;
		}
	}

void MainContentComponent::setPatient( File &f )
	{
	getParentComponent()->setName( String( "LimbCalc - " ) + f.getFileName() );
	patient = f;
	}
