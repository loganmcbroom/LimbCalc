#pragma once

//	Patients - stored as json files
//		Sessions
//			Individual Measurements

//Fix swapping out the displayed Session when clicking in listbox / adding new
//Graphics display of measurements

//Add patient file drop in
//Add ctrl s -> save, ctrl n - > new, ctrl o -> open

#include "../JuceLibraryCode/JuceHeader.h"
#include "LimbLogger.h"
#include "Session.h"

#include <vector>

class MainContentComponent : public Component
						   , public TextEditor::Listener
						   , public Button::Listener
						   , public ListBoxModel
{
public:
    MainContentComponent();
    ~MainContentComponent();

    void paint( Graphics & ) override;
    void resized() override;
	void textEditorTextChanged( TextEditor & ) override;

	int getNumRows() override { return sessions.size(); }
	void paintListBoxItem( int, Graphics &, int, int, bool );
	//void listBoxItemClicked( int, const MouseEvent & ) override;
	//void backgroundClicked( const MouseEvent & ) override;
	//void deleteKeyPressed( int lastRowSelected ) override;
	void selectedRowsChanged( int ) override;

private:

	struct LimbButton : public Button
		{
		LimbButton( const String &name )
			: Button( name )
			{
			}

		void paintButton( Graphics &g, bool isMouseOverButton, bool isButtonDown ) override
			{
			g.fillAll( Colour::fromHSV( .4, .5, .5, 1 ) );

			g.setFont( Font( 16.0f )  );
			g.setColour( Colours::white );
			g.drawText( getButtonText(), getLocalBounds(), Justification::centred );

			setMouseCursor( MouseCursor::StandardCursorType::PointingHandCursor );
			}
		};

	void calculateVolume();
	void calculateRelativeVolumes();

	void buttonClicked( Button * ) override;
	void openButtonPressed();
	void saveButtonPressed();
	void newButtonPressed();
	void addButtonPressed();
	void removeButtonPressed();
	void setPatient( File & );

	ListBox sessionsList;
	std::vector< Session > sessions;
	LimbButton openButton, newButton, saveButton, addButton, removeButton;
	LimbLogger log;
	File patient;
	int currentRow = -1;
	bool changes = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

