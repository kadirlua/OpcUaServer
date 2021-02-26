
// OpcUaServerDlg.h : header file
//

#pragma once
#include <opc/ua/node.h>
#include <opc/ua/subscription.h>
#include <opc/ua/server/server.h>

// COpcUaServerDlg dialog
class COpcUaServerDlg : public CDialogEx
{
	struct FileDeleter {
		inline void operator()(FILE *f) const noexcept {
			if (f)
				fclose(f);
		}
	};

	using LogFile_ptr = std::unique_ptr<FILE, FileDeleter>;

// Construction
public:
	COpcUaServerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OPCUASERVER_DIALOG };
#endif
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	BOOL m_bOpcServerStarted{};
	LogFile_ptr m_file;
	OpcUa::Server::AddressSpace::UniquePtr m_StandartNameSpace;
	std::shared_ptr<spdlog::logger> m_Logger;
	std::unique_ptr<OpcUa::UaServer> m_OpcServer;
	uint32_t m_serverIdx{};

	//void LogCallback(const std::string& err_msg);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editServerEndpoint;
	CString m_strServerEndpoint;
	CEdit m_editServerUri;
	CString m_strServerUri;

	CEdit m_editTag;
	CString m_strTag;
	CEdit m_editTagId;
	int m_strTagId;
	CEdit m_editTagValue;
	CString m_strTagValue;

	afx_msg void OnBnClickedOk();
	CTreeCtrl m_treeServerLog;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedInsertOrUpdateTag();
	afx_msg void OnBnClickedClearList();
	afx_msg void OnBnClickedGetObjectsNode();
	afx_msg void OnBnClickedGetRootNode();
	afx_msg void OnBnClickedGetServerNode();
	afx_msg void OnSelchangeServerLogging(NMHDR* pNMHDR, LRESULT* pResult);
};


class SubClient : public OpcUa::SubscriptionHandler
{
public:
	void DataChange(uint32_t handle, const OpcUa::Node& node, const OpcUa::Variant& val, OpcUa::AttributeId attr) override;
};