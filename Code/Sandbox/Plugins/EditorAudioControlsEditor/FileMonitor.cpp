// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include <CryString/CryPath.h>
#include "IAudioSystemEditor.h"
#include "AudioControlsEditorPlugin.h"
#include "AudioControlsEditorWindow.h"
#include "FileMonitor.h"

namespace ACE
{
//////////////////////////////////////////////////////////////////////////
CFileMonitor::CFileMonitor(CAudioControlsEditorWindow* window, int delay)
	: m_window(window)
	, m_delay(delay)
{
	setSingleShot(true);
	connect(this, &CFileMonitor::timeout, this, &CFileMonitor::ReloadData);
}

//////////////////////////////////////////////////////////////////////////
CFileMonitor::~CFileMonitor()
{
	stop();
	GetIEditor()->GetFileMonitor()->UnregisterListener(this);
}

//////////////////////////////////////////////////////////////////////////
void CFileMonitor::OnFileChange(const char* filename, EChangeType eType)
{
	start(m_delay);
}

//////////////////////////////////////////////////////////////////////////
void CFileMonitor::Disable()
{
	stop();
	GetIEditor()->GetFileMonitor()->UnregisterListener(this);
}

//////////////////////////////////////////////////////////////////////////
CFileMonitorSystem::CFileMonitorSystem(CAudioControlsEditorWindow* window, int delay)
	: CFileMonitor(window, delay)
{
	m_delayTimer = new QTimer();
	m_delayTimer->setSingleShot(true);
	connect(m_delayTimer, &QTimer::timeout, this, &CFileMonitorSystem::Enable);
}

//////////////////////////////////////////////////////////////////////////
void CFileMonitorSystem::Enable()
{
	GetIEditor()->GetFileMonitor()->RegisterListener(this, m_assetFolder, "xml");
}

//////////////////////////////////////////////////////////////////////////
void CFileMonitorSystem::EnableDelayed()
{
	m_delayTimer->start(m_delay);
}

//////////////////////////////////////////////////////////////////////////
void CFileMonitorSystem::ReloadData()
{
	m_window->ReloadSystemData();
}

//////////////////////////////////////////////////////////////////////////
void CFileMonitorMiddleware::Update()
{
	IAudioSystemEditor* const pAudioSystemImpl = CAudioControlsEditorPlugin::GetAudioSystemEditorImpl();

	if (pAudioSystemImpl != nullptr)
	{
		IImplementationSettings const* const pSettings = pAudioSystemImpl->GetSettings();

		if (pSettings != nullptr)
		{
			stop();
			m_monitorFolders.clear();

			int const gameFolderPathLength = (PathUtil::GetGameFolder() + CRY_NATIVE_PATH_SEPSTR).GetLength();

			string const& projectPath = pSettings->GetProjectPath();
			string const& projectPathSubstr = projectPath.substr(gameFolderPathLength);
			m_monitorFolders.emplace_back(projectPathSubstr.c_str());

			string const& soundBanksPath = pSettings->GetSoundBanksPath();
			string const& soundBanksPathSubstr = (soundBanksPath).substr(gameFolderPathLength);
			m_monitorFolders.emplace_back(soundBanksPathSubstr.c_str());

			string const& localizationPath = PathUtil::GetLocalizationFolder();
			m_monitorFolders.emplace_back(localizationPath.c_str());

			GetIEditor()->GetFileMonitor()->UnregisterListener(this);

			for (auto const& folder : m_monitorFolders)
			{
				GetIEditor()->GetFileMonitor()->RegisterListener(this, folder);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileMonitorMiddleware::ReloadData()
{
	m_window->ReloadMiddlewareData();
}
} // namespace ACE
