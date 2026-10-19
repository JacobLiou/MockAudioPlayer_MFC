// MciPlayer.cpp: MCI 播放引擎实现文件
//

#include "pch.h"
#include "MciPlayer.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

namespace {

// MCI 设备别名(别名按进程独立,固定名称即可)
const TCHAR kAlias[] = _T("mockmusic");

// MCI 命令中的路径需要引号,路径自身含引号时成对转义
CString QuotePath(LPCTSTR pszFilePath)
{
	CString strPath(pszFilePath);
	strPath.Replace(_T("\""), _T("\"\""));
	return _T("\"") + strPath + _T("\"");
}

} // namespace

CMciPlayer::~CMciPlayer()
{
	Close();
}

BOOL CMciPlayer::Open(LPCTSTR pszFilePath)
{
	Close();

	CString strCommand;
	strCommand.Format(_T("open %s type mpegvideo alias %s"), QuotePath(pszFilePath).GetString(), kAlias);
	if (Send(strCommand) != 0)
		return FALSE;

	m_strAlias = kAlias;
	Send(CString(_T("set ")) + kAlias + _T(" time format milliseconds"));
	return TRUE;
}

void CMciPlayer::Close()
{
	if (!IsOpen())
		return;
	Send(CString(_T("close ")) + kAlias);
	m_strAlias.Empty();
}

BOOL CMciPlayer::Play()
{
	return IsOpen() && Send(CString(_T("play ")) + kAlias) == 0;
}

BOOL CMciPlayer::Pause()
{
	return IsOpen() && Send(CString(_T("pause ")) + kAlias) == 0;
}

BOOL CMciPlayer::Resume()
{
	return IsOpen() && Send(CString(_T("resume ")) + kAlias) == 0;
}

BOOL CMciPlayer::Seek(DWORD dwPosMs)
{
	if (!IsOpen())
		return FALSE;
	CString strCommand;
	strCommand.Format(_T("seek %s to %u"), kAlias, dwPosMs);
	return Send(strCommand) == 0;
}

BOOL CMciPlayer::SetVolume(int nPercent)
{
	if (!IsOpen())
		return FALSE;
	if (nPercent < 0)
		nPercent = 0;
	if (nPercent > 100)
		nPercent = 100;
	CString strCommand;
	strCommand.Format(_T("setaudio %s volume to %d"), kAlias, nPercent * 10);
	return Send(strCommand) == 0;
}

DWORD CMciPlayer::GetLengthMs()
{
	return QueryStatus(_T("length"));
}

DWORD CMciPlayer::GetPositionMs()
{
	return QueryStatus(_T("position"));
}

BOOL CMciPlayer::IsPlaying()
{
	return GetMode() == _T("playing");
}

BOOL CMciPlayer::IsPaused()
{
	return GetMode() == _T("paused");
}

// 执行 status 命令并把返回文本转换为毫秒数值
DWORD CMciPlayer::QueryStatus(LPCTSTR pszItem)
{
	if (!IsOpen())
		return 0;
	TCHAR szResult[32] = { 0 };
	CString strCommand;
	strCommand.Format(_T("status %s %s"), kAlias, pszItem);
	Send(strCommand, szResult, _countof(szResult));
	return static_cast<DWORD>(_ttoi(szResult));
}

CString CMciPlayer::GetMode()
{
	TCHAR szMode[32] = { 0 };
	if (IsOpen())
	{
		CString strCommand;
		strCommand.Format(_T("status %s mode"), kAlias);
		Send(strCommand, szMode, _countof(szMode));
	}
	return szMode;
}

// 发送 MCI 命令,失败时记录错误文本
DWORD CMciPlayer::Send(LPCTSTR pszCommand, LPTSTR pszResult, UINT cchResult)
{
	const DWORD dwError = mciSendString(pszCommand, pszResult, cchResult, nullptr);
	if (dwError != 0)
	{
		TCHAR szText[256] = { 0 };
		mciGetErrorString(dwError, szText, _countof(szText));
		m_strLastError = szText;
	}
	return dwError;
}
