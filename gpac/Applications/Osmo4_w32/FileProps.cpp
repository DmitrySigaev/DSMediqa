// FileProps.cpp : implementation file
//

#include "stdafx.h"
#include "osmo4.h"
#include "FileProps.h"
#include "MainFrm.h"
#include <gpac/m4_decoder.h>
#include <gpac/m4_network.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileProps dialog


CFileProps::CFileProps(CWnd* pParent /*=NULL*/)
	: CDialog(CFileProps::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileProps)
	//}}AFX_DATA_INIT
}


void CFileProps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileProps)
	DDX_Control(pDX, IDC_VIEWSEL, m_ViewSel);
	DDX_Control(pDX, IDC_ODINFO, m_ODInfo);
	DDX_Control(pDX, IDC_ODTREE, m_ODTree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileProps, CDialog)
	//{{AFX_MSG_MAP(CFileProps)
	ON_NOTIFY(TVN_SELCHANGED, IDC_ODTREE, OnSelchangedOdtree)
	ON_BN_CLICKED(IDC_WORLD, OnWorld)
	ON_BN_CLICKED(IDC_VIEWSG, OnViewsg)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_VIEWSEL, OnSelchangeViewsel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileProps message handlers


#define FP_TIMER_ID	20

BOOL CFileProps::OnInitDialog() 
{
	CDialog::OnInitDialog();

	char sText[5000];
	sprintf(sText, "%s Properties", ((CMainFrame*)GetApp()->m_pMainWnd)->m_pPlayList->GetDisplayName());
	
	SetWindowText(sText);
	current_odm = NULL;

	m_ViewSel.InsertItem(0, "General");
	m_ViewSel.InsertItem(1, "Streams");
	m_ViewSel.InsertItem(2, "Playback");
	m_ViewSel.InsertItem(3, "Network");

	m_ODTree.SetIndent(0);
	RewriteODTree();
	SetTimer(FP_TIMER_ID, 500, NULL);

	return TRUE;
}


void CFileProps::WriteInlineTree(LPODMANAGER root_od, HTREEITEM parent)
{
	ODInfo info;
	WinGPAC *gpac = GetApp();

	/*browse all ODs*/
	u32 count = M4T_GetODCount(gpac->m_term, root_od);

	for (u32 i=0; i<count; i++) {
		LPODMANAGER odm = M4T_GetODManager(gpac->m_term, root_od, i);
		if (!odm) return;
		HTREEITEM item = m_ODTree.InsertItem("Object Descriptor", 0, 0, parent);
		m_ODTree.SetItemData(item, (DWORD) odm);
		/*browse all remotes*/
		while (1) {
			LPODMANAGER remote = M4T_GetRemoteOD(gpac->m_term, odm);
			if (!remote) break;
			if (M4T_GetODInfo(gpac->m_term, odm, &info) != M4OK) break;
			char sText[5000];
			sprintf(sText, "Remote OD (%s)", info.od->URLString);
			
			m_ODTree.SetItemText(item, sText);
			item = m_ODTree.InsertItem("Object Descriptor", 0, 0, item);
			m_ODTree.SetItemData(item, (DWORD) remote);
			odm = remote;
		}
		/*if inline propagate*/
		switch (M4T_IsInlineOD(gpac->m_term, odm)) {
		case 1:
			m_ODTree.SetItemText(item, "Inline Scene");
			WriteInlineTree(odm, item);
			break;
		case 2:
			m_ODTree.SetItemText(item, "Extern Proto Lib");
			WriteInlineTree(odm, item);
			break;
		default:
			break;
		}
	}
}

void CFileProps::RewriteODTree()
{
	ODInfo info;
	WinGPAC *gpac = GetApp();
	
	m_ODTree.DeleteAllItems();

	LPODMANAGER root_odm = M4T_GetRootOD(gpac->m_term);
	if (!root_odm) return;

	HTREEITEM root = m_ODTree.InsertItem("Root OD", 0, 0);
	m_ODTree.SetItemData(root, (DWORD) root_odm);

	/*browse all remotes*/
	while (1) {
		LPODMANAGER remote = M4T_GetRemoteOD(gpac->m_term, root_odm);
		if (!remote) break;
		if (M4T_GetODInfo(gpac->m_term, root_odm, &info) != M4OK) break;
		char sText[5000];
		sprintf(sText, "Remote OD (%s)", info.od->URLString);
		m_ODTree.SetItemText(root, sText);

		root = m_ODTree.InsertItem("Object Descriptor", 0, 0, root);
		m_ODTree.SetItemData(root, (DWORD) remote);
		root_odm = remote;
	}

	m_ODTree.SetItemText(root, "Inline Scene");
	WriteInlineTree(root_odm, root);
}

void CFileProps::OnSelchangedOdtree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;
	
	HTREEITEM item = m_ODTree.GetSelectedItem();
	LPODMANAGER odm = (LPODMANAGER) m_ODTree.GetItemData(item);
	if (!odm) return;
	
	SetInfo(odm);
}


void CFileProps::OnClose() 
{
	KillTimer(FP_TIMER_ID);
	DestroyWindow();
}

void CFileProps::OnDestroy() 
{
	CDialog::OnDestroy();
	delete this;
	((CMainFrame *)GetApp()->m_pMainWnd)->m_pProps = NULL;
}

void CFileProps::OnSelchangeViewsel(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SetInfo(current_odm);	
	*pResult = 0;
}

void CFileProps::SetInfo(LPODMANAGER odm)
{
	current_odm = odm;
	switch (m_ViewSel.GetCurSel()) {
	case 3: SetNetworkInfo(); break;
	case 2: SetDecoderInfo(); break;
	case 1: SetStreamsInfo(); break;
	default: SetGeneralInfo(); break;
	}
}

void CFileProps::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == FP_TIMER_ID) {
		switch (m_ViewSel.GetCurSel()) {
		case 3: SetNetworkInfo(); break;
		case 2: SetDecoderInfo(); break;
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CFileProps::SetGeneralInfo()
{
	char info[10000];
	char buf[1000];
	ODInfo odi;
	LPODMANAGER odm;
	u32 h, m, s, i, j;

	WinGPAC *gpac = GetApp();
	odm = current_odm;

	strcpy(info, "");
	if (!odm || M4T_GetODInfo(gpac->m_term, odm, &odi) != M4OK) return;

	sprintf(buf, "%sObject Descriptor ID %d\r\n", (odi.has_profiles) ? "Initial " : "", odi.od->objectDescriptorID);
	strcat(info, buf);
	if (odi.duration) {
		h = (u32) (odi.duration / 3600);
		m = (u32) (odi.duration / 60) - h*60;
		s = (u32) (odi.duration) - h*3600 - m*60;
		sprintf(buf, "Duration %02d:%02d:%02d\r\n", h, m, s);
		strcat(info, buf);
	} else {
		strcat(info, "Unknown duration\r\n");
	}
	if (odi.owns_service) {
		strcat(info, "Service Handler: ");
		strcat(info, odi.service_handler);
		strcat(info, "\r\n");
		strcat(info, "Service URL: ");
		strcat(info, odi.service_url);
		strcat(info, "\r\n");
	}
	
	if (odi.od->URLString) {
		strcat(info, "Remote OD - URL: ");
		strcat(info, odi.od->URLString);
		strcat(info, "\r\n");
	} 
	/*get OD content info*/
	if (odi.codec_name) {
		switch (odi.od_type) {
		case M4ST_VISUAL:
			sprintf(buf, "Video Object: Width %d - Height %d\r\n", odi.width, odi.height);
			strcat(info, buf);
			strcat(info, "Media Codec ");
			strcat(info, odi.codec_name);
			strcat(info, "\r\n");
			break;
		case M4ST_AUDIO:
			sprintf(buf, "Audio Object: Sample Rate %d - %d channels\r\n", odi.sample_rate, odi.num_channels);
			strcat(info, buf);
			strcat(info, "Media Codec ");
			strcat(info, odi.codec_name);
			strcat(info, "\r\n");
			break;
		case M4ST_PRIVATE_SCENE:
		case M4ST_SCENE:
			if (odi.width && odi.height) {
				sprintf(buf, "Scene Description: Width %d - Height %d\r\n", odi.width, odi.height);
			} else {
				sprintf(buf, "Scene Description: No size specified\r\n");
			}
			strcat(info, buf);
			strcat(info, "Scene Codec ");
			strcat(info, odi.codec_name);
			strcat(info, "\r\n");
			break;
		case M4ST_TEXT:
			if (odi.width && odi.height) {
				sprintf(buf, "Text Stream: Width %d - Height %d\r\n", odi.width, odi.height);
			} else {
				sprintf(buf, "Text Stream: No size specified\r\n");
			}
			strcat(info, buf);
			strcat(info, "Text Codec ");
			strcat(info, odi.codec_name);
			strcat(info, "\r\n");
			break;
		}
	}

	if (!ChainGetCount(odi.od->OCIDescriptors)) {
		m_ODInfo.SetWindowText(info);
		return;
	}

	strcat(info, "\r\nObject Content Information:\r\n");

	/*check OCI (not everything interests us) - FIXME: support for unicode*/
	for (i=0; i<ChainGetCount(odi.od->OCIDescriptors); i++) {
		Descriptor *desc = (Descriptor *) ChainGetEntry(odi.od->OCIDescriptors, i);
		switch (desc->tag) {
		case SegmentDescriptor_Tag:
		{
			SegmentDescriptor *sd = (SegmentDescriptor *) desc;
			strcat(info, "\r\nSegment Descriptor:\r\n");
			sprintf(buf, "Name: %s - start time %g sec - duration %g sec\r\n", sd->SegmentName, sd->startTime, sd->Duration);
			strcat(info, buf);
		}
			break;
		case ContentCreatorName_Tag:
		{
			ContentCreatorNameDescriptor *ccn = (ContentCreatorNameDescriptor *)desc;
			strcat(info, "\r\nContent Creators:\r\n");
			for (j=0; j<ChainGetCount(ccn->ContentCreators); j++) {
				contentCreatorInfo *ci = (contentCreatorInfo *) ChainGetEntry(ccn->ContentCreators, j);
				if (!ci->isUTF8) continue;
				strcat(info, "\t");
				strcat(info, ci->contentCreatorName);
				strcat(info, "\r\n");
			}
		}
			break;

		case ShortTextualDescriptor_Tag:
			{
				ShortTextualDescriptor *std = (ShortTextualDescriptor *)desc;
				strcat(info, "\r\n");
				strcat(info, std->eventName);
				strcat(info, ": ");
				strcat(info, std->eventText);
				strcat(info, "\r\n");
			}
			break;
		/*todo*/
		case ContentCreationDate_Tag:
			break;
		default:
			break;
		}

	}

	m_ODInfo.SetWindowText(info);
}

void CFileProps::OnWorld() 
{
	CString wit;
	char *str;
	Chain *descs;
	WinGPAC *gpac = GetApp();

	descs = NewChain();
	str = M4T_GetWorldInfo(gpac->m_term, current_odm, descs);
	if (!str) {
		MessageBox("No World Info available", "Sorry!");
		return;
	}

	wit = "";
	for (u32 i=0; i<ChainGetCount(descs); i++) {
		char *d = (char *) ChainGetEntry(descs, i);
		wit += d;
		wit += "\n";
		free(d);
	}
	MessageBox(wit, str);
	free(str);
	DeleteChain(descs);
}

void CFileProps::OnViewsg() 
{
	char szOutFile[M4_MAX_PATH];
	WinGPAC *gpac = GetApp();

	strcpy(szOutFile, gpac->m_config_dir);
	strcat(szOutFile, "scene_dump");

	M4Err e = M4T_DumpSceneGraph(gpac->m_term, (char *) szOutFile, gpac->m_ViewXMTA, 0, current_odm);

	if (e) {
		MessageBox(M4ErrToString(e), "Error while dumping");
	} else {
		ShellExecute(NULL, "open", szOutFile, NULL, NULL, SW_SHOWNORMAL);
	}
}

void CFileProps::SetDecoderInfo()
{
	ODInfo odi;
	char buf[1000], info[2000];
	u32 h, m, s;
	WinGPAC *gpac = GetApp();

	sprintf(info, "");
	m_ODInfo.SetWindowText("");

	if (!current_odm || M4T_GetODInfo(gpac->m_term, current_odm, &odi)) return;

	strcat(info, "Status: ");
	switch (odi.status) {
	case 0:
	case 1:
	case 2:
		h = (u32) (odi.current_time / 3600);
		m = (u32) (odi.current_time / 60) - h*60;
		s = (u32) (odi.current_time) - h*3600 - m*60;
		sprintf(buf, "%s\r\nObject Time: %02d:%02d:%02d\r\n", (odi.status==0) ? "Stopped" : (odi.status==1) ? "Playing" : "Paused", h, m, s);
		strcat(info, buf);
		break;
	case 3:
		strcat(info, "Not Setup");
		m_ODInfo.SetWindowText(info);
		return;
	default:
		strcat(info, "Setup Failed");
		m_ODInfo.SetWindowText(info);
		return;
	}
	/*get clock drift*/
	sprintf(buf, "Clock drift: %d ms\r\n", odi.clock_drift);
	strcat(info, buf);
	/*get buffering*/
	if (odi.buffer>=0) {
		sprintf(buf, "Buffering Time: %d ms\r\n", odi.buffer);
		strcat(info, buf);
	} else if (odi.buffer==-1) {
		strcat(info, "Not buffering\r\n");
	} else {
		strcat(info, "Not Playing\r\n");
	}
	/*get DB occupation*/
	if (odi.buffer>=0) {
		sprintf(buf, "Decoding Buffer: %d Access Units\r\n", odi.db_unit_count);
		strcat(info, buf);
	}
	/*get CB occupation*/
	if (odi.cb_max_count) {
		sprintf(buf, "Composition Memory: %d/%d Units\r\n", odi.cb_unit_count, odi.cb_max_count);
		strcat(info, buf);
	}

	Float avg_dec_time = 0;
	if (odi.nb_dec_frames) { 
		avg_dec_time = (Float) odi.total_dec_time; 
		avg_dec_time /= odi.nb_dec_frames; 
	}
	sprintf(buf, "Bitrate over last second: %d kbps\r\nMax bitrate over one second: %d kbps\r\nAverage Decoding Time %.2f ms (%d max)\r\nTotal decoded frames %d\r\n", 
		(u32) odi.avg_bitrate/1024, odi.max_bitrate/1024, avg_dec_time, odi.max_dec_time, odi.nb_dec_frames);
	strcat(info, buf);

	m_ODInfo.SetWindowText(info);
}


void CFileProps::SetStreamsInfo()
{
	u32 i, count;
	char info[10000];
	char buf[1000];
	ODInfo odi;
	LPODMANAGER odm;
	Bool is_media;

	WinGPAC *gpac = GetApp();
	odm = current_odm;

	strcpy(info, "");
	m_ODInfo.SetWindowText("");

	if (!odm || M4T_GetODInfo(gpac->m_term, odm, &odi) != M4OK) return;


	if (odi.has_profiles) {
		strcat(info, "MPEG-4 Profiles and Levels:\r\n");
		sprintf(buf, "\tOD Profile@Level %d\r\n", odi.OD_pl);
		strcat(info, buf);
		sprintf(buf, "\tScene Profile@Level %d\r\n", odi.scene_pl);
		strcat(info, buf);
		sprintf(buf, "\tGraphics Profile@Level %d\r\n", odi.graphics_pl);
		strcat(info, buf);
		sprintf(buf, "\tAudio Profile@Level %d\r\n", odi.audio_pl);
		strcat(info, buf);
		sprintf(buf, "\tVisual Profile@Level %d\r\n", odi.visual_pl);
		strcat(info, buf);
		sprintf(buf, "\tInline Content Profiled %s\r\n", odi.inline_pl ? "yes" : "no");
		strcat(info, buf);
		strcat(info, "\r\n");
	}
	is_media = 0;
	count = ChainGetCount(odi.od->ESDescriptors);

	for (i=0; i<count; i++) {
		ESDescriptor *esd = (ESDescriptor *) ChainGetEntry(odi.od->ESDescriptors, i);

		sprintf(buf, "\t** Stream ID %d - Clock ID %d **\r\n", esd->ESID, esd->OCRESID);
		strcat(info, buf);
		if (esd->dependsOnESID) {
			sprintf(buf, "Depends on Stream ID %d for decoding\r\n", esd->dependsOnESID);
			strcat(info, buf);
		}
		switch (esd->decoderConfig->streamType) {
		case M4ST_OD:
			sprintf(buf, "OD Stream - version %d\r\n", esd->decoderConfig->objectTypeIndication);
			strcat(info, buf);
			break;
		case M4ST_OCR:
			sprintf(buf, "OCR Stream\r\n");
			strcat(info, buf);
			break;
		case M4ST_SCENE:
			sprintf(buf, "Scene Description Stream - version %d\r\n", esd->decoderConfig->objectTypeIndication);
			strcat(info, buf);
			break;
		case M4ST_PRIVATE_SCENE:
			sprintf(buf, "GPAC Private Scene Description Stream\r\n");
			strcat(info, buf);
			break;
		case M4ST_VISUAL:
			is_media = 1;
			strcat(info, "Visual Stream - media type: ");
			switch (esd->decoderConfig->objectTypeIndication) {
			case 0x20:
				strcat(info, "MPEG-4\r\n");
				break;
			case 0x60:
				strcat(info, "MPEG-2 Simple Profile\r\n");
				break;
			case 0x61:
				strcat(info, "MPEG-2 Main Profile\r\n");
				break;
			case 0x62:
				strcat(info, "MPEG-2 SNR Profile\r\n");
				break;
			case 0x63:
				strcat(info, "MPEG-2 Spatial Profile\r\n");
				break;
			case 0x64:
				strcat(info, "MPEG-2 High Profile\r\n");
				break;
			case 0x65:
				strcat(info, "MPEG-2 422 Profile\r\n");
				break;
			case 0x6A:
				strcat(info, "MPEG-1\r\n");
				break;
			case 0x6C:
				strcat(info, "JPEG\r\n");
				break;
			case 0x6D:
				strcat(info, "PNG\r\n");
				break;
			default:
				sprintf(buf, "Private/Unknown (0x%x)\r\n", esd->decoderConfig->objectTypeIndication);
				strcat(info, buf);
				break;
			}
			break;

		case M4ST_AUDIO:
			is_media = 1;
			strcat(info, "Audio Stream - media type: ");
			switch (esd->decoderConfig->objectTypeIndication) {
			case 0x40:
				strcat(info, "MPEG-4\r\n");
				break;
			case 0x66:
				strcat(info, "MPEG-2 AAC Main Profile\r\n");
				break;
			case 0x67:
				strcat(info, "MPEG-2 AAC LowComplexity Profile\r\n");
				break;
			case 0x68:
				strcat(info, "MPEG-2 AAC Scalable Sampling Rate Profile\r\n");
				break;
			case 0x69:
			case 0x6B:
				strcat(info, "MPEG-1 / MPEG-2 Audio\r\n");
				break;
			default:
				sprintf(buf, "Private/Unknown (0x%x)\r\n", esd->decoderConfig->objectTypeIndication);
				strcat(info, buf);
				break;
			}
			break;
		case M4ST_MPEG7:
			sprintf(buf, "MPEG-7 Stream - version %d\r\n", esd->decoderConfig->objectTypeIndication);
			strcat(info, buf);
			break;
		case M4ST_IPMP:
			sprintf(buf, "IPMP Stream - version %d\r\n", esd->decoderConfig->objectTypeIndication);
			strcat(info, buf);
			break;
		case M4ST_OCI:
			sprintf(buf, "OCI Stream - version %d\r\n", esd->decoderConfig->objectTypeIndication);
			strcat(info, buf);
			break;
		case M4ST_MPEGJ:
			sprintf(buf, "MPEGJ Stream - version %d\r\n", esd->decoderConfig->objectTypeIndication);
			strcat(info, buf);
			break;
		case M4ST_INTERACT:
			sprintf(buf, "User Interaction Stream - version %d\r\n", esd->decoderConfig->objectTypeIndication);
			strcat(info, buf);
			break;
		case M4ST_TEXT:
			sprintf(buf, "3GPP/MPEG-4 Timed Text - version %d\r\n", esd->decoderConfig->objectTypeIndication);
			strcat(info, buf);
			break;
		default:
			sprintf(buf, "Private/Unknown (StreamType 0x%x OTI 0x%x)\r\n", esd->decoderConfig->streamType, esd->decoderConfig->objectTypeIndication);
			strcat(info, buf);
			break;
		}

		sprintf(buf, "Buffer Size %d\r\nAverage Bitrate %d bps\r\nMaximum Bitrate %d bps\r\n", esd->decoderConfig->bufferSizeDB, esd->decoderConfig->avgBitrate, esd->decoderConfig->maxBitrate);
		strcat(info, buf);
		if (esd->slConfig->predefined==SLPredef_SkipSL) {
			sprintf(buf, "Not using MPEG-4 Synchronization Layer\r\n");
		} else {
			sprintf(buf, "Stream Clock Resolution %d\r\n", esd->slConfig->timestampResolution);
		}
		strcat(info, buf);
		if (esd->URLString) {
			sprintf(buf, "Stream Location: %s\r\n", esd->URLString);
			strcat(info, buf);
		}

		/*check language*/
		if (ChainGetCount(esd->langDesc)) {
			LanguageDescriptor *ld = (LanguageDescriptor *)ChainGetEntry(esd->langDesc, 0);
			ld->langCode;
			sprintf(buf, "Stream Language: %c%c%c\r\n", (char) ((ld->langCode>>16)&0xFF), (char) ((ld->langCode>>8)&0xFF), (char) (ld->langCode & 0xFF) );
			strcat(info, buf);
		}
		strcat(info, "\r\n");
	}
	
	m_ODInfo.SetWindowText(info);
}

void CFileProps::SetNetworkInfo()
{
	char info[10000];
	char buf[10000];
	u32 id;
	NetStatCommand com;
	ODInfo odi;
	u32 d_enum, nb_streams;
	M4Err e;
	LPODMANAGER odm;
	WinGPAC *gpac = GetApp();
	odm = current_odm;

	strcpy(info, "");
	m_ODInfo.SetWindowText("");

	if (!odm || M4T_GetODInfo(gpac->m_term, odm, &odi) != M4OK) return;

	if (odi.owns_service) {
		const char *url;
		u32 done, total;
		Float bps;
		strcpy(info, "Current Downloads in service:\r\n");
		d_enum = 0;
		while (M4T_GetDownloadInfo(gpac->m_term, odm, &d_enum, &url, &done, &total, &bps)) {
			if (total) {
				sprintf(buf, "%s: %d / %d bytes (%.2f %%) - %.2f kBps\r\n", url, done, total, (100.0f*done)/total, bps/1024);
			} else {
				sprintf(buf, "%s: %.2f kBps\r\n", url, bps/1024);
			}
			strcat(info, buf);
		}
		if (!d_enum) strcpy(info, "No Downloads in service\r\n");
		strcat(info, "\r\n");
	}

	d_enum = 0;
	nb_streams = 0;
	while (M4T_GetChannelNetInfo(gpac->m_term, odm, &d_enum, &id, &com, &e)) {
		if (e) continue;
		if (!com.bw_down && !com.bw_up) continue;
		nb_streams ++;

		sprintf(buf, "Stream ID %d statistics:\r\n", id);
		strcat(info, buf);
		if (com.multiplex_port) {
			sprintf(buf, "\tMultiplex Port %d - multiplex ID %d\r\n", com.multiplex_port, com.port);
		} else {
			sprintf(buf, "\tPort %d\r\n", com.port);
		}
		strcat(info, buf);
		sprintf(buf, "\tPacket Loss Percentage: %.4f\r\n", com.pck_loss_percentage);
		strcat(info, buf);
		sprintf(buf, "\tDown Bandwidth: %.3f kbps\r\n", ((Float)com.bw_down) / 1024);
		strcat(info, buf);
		if (com.bw_up) {
			sprintf(buf, "\tUp Bandwidth: %d bps\r\n", com.bw_up);
			strcat(info, buf);
		}
		if (com.ctrl_port) {
			if (com.multiplex_port) {
				sprintf(buf, "\tControl Multiplex Port: %d - Control Multiplex ID %d\r\n", com.multiplex_port, com.ctrl_port);
			} else {
				sprintf(buf, "\tControl Port: %d\r\n", com.ctrl_port);
			}
			strcat(info, buf);
			sprintf(buf, "\tControl Down Bandwidth: %d bps\r\n", com.ctrl_bw_down);
			strcat(info, buf);
			sprintf(buf, "\tControl Up Bandwidth: %d bps\r\n", com.ctrl_bw_up);
			strcat(info, buf);
		}
		strcat(info, "\r\n");
	}
	if (!nb_streams) strcat(info, "No network streams in this object\r\n");

	m_ODInfo.SetWindowText(info);
}
