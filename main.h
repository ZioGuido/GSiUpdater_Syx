///////////////////////////////////////////////////////////////////////////
// GSi Updater - Version 1.0 - July 2013
// A utility by Guido Scognamiglio used to update GSi and Crumar Firmwares
// Uses wxWidgets and RtMidi for cross platform (Win and Mac) compatibility
//
// www.GenuineSoundware.com
//
///////////////////////////////////////////////////////////////////////////

#ifndef __NONAME_H__
#define __NONAME_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/gauge.h>
#include <wx/frame.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/timer.h>


///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class MainWindow
///////////////////////////////////////////////////////////////////////////////
class MainWindow : public wxFrame
{
public:
	// Construct
	MainWindow( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT(APPLICATION_NAME), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,320 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	// Destroy
	~MainWindow();

//private:
	wxStaticBitmap	*m_bitmap1;
	wxStaticText	*m_staticText3;
	wxStaticText	*m_staticText1;
	wxButton		*btnOpen;
	wxGauge			*progressBar;
	wxComboBox		*cmbMIDIOUT;
	wxStaticText	*lblInfo;
	wxTextCtrl		*txtFilePath;
	wxButton		*btnSend;
	wxTimer			*m_Timer;

	RtMidiOut	*MidiOut;
	int MidiOutPorts, MidiOutPort;
	bool RtMidiPortsAreOpen;
	std::vector<string> MidiOutPortName;

	size_t InFileSize;
	char InFileName[256];
	char InFileInfo[256];
	char infoLabel[256];
	unsigned char *fileBuffer;
	size_t sendByte;
	bool sendingMIDI;

	// This is called every x ms
	void OnTimerTimeout(wxTimerEvent& event)
    {
		std::vector<unsigned char> MidiMessage;	// allocate vector
		while (sendByte < InFileSize)
		{
			MidiMessage.push_back(fileBuffer[sendByte]);	// fill vector
			if (fileBuffer[sendByte++] == 247) break;		// until F7 is found
		}

		// try to send the sysex string
		try { MidiOut->sendMessage(&MidiMessage); } catch(RtError &rtErr) { alert("Unknown error sending MIDI data."); return; }

		// update label
		sprintf(infoLabel, "Sent %d/%d bytes (%d%%)", sendByte, InFileSize, int(sendByte*100/InFileSize));

		// inform about progress using the SEND button label
		btnSend->SetLabel(infoLabel);

		// increment bar
		progressBar->SetValue(sendByte);

		// Stop sending when reached the end of buffer
		if (sendByte >= InFileSize)
		{
			SendStop();
			sprintf(infoLabel, "Done. Sent %d bytes.", InFileSize);
			lblInfo->SetLabel(infoLabel);
		}
	}

	bool alert(char *msg)
	{
		wxMessageBox(msg, _T(APPLICATION_NAME), wxOK | wxICON_INFORMATION, this);
		return false;
	}

	void SendStop()
	{
		sendingMIDI = false;
		progressBar->SetValue(0);			// reset progress bar
		btnSend->SetLabel("SEND MIDI");		// reset button label
		lblInfo->SetLabel("");				// reset info label
		EnableSendButton();
		m_Timer->Stop();
	}

	void EnableSendButton()
	{
		if (RtMidiPortsAreOpen && InFileSize > 0)
			btnSend->Enable();
	}

	// Virtual event handlers, overide them in your derived class
	virtual void cmbMIDIOUTOnCombobox( wxCommandEvent& event )
	{
		// Get the combobox index
		if ( event.GetEventType() == wxEVT_COMMAND_COMBOBOX_SELECTED ) MidiOutPort = event.GetSelection();

		// Class all MIDI ports
		CloseAllMidiPorts();

		// Try to open the requested MIDI port
		RtMidiPortsAreOpen = OpenMidiOutPort();

		// Check if the SEND button can be enabled
		EnableSendButton();
	}

	virtual void btnOpenOnButtonClick( wxCommandEvent& event )
	{
		// Get the file path	// exit con cancel
		if (!FileOpen(event)) return;

		// Add it to the text box
		txtFilePath->SetValue(InFileName);

		char fileDesc[FILEDESCLEN] = "OK";

		FILE *inFile = fopen(InFileName, "rb");
		if (inFile)
		{
			// check file size
			fseek(inFile, 0, SEEK_END);
			InFileSize = ftell(inFile);

			// rewind file pointer
			rewind(inFile);

			// create buffer
			if (fileBuffer != NULL) delete [] fileBuffer;
			fileBuffer = new unsigned char[InFileSize+1];
			memset(fileBuffer, 0, sizeof(char) * (InFileSize+1));

			// fill buffer with data from current pointer position (BOF + FILEDESCLEN)
			fread(fileBuffer, sizeof(char), InFileSize, inFile);

			// close file
			fclose(inFile);

			// set the progress bar range
			progressBar->SetRange(InFileSize);

			// set info label
			sprintf(infoLabel, "File: %s. Size: %d bytes.", fileDesc, InFileSize);
			lblInfo->SetLabel(infoLabel);

			// Check if the SEND button can be enabled
			EnableSendButton();
		}
		else
		{
			char ermsg[256]; sprintf(ermsg, "ERROR: Could not open file\n: %s", InFileName);
			alert(ermsg);
		}
	}

	virtual void btnSendOnButtonClick( wxCommandEvent& event )
	{
		// Stop sending MIDI if this button is pressed during the upload
		if (sendingMIDI) { SendStop(); return; }

		sprintf(infoLabel, "Sending MIDI... Don't touch anything!");
		lblInfo->SetLabel(infoLabel);

		sendByte = 0;				// reset byte counter
		m_Timer->Start(TIMER_MS);	// start timer... Set interval in the main.cpp file
		sendingMIDI = true;			// inform that the MIDI upload is running
	}

	bool FileOpen(wxCommandEvent& WXUNUSED(event) );

	bool OpenMidiOutPort()
	{
		MidiOut->closePort();

		char ermsg[256];
		try { MidiOut->openPort(MidiOutPort); }
		catch ( RtError &rtErr ) {
			sprintf(ermsg, "Unable to open MIDI Output port: %s", MidiOutPortName[MidiOutPort].c_str());
			alert(ermsg);
			return false;
		}

		sprintf(infoLabel, "MIDI Port %s Ready.", MidiOutPortName[MidiOutPort].c_str());
		lblInfo->SetLabel(infoLabel);
		return true;
	}

	// Close MIDI Ports
	void CloseAllMidiPorts()
	{
		MidiOut->closePort();
		RtMidiPortsAreOpen = false;
	}

	// event handlers (these functions should _not_ be virtual)
	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif //__NONAME_H__
