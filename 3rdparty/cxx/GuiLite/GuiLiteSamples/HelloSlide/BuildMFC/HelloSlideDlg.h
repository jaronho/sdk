
// HelloSlideDlg.h : header file
//

#pragma once
extern void startHelloSlide(void* phy_fb, int width, int height, int color_bytes);
extern void sendTouch2HelloSlide(int x, int y, bool is_down);
extern void* getUiOfHelloSlide(int* width, int* height, bool force_update = false);

typedef struct
{
	unsigned int dwMsgId;
	unsigned int dwParam1;
	unsigned int dwParam2;
}OUTMSGINFO;

typedef struct tagMYBITMAPINFO {
	BITMAPINFOHEADER    bmiHeader;
	DWORD				biRedMask;
	DWORD				biGreenMask;
	DWORD				biBlueMask;
} MYBITMAPINFO;

// CHelloSlideDlg dialog
class CHelloSlideDlg : public CDialogEx
{
// Construction
public:
	CHelloSlideDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HELLOSLIDE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	void updateUI(CDC* pDC);
	CPoint pointMFC2GuiLite(CPoint point);
	static UINT ThreadHelloSlide(LPVOID pParam);
	static UINT ThreadRefreshUI(LPVOID pParam);
	static CHelloSlideDlg* m_the_dialog;
	CRect m_block_rect;
	int m_ui_width;
	int m_ui_height;
	int m_color_bytes;
	CBitmap		m_blockBmp;
	MYBITMAPINFO m_ui_bm_info;
	CDC			m_memDC;
	bool m_is_mouse_down;
};
