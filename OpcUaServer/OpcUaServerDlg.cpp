
// OpcUaServerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "OpcUaServer.h"
#include "OpcUaServerDlg.h"
#include "afxdialogex.h"

#include <opc/ua/server/addons/standard_address_space.h>
#include <opc/ua/server/addons/address_space.h>
#include <opc/ua/server/standard_address_space.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// COpcUaServerDlg dialog

using namespace OpcUa;

void SubClient::DataChange(uint32_t handle, const Node& node, const Variant& val, AttributeId attr)
{
	
	CString strMsg(_T("Received DataChange event for Node"));
	strMsg += CA2W(node.GetBrowseName().Name.c_str());
	
}

std::vector<OpcUa::Variant> MyMethod(NodeId context, std::vector<OpcUa::Variant> arguments)
{
	MessageBoxA(nullptr, "MyMethod called! ", "Error", MB_OK);
	std::vector<OpcUa::Variant> result;
	result.push_back(Variant(static_cast<uint8_t>(0)));
	return result;
}

static inline const Node* BrowseintoNode(CTreeCtrl& treeCtrl, HTREEITEM hParent, const Node& node)
{
	for (const auto& children : node.GetChildren())
	{
		BrowseintoNode(treeCtrl, treeCtrl.InsertItem(CString(children.GetBrowseName().Name.c_str()), hParent), children);
	}
	return nullptr;
}

static inline void printNodes(CTreeCtrl& treeCtrl, const Node& node)
{
	treeCtrl.SetRedraw(FALSE);
	for (const auto& children : node.GetChildren())
	{
		auto hItem = treeCtrl.InsertItem(CString(children.GetBrowseName().Name.c_str()), TVI_ROOT);
		BrowseintoNode(treeCtrl, hItem, children);
	}
	treeCtrl.SetRedraw(TRUE);
}

//void COpcUaServerDlg::LogCallback(const std::string& err_msg)
//{
//	m_ListServerLog.AddString(CA2W(err_msg.c_str()));
//}

COpcUaServerDlg::COpcUaServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_OPCUASERVER_DIALOG, pParent)
	, m_strServerEndpoint(_T("opc.tcp://127.0.0.1:49320"))
	, m_strServerUri(_T("urn://exampleserver.freeopcua.github.io"))
	, m_file(freopen("output.txt", "w", stdout))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_Logger = spdlog::stdout_logger_mt("server");

	m_Logger->set_level(spdlog::level::debug);
	//doesn't work!
	//m_Logger->set_error_handler(std::bind(&COpcUaServerDlg::LogCallback, this, std::placeholders::_1));

	/*m_StandartNameSpace = OpcUa::Server::CreateAddressSpace(m_Logger);
	OpcUa::Server::FillStandardNamespace(*m_StandartNameSpace, m_Logger);*/
	
	m_OpcServer = std::make_unique<OpcUa::UaServer>(m_Logger);
}

void COpcUaServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERVER_ENDPOINT, m_editServerEndpoint);
	DDX_Text(pDX, IDC_SERVER_ENDPOINT, m_strServerEndpoint);
	DDX_Control(pDX, IDC_SERVER_URI, m_editServerUri);
	DDX_Text(pDX, IDC_SERVER_URI, m_strServerUri);
	DDX_Control(pDX, IDC_SERVER_LOGGING, m_treeServerLog);
	DDX_Control(pDX, IDC_TAG, m_editTag);
	DDX_Text(pDX, IDC_TAG, m_strTag);
	DDX_Control(pDX, IDC_TAG_ID, m_editTagId);
	DDX_Text(pDX, IDC_TAG_ID, m_strTagId);
	DDX_Control(pDX, IDC_TAG_VALUE, m_editTagValue);
	DDX_Text(pDX, IDC_TAG_VALUE, m_strTagValue);
}

BEGIN_MESSAGE_MAP(COpcUaServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &COpcUaServerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &COpcUaServerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &COpcUaServerDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_INSERT_OR_UPDATE_TAG, &COpcUaServerDlg::OnBnClickedInsertOrUpdateTag)
	ON_BN_CLICKED(IDC_CLEAR_LIST, &COpcUaServerDlg::OnBnClickedClearList)
	ON_BN_CLICKED(IDC_GET_OBJECTS_NODE, &COpcUaServerDlg::OnBnClickedGetObjectsNode)
	ON_BN_CLICKED(IDC_GET_ROOT_NODE, &COpcUaServerDlg::OnBnClickedGetRootNode)
	ON_BN_CLICKED(IDC_GET_SERVER_NODE, &COpcUaServerDlg::OnBnClickedGetServerNode)
	ON_NOTIFY(TVN_SELCHANGED, IDC_SERVER_LOGGING, &COpcUaServerDlg::OnSelchangeServerLogging)
END_MESSAGE_MAP()


// COpcUaServerDlg message handlers

BOOL COpcUaServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetWindowTheme(m_treeServerLog.GetSafeHwnd(), L"Explorer", nullptr);

	m_editServerEndpoint.SetCueBanner(_T("opc.tcp://localhost:4840/freeopcua/server"));
	m_editServerUri.SetCueBanner(_T("urn://exampleserver.freeopcua.github.io"));
	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void COpcUaServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COpcUaServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COpcUaServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void COpcUaServerDlg::OnBnClickedOk()
{
	UpdateData();
	if (!m_strServerEndpoint.Trim().IsEmpty() && !m_strServerUri.Trim().IsEmpty())
	{
		try {
			m_Logger->info("Starting OPC Server...");
			m_OpcServer->SetEndpoint(CW2A(m_strServerEndpoint).m_psz);
			m_OpcServer->SetServerURI(CW2A(m_strServerUri).m_psz);
			m_OpcServer->Start();

			m_serverIdx = m_OpcServer->RegisterNamespace("http://examples.freeopcua.github.io");

			CString strWindowText;
			strWindowText.Format(_T("NameSpace Index : %u"), m_serverIdx);
			SetWindowText(strWindowText);

			m_bOpcServerStarted = TRUE;
		}
		catch (const std::exception& ex)
		{
			MessageBoxA(nullptr, ex.what(), "Error", MB_OK);
		}
	}
}


void COpcUaServerDlg::OnBnClickedButton1()
{
	if (!m_bOpcServerStarted)
	{
		AfxMessageBox(_T("Please start OPCServer first!"));
		return;
	}
	try {

		//Create our address space using different methods
		Node objects = m_OpcServer->GetObjectsNode();

		auto newFolder = objects.AddFolder(m_serverIdx, "My Folder ABC");
		newFolder.AddFolder(m_serverIdx, "New Sub Folder 1");
		newFolder.AddFolder(m_serverIdx, "New Sub Folder 2");
		newFolder.AddFolder(m_serverIdx, "New Sub Folder 3");
		auto newFolder2 = newFolder.AddFolder(m_serverIdx, "New Sub Folder 4");
		newFolder2.AddFolder(m_serverIdx, "New Sub Folder 4");
		newFolder2.AddFolder(m_serverIdx, "New Sub Folder 5");
		newFolder2.AddFolder(m_serverIdx, "New Sub Folder 6");
		auto newFolder3 = objects.AddFolder(m_serverIdx, "My Folder DEF");
		newFolder3.AddFolder(m_serverIdx, "Hello Folder");
		objects.AddFolder(m_serverIdx, "My Folder Test");

		//Add a custom object with specific nodeid
		NodeId nid(99, m_serverIdx);
		QualifiedName qn("NewObject", m_serverIdx);
		Node newobject = objects.AddObject(nid, qn);
		
		//Add a variable and a property with auto-generated nodeid to our custom object
		Node myvar = newobject.AddVariable(m_serverIdx, "MyVariable", Variant(8));
		Node myprop = newobject.AddVariable(m_serverIdx, "MyProperty", Variant(8.8));
		Node mymethod = newobject.AddMethod(m_serverIdx, "MyMethod", MyMethod);

		//browse root node on server side
		Node root = m_OpcServer->GetRootNode();
		m_Logger->info("Root node is: {}", root);
		m_Logger->info("Children are:");

		for (Node node : root.GetChildren())
		{
			m_Logger->info("    {}", node);
		}

		SubClient clt;
		auto sub = m_OpcServer->CreateSubscription(100, clt);
		sub->SubscribeDataChange(myvar);

		//Now write values to address space and send events so clients can have some fun
		uint32_t counter = 0;
		myvar.SetValue(Variant(counter)); //will change value and trigger datachange event

		//Create event
		m_OpcServer->EnableEventNotification();
		Event ev(ObjectId::BaseEventType); //you should create your own type
		ev.Severity = 2;
		ev.SourceNode = ObjectId::Server;
		ev.SourceName = "Event from FreeOpcUA";
		ev.Time = DateTime::Current();

		m_Logger->info("Ctrl-C to exit");

		for (int i = 0; i < 10; i++)
		{
			myvar.SetValue(Variant(++counter)); //will change value and trigger datachange event
			std::stringstream ss;
			ss << "This is event number: " << counter;
			ev.Message = LocalizedText(ss.str());
			m_OpcServer->TriggerEvent(ev);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(nullptr, ex.what(), "Error", MB_OK);
	}
}


void COpcUaServerDlg::OnBnClickedButton2()
{
	TCHAR dest[MAX_PATH];
	DWORD length = GetModuleFileName(nullptr, dest, MAX_PATH);
	PathRemoveFileSpec(dest);	//deprecated but ok for now
	CString strPath(dest);
	strPath.Append(_T("\\output.txt"));
	ShellExecute(nullptr, _T("open"), strPath, nullptr, nullptr, SW_SHOW);
}


static inline bool lookintoNode(const Node& node, const CStringA& tag){
	for (const auto& item : node.GetChildren())
	{
		if (item.GetBrowseName().Name == tag.GetString())
		{
			item.SetValue(Variant(atoi(tag)));
			return true;
		}
			
		lookintoNode(item, tag);
	}
	return false;
};

void COpcUaServerDlg::OnBnClickedInsertOrUpdateTag()
{
	UpdateData();
	if (!m_strTag.Trim().IsEmpty() && !m_strTagValue.Trim().IsEmpty())
	{
		bool bFound = false;
		CStringA pszTag = CW2A(m_strTag);
		CStringA pszTagValue = CW2A(m_strTagValue);
		try {
			auto objects = m_OpcServer->GetObjectsNode();
			auto children = objects.GetChildren();

			for (const auto& childNode : children)
			{
				if (childNode.GetBrowseName().Name == pszTag.GetString())
				{
					childNode.SetValue(Variant(atoi(pszTagValue)));
					bFound = true;
					break;
				}
				if (lookintoNode(childNode, pszTag))
				{
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				NodeId nid(m_strTagId, m_serverIdx);
				QualifiedName qn(pszTag.GetString(), m_serverIdx);
				Node newobject = objects.AddObject(nid, qn);
				newobject.SetValue(Variant(atoi(pszTagValue)));
			}
		}
		catch (const std::exception& ex)
		{
			MessageBoxA(nullptr, ex.what(), "Error", MB_OK);
		}
	}
}

void COpcUaServerDlg::OnBnClickedClearList()
{
	m_treeServerLog.DeleteAllItems();
}


void COpcUaServerDlg::OnBnClickedGetObjectsNode()
{
	try {
		printNodes(m_treeServerLog, m_OpcServer->GetObjectsNode());
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(nullptr, ex.what(), "Error", MB_OK);
	}
}


void COpcUaServerDlg::OnBnClickedGetRootNode()
{
	try {
		printNodes(m_treeServerLog, m_OpcServer->GetRootNode());
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(nullptr, ex.what(), "Error", MB_OK);
	}
}


void COpcUaServerDlg::OnBnClickedGetServerNode()
{
	try {
		printNodes(m_treeServerLog, m_OpcServer->GetServerNode());
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(nullptr, ex.what(), "Error", MB_OK);
	}
}

static inline Node browseNode(const Node& node, const CStringA& tag) {
	for (const auto& item : node.GetChildren())
	{
		if (item.GetBrowseName().Name == tag.GetString())
		{
			return item;
		}

		if (auto nodeItem = browseNode(item, tag); nodeItem.IsValid())
			return nodeItem;
	}
	return Node();
};

void COpcUaServerDlg::OnSelchangeServerLogging(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	HTREEITEM selectedItem = m_treeServerLog.GetSelectedItem();
	if (selectedItem != nullptr)
	{
		CString strItem = m_treeServerLog.GetItemText(selectedItem);
		CStringA lpszText = CW2A(strItem);
		try {
			Node nodeItem;
			auto objects = m_OpcServer->GetObjectsNode();
			for (const auto& children : objects.GetChildren())
			{
				if (children.GetBrowseName().Name == lpszText.GetString())
				{
					nodeItem = children;
					break;
				}

				nodeItem = browseNode(children, lpszText);
				if (nodeItem.IsValid())
					break;
			}

			if (nodeItem.IsValid())
			{
				m_strTag = CA2W(nodeItem.GetBrowseName().Name.c_str());
				m_strTagId = nodeItem.GetId().GetIntegerIdentifier();
				auto dataType = nodeItem.GetDataType();
				if (!dataType.IsNul())
				{
					auto type = dataType.Type();
					switch (type)
					{
					case VariantType::INT32:
						m_strTagValue.Format(_T("%d"), dataType.As<int>());
						break;
					case VariantType::DOUBLE:
						m_strTagValue.Format(_T("%f"), dataType.As<double>());
						break;
					case VariantType::FLOAT:
						m_strTagValue.Format(_T("%f"), dataType.As<float>());
						break;
					case VariantType::STRING:
						m_strTagValue.Format(_T("%s"), CA2W(dataType.As<std::string>().c_str()).m_psz);
						break;
					}
				}
				UpdateData(FALSE);
			}
		}
		catch (const std::exception& ex)
		{
			(void)ex;
			//MessageBoxA(nullptr, ex.what(), "Error", MB_OK);
		}
	}
}
