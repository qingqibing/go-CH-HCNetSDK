// DlgAttendanceSummaryInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "ClientDemo.h"
#include "DlgAttendanceSummaryInfo.h"
#include "afxdialogex.h"


// CDlgAttendanceSummaryInfo 对话框

IMPLEMENT_DYNAMIC(CDlgAttendanceSummaryInfo, CDialog)

CDlgAttendanceSummaryInfo::CDlgAttendanceSummaryInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgAttendanceSummaryInfo::IDD, pParent)
{
    memset(&m_struSearchInfoCond, 0, sizeof(m_struSearchInfoCond));
    memset(&m_struAttendanceSummaryCfg, 0, sizeof(m_struAttendanceSummaryCfg));
    m_iEmployeeNo = 0;
    m_csName = _T("");
    m_startDate = COleDateTime::GetCurrentTime();
    m_startTime = COleDateTime::GetCurrentTime();
    m_stopDate = COleDateTime::GetCurrentTime();
    m_stopTime = COleDateTime::GetCurrentTime();
    m_lHandle = -1;
    m_bGetNext = FALSE;
    m_hGetInfoThread = NULL;
}

CDlgAttendanceSummaryInfo::~CDlgAttendanceSummaryInfo()
{
    char szLan[128] = { 0 };

    if (m_lHandle >= 0)
    {
        if (!NET_DVR_StopRemoteConfig(m_lHandle))
        {
            g_StringLanType(szLan, "考勤汇总信息查询停止失败", "get attendance summary info stop failed");
            AfxMessageBox(szLan);
            return;
        }
    }
}

void CDlgAttendanceSummaryInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_COMMAND, m_cmbCommand);
    DDX_Text(pDX, IDC_EDIT_EMPLOYEENO, m_iEmployeeNo);
    DDX_Text(pDX, IDC_EDIT_NAME, m_csName);
    DDX_Control(pDX, IDC_LIST_ATTENDANCE_SUMMARY_INFO, m_lstAttendanceSummaryInfo);
    DDX_DateTimeCtrl(pDX, IDC_START_DATE, m_startDate);
    DDX_DateTimeCtrl(pDX, IDC_START_TIME, m_startTime);
    DDX_DateTimeCtrl(pDX, IDC_STOP_DATE, m_stopDate);
    DDX_DateTimeCtrl(pDX, IDC_STOP_TIME, m_stopTime);
}

BOOL CDlgAttendanceSummaryInfo::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_cmbCommand.SetCurSel(0);
    char szLanTemp[128] = { 0 };
    m_lstAttendanceSummaryInfo.SetExtendedStyle(m_lstAttendanceSummaryInfo.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);
    g_StringLanType(szLanTemp, "工号", "Employee No");
    m_lstAttendanceSummaryInfo.InsertColumn(0, szLanTemp, LVCFMT_LEFT, 40, -1);
    g_StringLanType(szLanTemp, "姓名", "Name");
    m_lstAttendanceSummaryInfo.InsertColumn(1, szLanTemp, LVCFMT_LEFT, 40, -1);
    g_StringLanType(szLanTemp, "部门名称", "Department Name");
    m_lstAttendanceSummaryInfo.InsertColumn(2, szLanTemp, LVCFMT_LEFT, 80, -1);
    g_StringLanType(szLanTemp, "标准工作时间", "Work Standard");
    m_lstAttendanceSummaryInfo.InsertColumn(3, szLanTemp, LVCFMT_LEFT, 100, -1);
    g_StringLanType(szLanTemp, "实际工作时间", "Work Actual");
    m_lstAttendanceSummaryInfo.InsertColumn(4, szLanTemp, LVCFMT_LEFT, 100, -1);
    g_StringLanType(szLanTemp, "迟到次数", "Late Times");
    m_lstAttendanceSummaryInfo.InsertColumn(5, szLanTemp, LVCFMT_LEFT, 80, -1);
    g_StringLanType(szLanTemp, "迟到累计时间", "Late Minutes");
    m_lstAttendanceSummaryInfo.InsertColumn(6, szLanTemp, LVCFMT_LEFT, 100, -1);
    g_StringLanType(szLanTemp, "早退次数", "Leave Early Times");
    m_lstAttendanceSummaryInfo.InsertColumn(7, szLanTemp, LVCFMT_LEFT, 80, -1);
    g_StringLanType(szLanTemp, "早退累计时间", "Leave Early Minutes");
    m_lstAttendanceSummaryInfo.InsertColumn(8, szLanTemp, LVCFMT_LEFT, 100, -1);
    g_StringLanType(szLanTemp, "标准加班时间", "Overtime Standard");
    m_lstAttendanceSummaryInfo.InsertColumn(9, szLanTemp, LVCFMT_LEFT, 100, -1);
    g_StringLanType(szLanTemp, "实际加班时间", "Overtime Actual");
    m_lstAttendanceSummaryInfo.InsertColumn(10, szLanTemp, LVCFMT_LEFT, 100, -1);
    g_StringLanType(szLanTemp, "标准出勤天数", "Attendance Standard");
    m_lstAttendanceSummaryInfo.InsertColumn(11, szLanTemp, LVCFMT_LEFT, 100, -1);
    g_StringLanType(szLanTemp, "实际出勤天数", "Attendance Actual");
    m_lstAttendanceSummaryInfo.InsertColumn(12, szLanTemp, LVCFMT_LEFT, 100, -1);
    g_StringLanType(szLanTemp, "旷工天数", "Absent Days");
    m_lstAttendanceSummaryInfo.InsertColumn(13, szLanTemp, LVCFMT_LEFT, 80, -1);

    return TRUE;
}

DWORD WINAPI CDlgAttendanceSummaryInfo::GetAttendanceSummaryThread(LPVOID lpAttendanceSummaryInfo)
{
    CDlgAttendanceSummaryInfo* pThis = reinterpret_cast<CDlgAttendanceSummaryInfo*>(lpAttendanceSummaryInfo);
    int iRet = 0;
    char szLan[128] = { 0 };
    char szInfoBuf[128] = { 0 };
    while (pThis->m_bGetNext)
    {
        iRet = NET_DVR_GetNextRemoteConfig(pThis->m_lHandle, &pThis->m_struAttendanceSummaryCfg, sizeof(NET_DVR_ATTENDANCE_SUMMARY_CFG));
        if (iRet == NET_SDK_GET_NEXT_STATUS_SUCCESS)
        {
            //成功处理函数
            pThis->AddAttendanceSummaryCfg(&pThis->m_struAttendanceSummaryCfg);
        }
        else
        {
            if (iRet == NET_SDK_GET_NETX_STATUS_NEED_WAIT)
            {
                g_pMainDlg->AddLog(0, OPERATION_SUCC_T, "正在查询!");
                Sleep(200);
                continue;
            }
            else if (iRet == NET_SDK_GET_NEXT_STATUS_FINISH)
            {
                g_StringLanType(szLan, "考勤汇总信息查询结束!", "get attendance summary info finish");
                sprintf(szInfoBuf, "%s[Info Count:%d]", szLan, pThis->m_lstAttendanceSummaryInfo.GetItemCount());
                AfxMessageBox(szInfoBuf);
                break;
            }
            else if (iRet == NET_SDK_GET_NEXT_STATUS_FAILED)
            {
                g_StringLanType(szLan, "长连接考勤汇总信息查询失败", "get attendance summary info failed");
                AfxMessageBox(szLan);
                break;
            }
            else
            {
                g_StringLanType(szLan, "未知状态", "unknown status");
                AfxMessageBox(szLan);
                break;
            }
        }
    }

    return 0;
}

void CDlgAttendanceSummaryInfo::AddAttendanceSummaryCfg(LPNET_DVR_ATTENDANCE_SUMMARY_CFG lpInter)
{
    int iItemCount = m_lstAttendanceSummaryInfo.GetItemCount();
    m_lstAttendanceSummaryInfo.InsertItem(iItemCount, "");

    char szStr[128] = { 0 };
    int count = 0;
    //工号
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwEmployeeNo);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //姓名
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%s", lpInter->byName);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //部门名称
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%s", lpInter->byDepartmentName);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //标准工作时间（分钟）
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwWorkStandard);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //实际工作时间（分钟）
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwWorkActual);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //迟到次数
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwLateTimes);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //迟到累计时间（分钟）
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwLateMinutes);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //早退次数
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwLeaveEarlyTimes);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //早退累计时间（分钟）
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwLeaveEarlyMinutes);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //标准加班时间（分钟）
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwOvertimeStandard);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //实际加班时间（分钟）
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwOvertimeActual);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //标准出勤天数（天）
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwAttendanceStandard);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //实际出勤天数（天）
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwAttendanceActual);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
    //旷工天数（天）
    memset(szStr, 0, sizeof(szStr));
    sprintf(szStr, "%d", lpInter->dwAbsentDays);
    m_lstAttendanceSummaryInfo.SetItemText(iItemCount, count, szStr);
    count += 1;
}

BEGIN_MESSAGE_MAP(CDlgAttendanceSummaryInfo, CDialog)
    ON_BN_CLICKED(IDC_BTN_GET_ATTENDANCE_SUMMARY_INFO, &CDlgAttendanceSummaryInfo::OnBnClickedBtnGetAttendanceSummaryInfo)
    ON_BN_CLICKED(IDC_BTN_CLEAN_ATTENDANCE_SUMMARY_INFO, &CDlgAttendanceSummaryInfo::OnBnClickedBtnCleanAttendanceSummaryInfo)
    ON_CBN_SELCHANGE(IDC_COMBO_COMMAND, &CDlgAttendanceSummaryInfo::OnCbnSelchangeComboCommand)
END_MESSAGE_MAP()


// CDlgAttendanceSummaryInfo 消息处理程序


void CDlgAttendanceSummaryInfo::OnBnClickedBtnGetAttendanceSummaryInfo()
{
    // TODO:  在此添加控件通知处理程序代码
    UpdateData(TRUE);

    char szLan[128] = { 0 };
    memset(&m_struSearchInfoCond, 0, sizeof(m_struSearchInfoCond));
    memset(&m_struAttendanceSummaryCfg, 0, sizeof(m_struAttendanceSummaryCfg));
    m_struSearchInfoCond.byCommand = m_cmbCommand.GetCurSel();
    if (m_struSearchInfoCond.byCommand == 0)
    {
        m_struSearchInfoCond.dwEmployeeNo = m_iEmployeeNo;
        memcpy(m_struSearchInfoCond.byName, (LPCSTR)m_csName, m_csName.GetAllocLength());
    }
    m_struSearchInfoCond.struStartTime.wYear = m_startDate.GetYear();
    m_struSearchInfoCond.struStartTime.byMonth = m_startDate.GetMonth();
    m_struSearchInfoCond.struStartTime.byDay = m_startDate.GetDay();
    m_struSearchInfoCond.struStartTime.byHour = m_startTime.GetHour();
    m_struSearchInfoCond.struStartTime.byMinute = m_startTime.GetMinute();
    m_struSearchInfoCond.struStartTime.bySecond = m_startTime.GetSecond();
    m_struSearchInfoCond.struEndTime.wYear = m_stopDate.GetYear();
    m_struSearchInfoCond.struEndTime.byMonth = m_stopDate.GetMonth();
    m_struSearchInfoCond.struEndTime.byDay = m_stopDate.GetDay();
    m_struSearchInfoCond.struEndTime.byHour = m_stopTime.GetHour();
    m_struSearchInfoCond.struEndTime.byMinute = m_stopTime.GetMinute();
    m_struSearchInfoCond.struEndTime.bySecond = m_stopTime.GetSecond();

    if (m_lHandle >= 0)
    {
        if (!NET_DVR_StopRemoteConfig(m_lHandle))
        {
            g_StringLanType(szLan, "考勤汇总信息查询停止失败", "get attendance summary info stop failed");
            AfxMessageBox(szLan);
            return;
        }
    }

    m_lHandle = NET_DVR_StartRemoteConfig(m_lServerID, NET_DVR_GET_ATTENDANCE_SUMMARY_INFO, &m_struSearchInfoCond, sizeof(m_struSearchInfoCond), NULL, NULL);
    if (m_lHandle >= 0)
    {
        m_bGetNext = TRUE;
        DWORD dwThreadId;
        m_lstAttendanceSummaryInfo.DeleteAllItems();
        m_hGetInfoThread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(GetAttendanceSummaryThread), this, 0, &dwThreadId);
        g_StringLanType(szLan, "考勤汇总信息查询成功", "get attendance summary info succeed");
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, szLan);
    }
    else
    {
        m_bGetNext = FALSE;
        g_StringLanType(szLan, "考勤汇总信息查询失败", "get attendance summary info failed");
        AfxMessageBox(szLan);
        g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_GET_ATTENDANCE_SUMMARY_INFO");
        return;
    }

    UpdateData(FALSE);
}


void CDlgAttendanceSummaryInfo::OnBnClickedBtnCleanAttendanceSummaryInfo()
{
    // TODO:  在此添加控件通知处理程序代码
    m_lstAttendanceSummaryInfo.DeleteAllItems();
}


void CDlgAttendanceSummaryInfo::OnCbnSelchangeComboCommand()
{
    // TODO:  在此添加控件通知处理程序代码
    if (0 == m_cmbCommand.GetCurSel())
    {
        GetDlgItem(IDC_STATIC_EMPLOYEENO)->ShowWindow(true);
        GetDlgItem(IDC_STATIC_NAME)->ShowWindow(true);
        GetDlgItem(IDC_EDIT_EMPLOYEENO)->ShowWindow(true);
        GetDlgItem(IDC_EDIT_NAME)->ShowWindow(true);
    }
    else
    {
        GetDlgItem(IDC_STATIC_EMPLOYEENO)->ShowWindow(false);
        GetDlgItem(IDC_STATIC_NAME)->ShowWindow(false);
        GetDlgItem(IDC_EDIT_EMPLOYEENO)->ShowWindow(false);
        GetDlgItem(IDC_EDIT_NAME)->ShowWindow(false);
    }
}
