
// PathFinderTestDlg.h : ͷ�ļ�
//

#pragma once

#include "nav.h"
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};

#include <vector>
// CPathFinderTestDlg �Ի���
class CPathFinderTestDlg : public CDialogEx
{
// ����
public:
	CPathFinderTestDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_PATHFINDERTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPath();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);

	void DrawMap();
	void DrawBegin(CPoint& pos);
	void DrawOver(CPoint& pos);

	void DrawPath(struct vector3* path,int size);

public:

	struct nav_mesh_context* mesh_ctx;
	int xoffset;
	int yoffset;
	int scale;
	int polyBegin;
	vector3* vtBegin;
	int polyOver;
	vector3* vtOver;
	int sizePath;
	struct vector3* resultPath;
	struct lua_State* L;
	bool showTile;
	LARGE_INTEGER freq;
public:
	afx_msg void OnUpdateIddPathfindertestDialog(CCmdUI *pCmdUI);
	afx_msg void Straightline();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	
	afx_msg void OnIgnoreLine();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnEnChangeEdit4();
	afx_msg void OnIgnorePath();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnClose();
	afx_msg void OnLoadMesh();
	afx_msg void OnSaveMesh();
	afx_msg void OnCheck();
};
