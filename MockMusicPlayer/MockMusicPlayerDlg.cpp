
// MockMusicPlayerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MockMusicPlayer.h"
#include "MockMusicPlayerDlg.h"
#include "afxdialogex.h"

#include <ctime>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

// 文件对话框使用的过滤器
const TCHAR kszAudioFilter[] = _T("音频文件 (*.mp3;*.wav;*.wma)|*.mp3;*.wav;*.wma|所有文件 (*.*)|*.*||");
const TCHAR kszPlaylistFilter[] = _T("播放列表 (*.m3u;*.txt)|*.m3u;*.txt|所有文件 (*.*)|*.*||");
const TCHAR kszSaveFilter[] = _T("播放列表 (*.m3u)|*.m3u|文本文件 (*.txt)|*.txt||");

// 从完整路径提取文件名(含扩展名),用于列表显示与排序
CString GetFileNameOf(const CString& strPath)
{
	int nPos = strPath.ReverseFind(_T('\\'));
	const int nPosAlt = strPath.ReverseFind(_T('/'));
	if (nPosAlt > nPos)
		nPos = nPosAlt;
	return (nPos >= 0) ? strPath.Mid(nPos + 1) : strPath;
}

// 毫秒 → “mm:ss”
CString FormatTime(DWORD dwMs)
{
	const DWORD dwSeconds = dwMs / 1000;
	CString strTime;
	strTime.Format(_T("%02u:%02u"), dwSeconds / 60, dwSeconds % 60);
	return strTime;
}

// “当前时间 / 总时长”文本
CString FormatProgress(DWORD dwPosMs, DWORD dwLenMs)
{
	return FormatTime(dwPosMs) + _T(" / ") + FormatTime(dwLenMs);
}

// UTF-8 文本 → Unicode
CString Utf8ToWide(const CStringA& strUtf8)
{
	const int cbUtf8 = strUtf8.GetLength();
	const int cchWide = MultiByteToWideChar(CP_UTF8, 0, strUtf8, cbUtf8, nullptr, 0);
	CString strWide;
	if (cchWide > 0)
	{
		MultiByteToWideChar(CP_UTF8, 0, strUtf8, cbUtf8, strWide.GetBuffer(cchWide), cchWide);
		strWide.ReleaseBuffer(cchWide);
	}
	return strWide;
}

// Unicode → UTF-8 文本
CStringA WideToUtf8(const CString& strWide)
{
	const int cchWide = strWide.GetLength();
	const int cbUtf8 = WideCharToMultiByte(CP_UTF8, 0, strWide, cchWide, nullptr, 0, nullptr, nullptr);
	CStringA strUtf8;
	if (cbUtf8 > 0)
	{
		WideCharToMultiByte(CP_UTF8, 0, strWide, cchWide, strUtf8.GetBuffer(cbUtf8), cbUtf8, nullptr, nullptr);
		strUtf8.ReleaseBuffer(cbUtf8);
	}
	return strUtf8;
}

// 将 CFileException 的文本显示在错误消息框中
void ShowFileError(LPCTSTR pszPrefix, CFileException& fe)
{
	CString strError;
	LPTSTR pszError = strError.GetBuffer(256);
	fe.GetErrorMessage(pszError, 256);
	strError.ReleaseBuffer();
	AfxMessageBox(CString(pszPrefix) + strError, MB_ICONERROR);
}

} // namespace


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CMockMusicPlayerDlg 对话框



CMockMusicPlayerDlg::CMockMusicPlayerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MOCKMUSICPLAYER_DIALOG, pParent)
	, m_nCurrent(-1)
	, m_nVolume(100)
	, m_bPlaying(FALSE)
	, m_bSeeking(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMockMusicPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PLAYLIST, m_listPlaylist);
	DDX_Control(pDX, IDC_SLIDER_PROGRESS, m_sliderProgress);
	DDX_Control(pDX, IDC_SLIDER_VOLUME, m_sliderVolume);
}

BEGIN_MESSAGE_MAP(CMockMusicPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_ADD, &CMockMusicPlayerDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_REMOVE, &CMockMusicPlayerDlg::OnBnClickedBtnRemove)
	ON_BN_CLICKED(IDC_BTN_REMOVE_ALL, &CMockMusicPlayerDlg::OnBnClickedBtnRemoveAll)
	ON_BN_CLICKED(IDC_BTN_LOAD, &CMockMusicPlayerDlg::OnBnClickedBtnLoad)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CMockMusicPlayerDlg::OnBnClickedBtnSave)
	ON_BN_CLICKED(IDC_BTN_SORT_AZ, &CMockMusicPlayerDlg::OnBnClickedBtnSortAz)
	ON_BN_CLICKED(IDC_BTN_SORT_ZA, &CMockMusicPlayerDlg::OnBnClickedBtnSortZa)
	ON_BN_CLICKED(IDC_BTN_MOVE_UP, &CMockMusicPlayerDlg::OnBnClickedBtnMoveUp)
	ON_BN_CLICKED(IDC_BTN_MOVE_DOWN, &CMockMusicPlayerDlg::OnBnClickedBtnMoveDown)
	ON_BN_CLICKED(IDC_BTN_PLAY, &CMockMusicPlayerDlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_STOP, &CMockMusicPlayerDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CMockMusicPlayerDlg::OnBnClickedBtnPause)
	ON_BN_CLICKED(IDC_BTN_PREV, &CMockMusicPlayerDlg::OnBnClickedBtnPrev)
	ON_BN_CLICKED(IDC_BTN_REWIND, &CMockMusicPlayerDlg::OnBnClickedBtnRewind)
	ON_BN_CLICKED(IDC_BTN_FORWARD, &CMockMusicPlayerDlg::OnBnClickedBtnForward)
	ON_BN_CLICKED(IDC_BTN_NEXT, &CMockMusicPlayerDlg::OnBnClickedBtnNext)
	ON_BN_CLICKED(IDC_BTN_HELP, &CMockMusicPlayerDlg::OnBnClickedBtnHelp)
	ON_BN_CLICKED(IDC_BTN_QUIT, &CMockMusicPlayerDlg::OnBnClickedBtnQuit)
	ON_LBN_DBLCLK(IDC_LIST_PLAYLIST, &CMockMusicPlayerDlg::OnLbnDblclkListPlaylist)
END_MESSAGE_MAP()


// CMockMusicPlayerDlg 消息处理程序

BOOL CMockMusicPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时,框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);			// 设置小图标

	// TODO: 在此添加额外的初始化代码

	// 初始化滑块、默认播放选项与状态刷新定时器
	m_sliderProgress.SetRange(0, 1000);
	m_sliderVolume.SetRange(0, 100);
	m_sliderVolume.SetPos(m_nVolume);
	CheckDlgButton(IDC_CHECK_AUTO_NEXT, BST_CHECKED);
	srand(static_cast<UINT>(time(nullptr)));
	SetTimer(TIMER_PLAYBACK, 250, nullptr);

	return TRUE;  // 除非将焦点设置到控件,否则返回 TRUE
}

void CMockMusicPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMockMusicPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMockMusicPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMockMusicPlayerDlg::OnDestroy()
{
	KillTimer(TIMER_PLAYBACK);
	m_player.Close();
	CDialogEx::OnDestroy();
}

// 定时刷新进度显示,并检测曲目是否自然播完
void CMockMusicPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_PLAYBACK)
	{
		if (!m_bSeeking && m_player.IsOpen())
		{
			UpdateProgressUi();
			// 播放流程中模式变为 stopped 且不是暂停 → 自然播完
			if (m_bPlaying && !m_player.IsPlaying() && !m_player.IsPaused())
				OnPlaybackEnded();
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

// 进度/音量滑块的通知:拖动进度时仅刷新时间文本,松开后再定位;
// 音量滑块实时生效
void CMockMusicPlayerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);

	if (pScrollBar != nullptr && pScrollBar->GetSafeHwnd() == m_sliderVolume.GetSafeHwnd())
	{
		m_nVolume = m_sliderVolume.GetPos();
		m_player.SetVolume(m_nVolume);
		return;
	}

	if (pScrollBar == nullptr || pScrollBar->GetSafeHwnd() != m_sliderProgress.GetSafeHwnd() || !m_player.IsOpen())
		return;

	const DWORD dwLenMs = m_player.GetLengthMs();
	const DWORD dwPosMs = static_cast<DWORD>(static_cast<ULONGLONG>(m_sliderProgress.GetPos()) * dwLenMs / 1000);
	if (nSBCode == TB_THUMBTRACK)
	{
		m_bSeeking = TRUE;
		SetDlgItemText(IDC_STATIC_PROGRESS, FormatProgress(dwPosMs, dwLenMs));
	}
	else
	{
		m_bSeeking = FALSE;
		SeekToMs(dwPosMs);
	}
}


// CMockMusicPlayerDlg 播放列表

// 添加音频文件(可多选)
void CMockMusicPlayerDlg::OnBnClickedBtnAdd()
{
	CFileDialog dlg(TRUE, _T("mp3"), nullptr,
		OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
		kszAudioFilter);

	// 多选需要扩大 OPENFILENAME 的路径缓冲区
	std::vector<TCHAR> buffer(32768);
	dlg.GetOFN().lpstrFile = buffer.data();
	dlg.GetOFN().nMaxFile = static_cast<DWORD>(buffer.size());

	if (dlg.DoModal() != IDOK)
		return;

	int nFirst = -1;
	POSITION pos = dlg.GetStartPosition();
	while (pos != nullptr)
	{
		const int nIndex = AddFile(dlg.GetNextPathName(pos));
		if (nFirst < 0)
			nFirst = nIndex;
	}
	if (nFirst >= 0)
		RefreshPlaylist(nFirst);
}

// 移除选中条目(正在播放该条目时先停止播放)
void CMockMusicPlayerDlg::OnBnClickedBtnRemove()
{
	const int nIndex = m_listPlaylist.GetCurSel();
	if (nIndex == LB_ERR)
	{
		AfxMessageBox(_T("请先在列表中选择一个条目。"), MB_ICONINFORMATION);
		return;
	}

	if (nIndex == m_nCurrent)
		StopPlayback();
	m_arrPlaylist.RemoveAt(nIndex);
	if (m_nCurrent > nIndex)
		m_nCurrent--;
	RefreshPlaylist(nIndex);
}

// 全部移除(需确认)
void CMockMusicPlayerDlg::OnBnClickedBtnRemoveAll()
{
	if (m_arrPlaylist.GetCount() == 0)
		return;
	if (AfxMessageBox(_T("确定要清空播放列表吗?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;
	ClearPlaylist();
}

// 加载列表(m3u/文本文件,UTF-8,每行一个路径;替换当前列表)
void CMockMusicPlayerDlg::OnBnClickedBtnLoad()
{
	CFileDialog dlg(TRUE, _T("m3u"), nullptr,
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR, kszPlaylistFilter);
	if (dlg.DoModal() != IDOK)
		return;

	CFile file;
	CFileException fe;
	if (!file.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite, &fe))
	{
		ShowFileError(_T("无法打开列表文件:\n"), fe);
		return;
	}

	const UINT cbFile = static_cast<UINT>(file.GetLength());
	CStringA strData;
	const UINT cbRead = file.Read(strData.GetBuffer(static_cast<int>(cbFile)), cbFile);
	strData.ReleaseBuffer(static_cast<int>(cbRead));
	file.Close();

	// 跳过 UTF-8 BOM
	if (strData.GetLength() >= 3 && static_cast<BYTE>(strData[0]) == 0xEF
		&& static_cast<BYTE>(strData[1]) == 0xBB && static_cast<BYTE>(strData[2]) == 0xBF)
		strData = strData.Mid(3);

	const CString strContent = Utf8ToWide(strData);

	ClearPlaylist();
	int nFirst = -1;
	int nStart = 0;
	const int cch = strContent.GetLength();
	while (nStart < cch)
	{
		const int nEnd = strContent.Find(_T('\n'), nStart);
		const int nStop = (nEnd < 0) ? cch : nEnd;
		CString strLine = strContent.Mid(nStart, nStop - nStart);
		strLine.Trim(_T("\r \t"));
		nStart = nStop + 1;

		// 跳过空行与 m3u 扩展信息行;文件不存在的条目不加入
		if (strLine.IsEmpty() || strLine.GetAt(0) == _T('#'))
			continue;
		if (GetFileAttributes(strLine) == INVALID_FILE_ATTRIBUTES)
			continue;
		const int nIndex = AddFile(strLine);
		if (nFirst < 0)
			nFirst = nIndex;
	}

	if (nFirst < 0)
		AfxMessageBox(_T("列表文件中没有可用的音频文件。"), MB_ICONINFORMATION);
	RefreshPlaylist(nFirst);
}

// 保存列表为 m3u(UTF-8 文本,每行一个路径)
void CMockMusicPlayerDlg::OnBnClickedBtnSave()
{
	if (m_arrPlaylist.GetCount() == 0)
	{
		AfxMessageBox(_T("播放列表为空,没有可保存的内容。"), MB_ICONINFORMATION);
		return;
	}

	CFileDialog dlg(FALSE, _T("m3u"), _T("playlist"),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, kszSaveFilter);
	if (dlg.DoModal() != IDOK)
		return;

	CFile file;
	CFileException fe;
	if (!file.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite, &fe))
	{
		ShowFileError(_T("无法保存列表文件:\n"), fe);
		return;
	}

	for (int i = 0; i < static_cast<int>(m_arrPlaylist.GetCount()); i++)
	{
		const CStringA strLine = WideToUtf8(m_arrPlaylist.GetAt(i));
		file.Write(static_cast<LPCSTR>(strLine), strLine.GetLength());
		file.Write("\r\n", 2);
	}
	file.Close();
}

// 按文件名 A-Z / Z-A 排序
void CMockMusicPlayerDlg::OnBnClickedBtnSortAz()
{
	SortPlaylist(TRUE);
}

void CMockMusicPlayerDlg::OnBnClickedBtnSortZa()
{
	SortPlaylist(FALSE);
}

// 上移/下移选中条目
void CMockMusicPlayerDlg::OnBnClickedBtnMoveUp()
{
	MoveSelectedItem(-1);
}

void CMockMusicPlayerDlg::OnBnClickedBtnMoveDown()
{
	MoveSelectedItem(1);
}


// CMockMusicPlayerDlg 播放控制

// 播放:优先恢复暂停,否则播放选中项/当前曲/第一曲
void CMockMusicPlayerDlg::OnBnClickedBtnPlay()
{
	if (m_player.IsOpen() && m_player.IsPaused())
	{
		m_player.Resume();
		m_bPlaying = TRUE;
		return;
	}

	if (m_arrPlaylist.GetCount() == 0)
	{
		AfxMessageBox(_T("播放列表为空,请先添加音频文件。"), MB_ICONINFORMATION);
		return;
	}

	int nIndex = m_listPlaylist.GetCurSel();
	if (nIndex == LB_ERR)
		nIndex = (m_nCurrent >= 0) ? m_nCurrent : 0;
	PlayItem(nIndex);
}

// 停止并复位进度显示
void CMockMusicPlayerDlg::OnBnClickedBtnStop()
{
	StopPlayback();
}

// 暂停/继续
void CMockMusicPlayerDlg::OnBnClickedBtnPause()
{
	if (!m_player.IsOpen())
		return;
	if (m_player.IsPaused())
	{
		m_player.Resume();
		m_bPlaying = TRUE;
	}
	else
	{
		m_player.Pause();
	}
}

// 上一曲/下一曲(列表内循环切换)
void CMockMusicPlayerDlg::OnBnClickedBtnPrev()
{
	const int nCount = static_cast<int>(m_arrPlaylist.GetCount());
	if (nCount == 0)
		return;
	int nIndex = (m_nCurrent >= 0) ? m_nCurrent : m_listPlaylist.GetCurSel();
	if (nIndex < 0)
		nIndex = 0;
	PlayItem((nIndex - 1 + nCount) % nCount);
}

void CMockMusicPlayerDlg::OnBnClickedBtnNext()
{
	const int nCount = static_cast<int>(m_arrPlaylist.GetCount());
	if (nCount == 0)
		return;
	int nIndex = (m_nCurrent >= 0) ? m_nCurrent : m_listPlaylist.GetCurSel();
	if (nIndex < 0)
		nIndex = 0;
	PlayItem((nIndex + 1) % nCount);
}

// 快退/快进 5 秒
void CMockMusicPlayerDlg::OnBnClickedBtnRewind()
{
	SeekByMs(-SEEK_STEP_MS);
}

void CMockMusicPlayerDlg::OnBnClickedBtnForward()
{
	SeekByMs(SEEK_STEP_MS);
}

// 使用说明
void CMockMusicPlayerDlg::OnBnClickedBtnHelp()
{
	AfxMessageBox(
		_T("MockMusicPlayer 使用说明\n\n")
		_T("1. “添加项目...”可一次选择多个音频文件(mp3/wav/wma),双击列表项立即播放;\n")
		_T("2. 一曲播完后的选曲规则:勾选“随机播放”时随机选下一首;")
		_T("否则勾选“自动下一首”时顺序切换,到列表末尾后,若勾选“循环播放”则回到开头,否则停止;")
		_T("未勾选“自动下一首”时,若勾选“循环播放”则重播当前曲,否则停止;\n")
		_T("3. “<<”与“>>”为 5 秒快退/快进;拖动进度滑块定位播放位置,拖动音量滑块调节音量;\n")
		_T("4. “加载列表/保存列表”使用 m3u 格式(UTF-8 文本,每行一个文件路径)。"),
		MB_ICONINFORMATION);
}

// 退出
void CMockMusicPlayerDlg::OnBnClickedBtnQuit()
{
	EndDialog(IDCANCEL);
}

// 双击列表项播放
void CMockMusicPlayerDlg::OnLbnDblclkListPlaylist()
{
	const int nIndex = m_listPlaylist.GetCurSel();
	if (nIndex != LB_ERR)
		PlayItem(nIndex);
}


// CMockMusicPlayerDlg 播放列表与播放逻辑

// 用 m_arrPlaylist 重建列表框,nSelect >= 0 时选中该行(越界自动修正)
void CMockMusicPlayerDlg::RefreshPlaylist(int nSelect)
{
	m_listPlaylist.ResetContent();
	const int nCount = static_cast<int>(m_arrPlaylist.GetCount());
	for (int i = 0; i < nCount; i++)
		m_listPlaylist.AddString(GetFileNameOf(m_arrPlaylist.GetAt(i)));

	if (nSelect >= nCount)
		nSelect = nCount - 1;
	if (nSelect >= 0)
		m_listPlaylist.SetCurSel(nSelect);
}

// 停止播放并清空列表
void CMockMusicPlayerDlg::ClearPlaylist()
{
	StopPlayback();
	m_arrPlaylist.RemoveAll();
	m_nCurrent = -1;
	RefreshPlaylist(-1);
}

// 追加一个文件路径,返回新条目索引
int CMockMusicPlayerDlg::AddFile(LPCTSTR pszFilePath)
{
	m_arrPlaylist.Add(pszFilePath);
	return static_cast<int>(m_arrPlaylist.GetCount()) - 1;
}

// 按显示文件名排序(忽略大小写),正在播放的曲目索引跟随移动
void CMockMusicPlayerDlg::SortPlaylist(BOOL bAscending)
{
	CString strPlaying;
	const BOOL bHasCurrent = (m_nCurrent >= 0 && m_nCurrent < static_cast<int>(m_arrPlaylist.GetCount()));
	if (bHasCurrent)
		strPlaying = m_arrPlaylist.GetAt(m_nCurrent);

	const int nCount = static_cast<int>(m_arrPlaylist.GetCount());
	for (int i = 0; i < nCount - 1; i++)
	{
		for (int j = 0; j < nCount - 1 - i; j++)
		{
			const int nCmp = GetFileNameOf(m_arrPlaylist.GetAt(j)).CompareNoCase(GetFileNameOf(m_arrPlaylist.GetAt(j + 1)));
			const BOOL bSwap = bAscending ? (nCmp > 0) : (nCmp < 0);
			if (!bSwap)
				continue;
			const CString strTemp = m_arrPlaylist.GetAt(j);
			m_arrPlaylist.SetAt(j, m_arrPlaylist.GetAt(j + 1));
			m_arrPlaylist.SetAt(j + 1, strTemp);
		}
	}

	if (bHasCurrent)
		m_nCurrent = FindPlaylistIndex(strPlaying);
	RefreshPlaylist(m_nCurrent);
}

// 上移/下移选中条目(与相邻条目交换)
void CMockMusicPlayerDlg::MoveSelectedItem(int nDelta)
{
	const int nIndex = m_listPlaylist.GetCurSel();
	if (nIndex == LB_ERR)
	{
		AfxMessageBox(_T("请先在列表中选择一个条目。"), MB_ICONINFORMATION);
		return;
	}

	const int nTarget = nIndex + nDelta;
	if (nTarget < 0 || nTarget >= static_cast<int>(m_arrPlaylist.GetCount()))
		return;	// 已在边界

	const CString strTemp = m_arrPlaylist.GetAt(nIndex);
	m_arrPlaylist.SetAt(nIndex, m_arrPlaylist.GetAt(nTarget));
	m_arrPlaylist.SetAt(nTarget, strTemp);

	if (m_nCurrent == nIndex)
		m_nCurrent = nTarget;
	else if (m_nCurrent == nTarget)
		m_nCurrent = nIndex;
	RefreshPlaylist(nTarget);
}

// 按路径查找条目索引,未找到返回 -1
int CMockMusicPlayerDlg::FindPlaylistIndex(const CString& strPath)
{
	for (int i = 0; i < static_cast<int>(m_arrPlaylist.GetCount()); i++)
	{
		if (m_arrPlaylist.GetAt(i).CompareNoCase(strPath) == 0)
			return i;
	}
	return -1;
}

// 播放指定曲目:重新打开设备并应用当前音量
BOOL CMockMusicPlayerDlg::PlayItem(int nIndex)
{
	m_bPlaying = FALSE;

	if (nIndex < 0 || nIndex >= static_cast<int>(m_arrPlaylist.GetCount()))
		return FALSE;

	if (!m_player.Open(m_arrPlaylist.GetAt(nIndex)))
	{
		AfxMessageBox(_T("无法播放该文件:\n") + m_player.GetLastErrorMessage(), MB_ICONERROR);
		return FALSE;
	}
	m_player.SetVolume(m_nVolume);
	if (!m_player.Play())
	{
		AfxMessageBox(_T("无法播放该文件:\n") + m_player.GetLastErrorMessage(), MB_ICONERROR);
		m_player.Close();
		return FALSE;
	}

	m_nCurrent = nIndex;
	m_bPlaying = TRUE;
	m_listPlaylist.SetCurSel(nIndex);
	UpdateProgressUi();
	return TRUE;
}

// 停止播放并复位进度显示(保留列表与当前曲目索引)
void CMockMusicPlayerDlg::StopPlayback()
{
	m_bPlaying = FALSE;
	m_player.Close();
	ResetProgressUi();
}

// 定位到绝对位置,拖动前在播放则定位后继续播放
void CMockMusicPlayerDlg::SeekToMs(DWORD dwPosMs)
{
	if (!m_player.IsOpen())
		return;

	const DWORD dwLenMs = m_player.GetLengthMs();
	if (dwLenMs > 0 && dwPosMs > dwLenMs)
		dwPosMs = dwLenMs;

	const BOOL bWasPlaying = m_player.IsPlaying();
	if (!m_player.Seek(dwPosMs))
		return;
	if (bWasPlaying)
		m_player.Play();
	UpdateProgressUi();
}

// 相对当前位置快退/快进
void CMockMusicPlayerDlg::SeekByMs(int nDeltaMs)
{
	if (!m_player.IsOpen())
		return;

	int nPos = static_cast<int>(m_player.GetPositionMs()) + nDeltaMs;
	if (nPos < 0)
		nPos = 0;
	const DWORD dwLenMs = m_player.GetLengthMs();
	if (dwLenMs > 0 && static_cast<DWORD>(nPos) > dwLenMs)
		nPos = static_cast<int>(dwLenMs);
	SeekToMs(static_cast<DWORD>(nPos));
}

// 按当前位置刷新进度滑块与时间文本
void CMockMusicPlayerDlg::UpdateProgressUi()
{
	const DWORD dwPosMs = m_player.GetPositionMs();
	const DWORD dwLenMs = m_player.GetLengthMs();
	if (dwLenMs > 0)
		m_sliderProgress.SetPos(static_cast<int>(static_cast<ULONGLONG>(dwPosMs) * 1000 / dwLenMs));
	SetDlgItemText(IDC_STATIC_PROGRESS, FormatProgress(dwPosMs, dwLenMs));
}

// 复位进度显示为 00:00 / 00:00
void CMockMusicPlayerDlg::ResetProgressUi()
{
	m_sliderProgress.SetPos(0);
	SetDlgItemText(IDC_STATIC_PROGRESS, FormatProgress(0, 0));
}

// 一曲自然播完,按选项决定下一曲
void CMockMusicPlayerDlg::OnPlaybackEnded()
{
	m_bPlaying = FALSE;

	const int nCount = static_cast<int>(m_arrPlaylist.GetCount());
	if (nCount == 0)
	{
		StopPlayback();
		return;
	}

	const BOOL bLoop = IsOptionChecked(IDC_CHECK_LOOP);
	const BOOL bAutoNext = IsOptionChecked(IDC_CHECK_AUTO_NEXT);
	const BOOL bShuffle = IsOptionChecked(IDC_CHECK_SHUFFLE);

	if (bShuffle)
	{
		int nIndex = rand() % nCount;
		if (nCount > 1 && nIndex == m_nCurrent)
			nIndex = (nIndex + 1 + rand() % (nCount - 1)) % nCount;
		PlayItem(nIndex);
	}
	else if (bAutoNext)
	{
		if (m_nCurrent + 1 < nCount)
			PlayItem(m_nCurrent + 1);
		else if (bLoop)
			PlayItem(0);
		else
			StopPlayback();
	}
	else if (bLoop)
	{
		PlayItem(m_nCurrent);	// 未开自动连播时,循环即重播当前曲
	}
	else
	{
		StopPlayback();
	}
}
