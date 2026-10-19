
// MockMusicPlayerDlg.h: 头文件
//

#pragma once

#include "MciPlayer.h"

// 定时器 ID 与快进快退步长
enum
{
	TIMER_PLAYBACK = 1,		// 播放状态刷新定时器
	SEEK_STEP_MS = 5000,	// “<<”“>>”步长(毫秒)
};

// CMockMusicPlayerDlg 对话框
class CMockMusicPlayerDlg : public CDialogEx
{
// 构造
public:
	CMockMusicPlayerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MOCKMUSICPLAYER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 控件绑定
	CListBox m_listPlaylist;
	CSliderCtrl m_sliderProgress;
	CSliderCtrl m_sliderVolume;

	// 播放列表与播放状态
	CMciPlayer m_player;			// MCI 播放引擎
	CStringArray m_arrPlaylist;		// 曲目完整路径,顺序与列表框一致
	int m_nCurrent;					// 正在播放的曲目索引,-1 表示未播放
	int m_nVolume;					// 音量百分比 0-100
	BOOL m_bPlaying;				// 曲目处于播放流程中(含暂停)
	BOOL m_bSeeking;				// 用户正在拖动进度滑块

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnRemove();
	afx_msg void OnBnClickedBtnRemoveAll();
	afx_msg void OnBnClickedBtnLoad();
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnBnClickedBtnSortAz();
	afx_msg void OnBnClickedBtnSortZa();
	afx_msg void OnBnClickedBtnMoveUp();
	afx_msg void OnBnClickedBtnMoveDown();
	afx_msg void OnBnClickedBtnPlay();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedBtnPause();
	afx_msg void OnBnClickedBtnPrev();
	afx_msg void OnBnClickedBtnRewind();
	afx_msg void OnBnClickedBtnForward();
	afx_msg void OnBnClickedBtnNext();
	afx_msg void OnBnClickedBtnHelp();
	afx_msg void OnBnClickedBtnQuit();
	afx_msg void OnLbnDblclkListPlaylist();
	DECLARE_MESSAGE_MAP()

// 播放列表与播放逻辑
private:
	BOOL IsOptionChecked(UINT nId) { return IsDlgButtonChecked(nId) == BST_CHECKED; }
	void RefreshPlaylist(int nSelect);
	void ClearPlaylist();
	int AddFile(LPCTSTR pszFilePath);
	void SortPlaylist(BOOL bAscending);
	void MoveSelectedItem(int nDelta);
	int FindPlaylistIndex(const CString& strPath);
	BOOL PlayItem(int nIndex);
	void StopPlayback();
	void SeekToMs(DWORD dwPosMs);
	void SeekByMs(int nDeltaMs);
	void UpdateProgressUi();
	void ResetProgressUi();
	void OnPlaybackEnded();
};
