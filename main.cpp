///////////////////////////////////////////////////////////////////////////
// GSi Updater - Version 1.0 - July 2013
// A utility by Guido Scognamiglio used to update GSi and Crumar Firmwares
// Uses wxWidgets and RtMidi for cross platform (Win and Mac) compatibility
//
// www.GenuineSoundware.com
//
///////////////////////////////////////////////////////////////////////////

#define APPLICATION_NAME	"GSi Updater v.1.0"
#define FILEDESCLEN			64
#define TIMER_MS			750	// Timer interval in milliseconds. Set it to an acceptable interval for the receiving unit.

///////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "RtMidi.h"
#include "main.h"
#include "cover.bmp.h"		// <--- This contains the cover image, change this if you need a different cover

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif


// Define a new application type, each program should derive a class from wxApp
class GSiUpdater : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

// TIMER needs an univoque ID!
enum
{
    TIMER_ID = 1010
};

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_TIMER(TIMER_ID, MainWindow::OnTimerTimeout)
END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(GSiUpdater)

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool GSiUpdater::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

	// create the main application window
    MainWindow *frame = new MainWindow(NULL);

	// and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

	// Alert if no MIDI OUT ports have been found.
	if (frame->MidiOutPorts == 0)
		frame->alert("No MIDI OUTPUT ports have been found on this computer.\nPlease connect a MIDI interface and retry.");


    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

///////////////////////////////////////////////////////////////////////////
// CONSTRUCT
MainWindow::MainWindow( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	// Init variables
	fileBuffer = NULL;
	MidiOutPort = -1;
	RtMidiPortsAreOpen = false;
	memset (InFileName, 0, sizeof(char) * 256);
	sendingMIDI = false;


	///////////////////////////////////////////////////////////////////////////
	// BUILD WINDOW
	this->SetSizeHints( wxSize( 500,320 ), wxSize( 500,320 ) );
	this->SetBackgroundColour( wxColour( 255, 255, 255 ) );

#if WIN32
	SetIcon(wxICON(ICON_GSi));
#endif

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );

	m_bitmap1 = new wxStaticBitmap( this, wxID_ANY, cover_bmp_to_wx_bitmap(), wxDefaultPosition, wxSize( 480,70 ), 0 );
	//m_bitmap1 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("COVERBMP"), wxBITMAP_TYPE_RESOURCE ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( m_bitmap1, 0, wxALL, 5 );

	bSizer2->Add( bSizer6, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText3 = new wxStaticText( this, wxID_ANY, wxT("MIDI OUT PORT:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	bSizer5->Add( m_staticText3, 0, wxALL, 5 );

	/////////////////////////////////////////////////////////////////////////////////
	// MIDI OUT PORT COMBO BOX
	cmbMIDIOUT = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 300,-1 ), 0, NULL, wxCB_DROPDOWN|wxCB_READONLY|wxCB_SIMPLE );
	bSizer5->Add( cmbMIDIOUT, 0, wxALL, 5 );
	/////////////////////////////////////////////////////////////////////////////////

	bSizer2->Add( bSizer5, 0, wxALIGN_CENTER, 5 );

	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Select the file to load:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer2->Add( m_staticText1, 0, wxALL, 5 );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );

	/////////////////////////////////////////////////////////////////////////////////
	// The file path
	txtFilePath = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 380,-1 ), 0 );
	bSizer1->Add( txtFilePath, 0, wxALL, 5 );
	/////////////////////////////////////////////////////////////////////////////////

	btnOpen = new wxButton( this, wxID_ANY, wxT("Browse"), wxDefaultPosition, wxSize( 90,-1 ), 0 );
	bSizer1->Add( btnOpen, 0, wxALL, 5 );

	bSizer2->Add( bSizer1, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	/////////////////////////////////////////////////////////////////////////////////
	// THE INFORMATION LABEL
	lblInfo = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	lblInfo->Wrap( -1 );
	bSizer4->Add( lblInfo, 0, wxALL, 5 );
	/////////////////////////////////////////////////////////////////////////////////

	bSizer2->Add( bSizer4, 0, wxEXPAND, 5 );

	progressBar = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxSize( 480,30 ), wxGA_HORIZONTAL );
	progressBar->SetValue( 0 );
	bSizer2->Add( progressBar, 0, wxALL, 5 );

	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );

	btnSend = new wxButton( this, wxID_ANY, wxT("SEND MIDI"), wxPoint( -1,-1 ), wxSize( 240,30 ), 0 );
	btnSend->SetFont( wxFont( 8, 74, 90, 92, false, wxT("Tahoma") ) );
	btnSend->Enable( false );

	bSizer7->Add( btnSend, 0, wxALL, 5 );

	bSizer2->Add( bSizer7, 0, wxALIGN_CENTER, 5 );

	this->SetSizer( bSizer2 );
	this->Layout();

	this->Centre( wxBOTH );

	// Setup Timer
	m_Timer = new wxTimer(this, TIMER_ID); // timer needs an ID, can't use wxID_ANY

	// Connect Events
	cmbMIDIOUT->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( MainWindow::cmbMIDIOUTOnCombobox ), NULL, this );
	btnOpen->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainWindow::btnOpenOnButtonClick ), NULL, this );
	btnSend->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainWindow::btnSendOnButtonClick ), NULL, this );

	/////////////////////////////////////////////////////////////////////////////////
	// RTMIDI
	MidiOut = new RtMidiOut();
	MidiOutPorts = MidiOut->getPortCount();

	sprintf(infoLabel, "Found %d MIDI OUT ports", MidiOutPorts);
	lblInfo->SetLabel(infoLabel);

	for (int i=0; i<MidiOutPorts; i++)
	{
		MidiOutPortName.push_back(MidiOut->getPortName(i));

		char txt[64]; sprintf(txt, "%d) %s", i+1, MidiOut->getPortName(i).c_str());
		cmbMIDIOUT->Append(txt);
	}
}

// DESTROY
MainWindow::~MainWindow()
{
	// Disconnect Events
	cmbMIDIOUT->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( MainWindow::cmbMIDIOUTOnCombobox ), NULL, this );
	btnOpen->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainWindow::btnOpenOnButtonClick ), NULL, this );
	btnSend->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainWindow::btnSendOnButtonClick ), NULL, this );

	CloseAllMidiPorts();
	delete MidiOut;

	if (fileBuffer) delete [] fileBuffer;
}

bool MainWindow::FileOpen(wxCommandEvent& WXUNUSED(event) )
{
	static wxString s_extDef;
	wxString path = wxFileSelector(
		_T("Select the file to load"),
		wxEmptyString, wxEmptyString,
		s_extDef,
		wxString::Format
		(
			_T("System Exclusive (*.SYX)|*.syx|All files (%s)|%s"),
			wxFileSelectorDefaultWildcardStr,
			wxFileSelectorDefaultWildcardStr
		),
		wxFD_OPEN|wxFD_CHANGE_DIR|wxFD_PREVIEW,
		this
	);

	if (!path) return false;

	// it is just a sample, would use wxSplitPath in real program
	s_extDef = path.AfterLast(_T('.'));

	//wxLogMessage(_T("You selected the file '%s', remembered extension '%s'"), (const wxChar*) path, (const wxChar*) s_extDef);
	strcpy(InFileName, (const wxChar*) path);

	return true;
}
