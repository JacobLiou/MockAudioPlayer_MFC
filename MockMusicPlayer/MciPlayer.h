// MciPlayer.h: MCI 播放引擎头文件
//

#pragma once

// CMciPlayer: 基于 Win32 多媒体控制接口(mciSendString)的简易播放引擎。
// 采用固定别名的 mpegvideo 设备,生命周期为:Open → Play/Pause/Resume/Seek → Close。
class CMciPlayer
{
public:
	CMciPlayer() = default;
	~CMciPlayer();

	BOOL Open(LPCTSTR pszFilePath);	// 关闭上一曲并打开新文件
	void Close();					// 关闭设备
	BOOL Play();					// 从当前位置播放
	BOOL Pause();
	BOOL Resume();
	BOOL Seek(DWORD dwPosMs);		// 定位到绝对位置(毫秒)
	BOOL SetVolume(int nPercent);	// 音量 0-100(映射为 MCI 的 0-1000)

	DWORD GetLengthMs();			// 曲目总时长(毫秒),失败返回 0
	DWORD GetPositionMs();			// 当前位置(毫秒)
	BOOL IsOpen() const { return !m_strAlias.IsEmpty(); }
	BOOL IsPlaying();
	BOOL IsPaused();

	CString GetLastErrorMessage() const { return m_strLastError; }

private:
	DWORD Send(LPCTSTR pszCommand, LPTSTR pszResult = nullptr, UINT cchResult = 0);
	DWORD QueryStatus(LPCTSTR pszItem);
	CString GetMode();

	CString m_strAlias;		// 设备别名,为空表示未打开
	CString m_strLastError;	// 最近一次 MCI 错误文本
};
