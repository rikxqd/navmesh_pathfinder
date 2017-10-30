
// PathFinderTestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PathFinderTest.h"
#include "PathFinderTestDlg.h"
#include "afxdialogex.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"  
#include "lua-nav.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPathFinderTestDlg �Ի���



CPathFinderTestDlg::CPathFinderTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPathFinderTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CPathFinderTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPathFinderTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CPathFinderTestDlg::OnPath)
	ON_WM_MOUSEACTIVATE()
	ON_UPDATE_COMMAND_UI(IDD_PATHFINDERTEST_DIALOG, &CPathFinderTestDlg::OnUpdateIddPathfindertestDialog)
	ON_BN_CLICKED(IDC_BUTTON2, &CPathFinderTestDlg::Straightline)
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON3, &CPathFinderTestDlg::OnIgnoreLine)
	ON_EN_CHANGE(IDC_EDIT2, &CPathFinderTestDlg::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT3, &CPathFinderTestDlg::OnEnChangeEdit3)
	ON_EN_CHANGE(IDC_EDIT4, &CPathFinderTestDlg::OnEnChangeEdit4)
	ON_BN_CLICKED(IDC_BUTTON4, &CPathFinderTestDlg::OnIgnorePath)
	ON_WM_RBUTTONUP()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON5, &CPathFinderTestDlg::OnLoadMesh)
	ON_BN_CLICKED(IDC_BUTTON6, &CPathFinderTestDlg::OnSaveMesh)
	ON_BN_CLICKED(IDC_CHECK1, &CPathFinderTestDlg::OnCheck)
END_MESSAGE_MAP()


// CPathFinderTestDlg ��Ϣ�������

BOOL CPathFinderTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//_CrtSetBreakAlloc(3850);
	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	rapidjson::Document _config;
	FILE* file = fopen("mesh.json","r");
	assert(file != NULL);
	fseek(file,0,SEEK_END);
	int len = ftell(file);
	char* json = (char*)malloc(len+1);
	memset(json,0,len+1);
	rewind(file);
	fread(json,1,len,file);
	fclose(file);
	_config.Parse(json);

	const rapidjson::Value& v = _config["v"];
	double** v_ptr = (double**)malloc(sizeof(*v_ptr) * v.Size());
	for (int i = 0 ;i < v.Size();i++)
	{
		v_ptr[i] = (double*)malloc(sizeof(double) * 3);
		const rapidjson::Value& tmp = v[i];
		v_ptr[i][0] = tmp[0].GetDouble();
		v_ptr[i][1] = tmp[1].GetDouble();
		v_ptr[i][2] = tmp[2].GetDouble();
	}

	const rapidjson::Value& p = _config["p"];
	int** p_ptr = (int**)malloc(sizeof(*p_ptr) * p.Size());
	int a = p.Size();
	for (int i = 0;i < p.Size();i++)
	{
		const rapidjson::Value& tmp = p[i];
		p_ptr[i] = (int*)malloc(sizeof(int) *(tmp.Size()));
		for (int j = 0;j < tmp.Size();j++)
		{
			p_ptr[i][j] = tmp[j].GetInt();
		}
	}
	free(json);
	struct vector3 start,over;
	start.x = 16;
	start.z = 7;
	over.x = 44;
	over.z = 85;
	this->mesh_ctx  = load_mesh(v_ptr,v.Size(),p_ptr, p.Size());
	xoffset = 100;
	yoffset = 50;
	polyBegin = -1;
	polyOver = -1;
	scale = 4;
	vtOver = vtBegin = NULL;
	
	for (int i = 0; i < v.Size(); i++)
	{
		free(v_ptr[i]);
	}
	free(v_ptr);
	for (int i = 0; i < p.Size(); i++)
	{
		free(p_ptr[i]);
	}
	free(p_ptr);

	CString str;
	str.Format(_T("%d"), xoffset);
	((CEdit*)GetDlgItem(IDC_EDIT2))->SetWindowTextW(str);
	str.Format(_T("%d"),yoffset);
	((CEdit*)GetDlgItem(IDC_EDIT3))->SetWindowTextW(str);
	str.Format(_T("%d"),scale);
	((CEdit*)GetDlgItem(IDC_EDIT4))->SetWindowTextW(str);

	QueryPerformanceFrequency(&freq);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CPathFinderTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPathFinderTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);

	
	}
	else
	{
		CDialogEx::OnPaint();
	}

	DrawMap();
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPathFinderTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPathFinderTestDlg::DrawPath(struct vector3* path,int size)
{
	CClientDC dc(this);
	CPen pen(PS_SOLID, 1, RGB(255, 255, 255));
	CPen* open = dc.SelectObject(&pen);


	dc.MoveTo(path[0].x*scale+xoffset,path[0].z*scale+yoffset);
	for (int i = 1;i < size;i++)
	{
		dc.LineTo(path[i].x*scale+xoffset,path[i].z*scale+yoffset);
		dc.MoveTo(path[i].x*scale+xoffset,path[i].z*scale+yoffset);
	}

	dc.SelectObject(open);
}


int lua_draw(lua_State* L)
{
	CPathFinderTestDlg* self = (CPathFinderTestDlg*)lua_touserdata(L,1);
	struct nav_path* path = (nav_path*)lua_touserdata(L, 2);

	self->DrawPath(path->wp,path->offset);
	return 0;
}


void OnSearchDump(void* self, int index)
{
	CPathFinderTestDlg* dlgPtr = (CPathFinderTestDlg*)self;
	CClientDC dc(dlgPtr);
	CBrush brush(RGB(123, 255, 0));
	dc.SelectObject(&brush);

	struct nav_node* node = get_node(dlgPtr->mesh_ctx, index);
	CPoint* pt = new CPoint[node->size];
	for (int j = 0; j < node->size; j++)
	{
		struct vector3* pos = &dlgPtr->mesh_ctx->vertices[node->poly[j]];
		pt[j].x = pos->x*dlgPtr->scale + dlgPtr->xoffset;
		pt[j].y = pos->z*dlgPtr->scale + dlgPtr->yoffset;
	}
	dc.Polygon(pt, node->size);
	delete[] pt;
}

void CPathFinderTestDlg::OnPath()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	
	if (polyBegin == -1)
	{
		CString str;
		str.Format(_T("ʼ�㻹û����"));
		MessageBox(str);
	}
	else if (polyOver == -1)
	{
		CString str;
		str.Format(_T("�յ㻹û����"));
		MessageBox(str);
	}
	else
	{
		for(int i = 0;i < 8;i++)
		{
			set_mask(&mesh_ctx->mask_ctx,i,1);
		}
		//set_mask(&mesh_ctx->mask_ctx,1,0);


		LARGE_INTEGER counterBegin, counterEnd;
		QueryPerformanceCounter(&counterBegin);
		struct vector3 ptBegin;
		struct vector3 ptOver;
		struct nav_path* path;
		for (int i = 0; i < 10000;i++)
		{
			
			ptBegin.x = (double)(vtBegin->x - xoffset) / scale;
			ptBegin.z = (double)(vtBegin->z - yoffset) / scale;
			
			ptOver.x = (double)(vtOver->x - xoffset) / scale;
			ptOver.z = (double)(vtOver->z - yoffset) / scale;
			path = astar_find(mesh_ctx, &ptBegin, &ptOver, NULL, this);
		}
	

		QueryPerformanceCounter(&counterEnd);
		
		double pathCost = (double)((counterEnd.QuadPart - counterBegin.QuadPart)*1000) / (double)freq.QuadPart;
		CString str;
		str.Format(_T("��ʱ:%fms"), pathCost);
		((CStatic*)GetDlgItem(IDC_STATIC1))->SetWindowTextW(str);

		DrawPath(path->wp,path->offset);

		ptBegin.x = (double)(vtBegin->x-xoffset)/scale;
		ptBegin.z = (double)(vtBegin->z-yoffset)/scale;
		ptOver.x = (double)(vtOver->x-xoffset)/scale;
		ptOver.z = (double)(vtOver->z-yoffset)/scale;

		//lua_pushcfunction(L,lua_draw);
		//lua_setglobal(L,"draw");

		//lua_getglobal(L,"find");
		//lua_pushlightuserdata(L,this);
		//lua_pushnumber(L,ptBegin.x);
		//lua_pushnumber(L,ptBegin.z);
		//lua_pushnumber(L,ptOver.x);
		//lua_pushnumber(L,ptOver.z);
		//int r = lua_pcall(L,5,0,0);
		//if (r != LUA_OK) 
		//{
		//	CString str(lua_tostring(L,-1));
		//	MessageBox(str);
		//}

		//release_mesh(mesh_ctx);
	}
}


int CPathFinderTestDlg::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ

	return CDialogEx::OnMouseActivate(pDesktopWnd, nHitTest, message);
}


void CPathFinderTestDlg::DrawMap()
{
	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
	CClientDC dc(this);
	CPen *pOldPen = dc.SelectObject(&pen);

	CBrush brush(RGB(255,0,0));
	CBrush *obrush = dc.SelectObject(&brush);

	CBrush brushDoor(RGB(88,88,0));
	for (int i = 0;i < mesh_ctx->size;i++)
	{
		struct nav_node* node = get_node(mesh_ctx,i);
		CPoint* pt = new CPoint[node->size];

		if (node->mask == 0 )
		{
			dc.SelectObject(&brush);
		}
		else
		{
			dc.SelectObject(&brushDoor);
		}
		for (int j = 0; j < node->size;j++)
		{
			struct vector3* pos = &mesh_ctx->vertices[node->poly[j]];
			
			pt[j].x = pos->x*scale+xoffset;
			pt[j].y = pos->z*scale+yoffset;
		}
		dc.Polygon(pt,node->size);
		delete[] pt;
		dc.SelectObject(obrush);
	}

	obrush = dc.SelectObject(&brush);
	CBrush brush_empty0(RGB(255,255,0));
	CBrush brush_empty1(RGB(255,0,0));
	CBrush brush_empty2(RGB(88,88,0));

	int check = ((CButton *)GetDlgItem(IDC_CHECK1))->GetCheck();
	if (check)
	{
		for (int i = 0; i < mesh_ctx->width* mesh_ctx->heigh; i++)
		{
			struct nav_tile* tile = &mesh_ctx->tile[i];
			CPoint pt[4];

			if (tile->offset == 0)
			{
				dc.SelectObject(&brush_empty0);
			}
			else
				dc.SelectObject(&brush_empty1);
			/*	if (tile->mask == -1)
			dc.SelectObject(&brush_empty0);
			else*/
			//dc.SelectObject(&brush_empty1);

			for (int j = 0; j < 4; j++)
			{
				struct vector3* pos = &tile->pos[j];

				pt[j].x = pos->x*scale + xoffset;
				pt[j].y = pos->z*scale + yoffset;
			}
			dc.Polygon(pt, 4);
		}
	}
	
	dc.SelectObject(obrush);
	
	if (polyBegin != -1)
	{
		CBrush brush(RGB(0,255,0));
		dc.SelectObject(&brush);

		struct nav_node* node = get_node(mesh_ctx,polyBegin);
		CPoint* pt = new CPoint[node->size];
		for (int j = 0; j < node->size;j++)
		{
			struct vector3* pos = &mesh_ctx->vertices[node->poly[j]];
			pt[j].x = pos->x*scale+xoffset;
			pt[j].y = pos->z*scale+yoffset;
		}
		dc.Polygon(pt,node->size);
		delete[] pt;
	}

	if (polyOver != -1)
	{
		CBrush brush(RGB(0,0,255));
		dc.SelectObject(&brush);

		struct nav_node* node = get_node(mesh_ctx,polyOver);
		CPoint* pt = new CPoint[node->size];
		for (int j = 0; j < node->size;j++)
		{
			struct vector3* pos = &mesh_ctx->vertices[node->poly[j]];
			pt[j].x = pos->x*scale+xoffset;
			pt[j].y = pos->z*scale+yoffset;
		}
		dc.Polygon(pt,node->size);
		delete[] pt;
	}


	if (vtBegin != NULL)
	{
		CBrush brush(RGB(50,50,50));
		dc.SelectObject(&brush);
		dc.Ellipse(vtBegin->x-3,vtBegin->z-3,vtBegin->x+3,vtBegin->z+3); 
		dc.Ellipse(vtBegin->x-3+500,vtBegin->z-3,vtBegin->x+3+500,vtBegin->z+3); 
		int x = (vtBegin->x-xoffset)/scale - mesh_ctx->lt.x;
		int z = (vtBegin->z-yoffset)/scale - mesh_ctx->lt.z;
		int index = x + z * mesh_ctx->width;
		struct nav_tile* tile = & mesh_ctx->tile[index];

		//���ӿ�Ծ���ٸ������
		/*	for (int i = 0;i < tile->offset;i++)
			{
			int node_id = tile->node[i];

			CBrush brush(RGB(111,111,66));
			dc.SelectObject(&brush);

			struct nav_node* node = get_node(mesh_ctx,node_id);
			CPoint* pt0 = new CPoint[node->size];
			for (int j = 0; j < node->size;j++)
			{
			struct vector3* pos = &mesh_ctx->vertices[node->poly[j]];
			pt0[j].x = pos->x*scale+xoffset;
			pt0[j].y = pos->z*scale+yoffset;
			}
			dc.Polygon(pt0,node->size);
			delete[] pt0;
			}
			*/
		
		CPoint pt[4];
		CBrush brush00(RGB(99,99,99));
		dc.SelectObject(&brush00);

		for (int j = 0; j < 4;j++)
		{
			struct vector3* pos = &tile->pos[j];

			pt[j].x = pos->x*scale+xoffset;
			pt[j].y = pos->z*scale+yoffset;
		}
		dc.Polygon(pt,4);

		for (int j = 0; j < 4;j++)
		{
			struct vector3* pos = &tile->pos[j];

			pt[j].x = pos->x*scale+xoffset +300;
			pt[j].y = pos->z*scale+yoffset;
		}
		dc.Polygon(pt,4);
	}

	if (vtOver != NULL)
	{
		CBrush brush(RGB(250,50,50));
		dc.SelectObject(&brush);
		dc.Ellipse(vtOver->x-3,vtOver->z-3,vtOver->x+3,vtOver->z+3); 
	}

	dc.SelectObject(pOldPen);
	dc.SelectObject(obrush);

}

void CPathFinderTestDlg::DrawBegin(CPoint& pos)
{
	struct nav_node* node = get_node_with_pos(mesh_ctx,(double)(pos.x-xoffset)/scale,0,(double)(pos.y-yoffset)/scale);

	if (node == NULL)
		return;

	if (vtBegin != NULL)
	{
		free(vtBegin);
		vtBegin = NULL;
	}

	polyBegin = node->id;

	vtBegin = (vector3*)malloc(sizeof(*vtBegin));
	vtBegin->x = pos.x;
	vtBegin->z = pos.y;

	CString text;
	text.Format(_T("���:x:%f,z:%f"), (double)(pos.x - xoffset) / scale, (double)(pos.y - yoffset) / scale);
	((CStatic*)GetDlgItem(IDC_STATIC2))->SetWindowTextW(text);

	CString temp;

	if (node->size == 3)
	{
		temp.Format(_T("%d %d %d %d"), node->id,node->poly[0],node->poly[1],node->poly[2]);
	}
	else if (node->size == 4)
	{
		temp.Format(_T("%d %d %d %d %d"), node->id,node->poly[0],node->poly[1],node->poly[2],node->poly[3]);
	}
	else if (node->size == 5)
	{
		temp.Format(_T("%d %d %d %d %d %d"), node->id,node->poly[0],node->poly[1],node->poly[2],node->poly[3],node->poly[4]);
	}
	else if (node->size == 6)
	{
		temp.Format(_T("%d %d %d %d %d %d %d"), node->id,node->poly[0],node->poly[1],node->poly[2],node->poly[3],node->poly[4],node->poly[5]);
	}

	LPCTSTR str = temp;
	((CEdit*)GetDlgItem(IDC_EDIT5))->SetWindowTextW(str);

	Invalidate();
}

void OnAroundDump(void* self, int index)
{
	CPathFinderTestDlg* dlgPtr = (CPathFinderTestDlg*)self;
	CClientDC dc(dlgPtr);
	CBrush brush(RGB(66, 66, 66));

	struct nav_tile* tile = &dlgPtr->mesh_ctx->tile[index];
	CPoint pt[4];
	
	
	CBrush* obrush = dc.SelectObject(&brush);

	for (int j = 0; j < 4; j++)
	{
		struct vector3* pos = &tile->pos[j];

		pt[j].x = pos->x*dlgPtr->scale + dlgPtr->xoffset;
		pt[j].y = pos->z*dlgPtr->scale + dlgPtr->yoffset;
	}
	dc.Polygon(pt, 4);
	dc.SelectObject(obrush);
}

void CPathFinderTestDlg::DrawOver(CPoint& pos)
{
	double real_x = (double)(pos.x - xoffset) / scale;
	double real_z = (double)(pos.y - yoffset) / scale;
	struct nav_node* node = get_node_with_pos(mesh_ctx, real_x, 0, real_z);
	if (node == NULL)
	{
		vector3* pos = around_movable(mesh_ctx, real_x, real_z, 0, 5, OnAroundDump, this);
		if (pos)
		{
			CBrush brush(RGB(66, 88, 188));
			CClientDC dc(this);
			CBrush* obrush = dc.SelectObject(&brush);
			dc.Ellipse(pos->x*scale + xoffset - 3, pos->z*scale + yoffset - 3, pos->x*scale + xoffset + 3, pos->z*scale + yoffset + 3);
			dc.SelectObject(obrush);
		}
	
		return;
	}
		

	if (vtOver != NULL)
	{
		free(vtOver);
		vtOver = NULL;
	}

	polyOver = node->id;

	vtOver = (vector3*)malloc(sizeof(*vtOver));
	vtOver->x = pos.x;
	vtOver->z = pos.y;

	CString str;
	str.Format(_T("�յ�:x:%05f,z:%05f"), (double)(pos.x - xoffset) / scale, (double)(pos.y - yoffset) / scale);
	((CStatic*)GetDlgItem(IDC_STATIC3))->SetWindowTextW(str);

	Invalidate();
}

void CPathFinderTestDlg::OnUpdateIddPathfindertestDialog(CCmdUI *pCmdUI)
{
	// TODO:  �ڴ������������û����洦��������
	DrawMap();
}



void CPathFinderTestDlg::Straightline()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (polyBegin == -1)
	{
		CString str;
		str.Format(_T("ʼ�㻹û����"));
		MessageBox(str);
	}
	else if (polyOver == -1)
	{
		CString str;
		str.Format(_T("�յ㻹û����"));
		MessageBox(str);
	}
	else
	{
		for(int i = 0;i < 8;i++)
		{
			set_mask(&mesh_ctx->mask_ctx,i,1);
		}
		set_mask(&mesh_ctx->mask_ctx,1,0);
		vector3 vt0;
		vt0.x = (double)(vtBegin->x-xoffset)/scale;
		vt0.y = 0;
		vt0.z = (double)(vtBegin->z-yoffset)/scale;

		vector3 vt1;
		vt1.x = (double)(vtOver->x-xoffset)/scale;
		vt1.y = 0;
		vt1.z = (double)(vtOver->z-yoffset)/scale;

		LARGE_INTEGER counterBegin, counterEnd;
		QueryPerformanceCounter(&counterBegin);
		vector3 vt;
		bool ok = raycast(mesh_ctx,&vt0,&vt1,&vt);
		QueryPerformanceCounter(&counterEnd);

		double pathCost = (double)((counterEnd.QuadPart - counterBegin.QuadPart) * 1000) / (double)freq.QuadPart;
		CString str;
		str.Format(_T("��ʱ:%fms"), pathCost);
		((CStatic*)GetDlgItem(IDC_STATIC1))->SetWindowTextW(str);
		if (ok)
		{
			POINT from;
			from.x = vtBegin->x;
			from.y = vtBegin->z;

			POINT to;
			to.x = vt.x*scale+xoffset;
			to.y = vt.z*scale+yoffset;

			CPen pen(PS_SOLID, 1, RGB(255, 255, 255));
			CClientDC dc(this);
			CPen *open = dc.SelectObject(&pen);
			dc.MoveTo(from);
			dc.LineTo(to);
			dc.SelectObject(open);
		}
	}
}


void CPathFinderTestDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CPathFinderTestDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ


	this->DrawBegin(point);

	CDialogEx::OnLButtonUp(nFlags, point);
}



void CPathFinderTestDlg::OnIgnoreLine()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (polyBegin == NULL)
	{
		CString str;
		str.Format(_T("ʼ�㻹û����"));
		MessageBox(str);
	}
	else if (polyOver == NULL)
	{
		CString str;
		str.Format(_T("�յ㻹û����"));
		MessageBox(str);
	}
	else
	{
		for(int i = 0;i < 8;i++)
		{
			set_mask(&mesh_ctx->mask_ctx,i,1);
		}
		vector3 vt0;
		vt0.x = (double)(vtBegin->x-xoffset)/scale;
		vt0.y = 0;
		vt0.z = (double)(vtBegin->z-yoffset)/scale;

		vector3 vt1;
		vt1.x = (double)(vtOver->x-xoffset)/scale;
		vt1.y = 0;
		vt1.z = (double)(vtOver->z-yoffset)/scale;

		vector3 vt;
		bool ok = raycast(mesh_ctx,&vt0,&vt1,&vt);
		if (ok)
		{
			POINT from;
			from.x = vtBegin->x;
			from.y = vtBegin->z;

			POINT to;
			to.x = vt.x*scale+xoffset;
			to.y = vt.z*scale+yoffset;

			CPen pen(PS_SOLID, 1, RGB(255, 255, 255));
			CClientDC dc(this);
			CPen *open = dc.SelectObject(&pen);
			dc.MoveTo(from);
			dc.LineTo(to);
			dc.SelectObject(open);
		}
		set_mask(&mesh_ctx->mask_ctx,0,1);
	}
}


void CPathFinderTestDlg::OnEnChangeEdit2()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�



	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;
	((CEdit*)GetDlgItem(IDC_EDIT2))->GetWindowTextW(str);
	xoffset = _ttoi(str);
	Invalidate();
}


void CPathFinderTestDlg::OnEnChangeEdit3()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�



	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;
	((CEdit*)GetDlgItem(IDC_EDIT3))->GetWindowTextW(str);
	yoffset = _ttoi(str);
	Invalidate();
}


void CPathFinderTestDlg::OnEnChangeEdit4()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	CString str;
	((CEdit*)GetDlgItem(IDC_EDIT4))->GetWindowTextW(str);
	scale = _ttoi(str);
	Invalidate();

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CPathFinderTestDlg::OnIgnorePath()
{
	if (polyBegin == -1)
	{
		CString str;
		str.Format(_T("ʼ�㻹û����"));
		MessageBox(str);
	}
	else if (polyOver == -1)
	{
		CString str;
		str.Format(_T("�յ㻹û����"));
		MessageBox(str);
	}
	else
	{
		for(int i = 0;i < 8;i++)
		{
			set_mask(&mesh_ctx->mask_ctx,i,1);
		}

		struct vector3 ptBegin;
		ptBegin.x = (double)(vtBegin->x-xoffset)/scale;
		ptBegin.z = (double)(vtBegin->z-yoffset)/scale;
		struct vector3 ptOver;
		ptOver.x = (double)(vtOver->x-xoffset)/scale;
		ptOver.z = (double)(vtOver->z-yoffset)/scale;

		struct vector3 smooth[64];
		int index = 0;

		LARGE_INTEGER counterBegin, counterEnd;
		QueryPerformanceCounter(&counterBegin);
		vector3 vt;
		struct nav_path* path = astar_find(mesh_ctx, &ptBegin, &ptOver, NULL, NULL);
	

		//
		//int i = 0;
		//while(i < path->offset)
		//{
		//	smooth[index].x = path->wp[i].x;
		//	smooth[index].z = path->wp[i].z;
		//	index++;
		//	int j;
		//	for(j = i + 2;j < path->offset;j++)
		//	{
		//		struct vector3 result;
		//		if (raycast(mesh_ctx,&path->wp[i],&path->wp[j],&result))
		//		{
		//			double distance = (result.x - path->wp[j].x)*(result.x - path->wp[j].x)+(result.z - path->wp[j].z)*(result.z - path->wp[j].z) ;
		//			if (distance!= 0)
		//			{
		//				i = j-1;
		//				break;
		//			}
		//		}
		//		else
		//		{
		//			i = j-1;
		//			break;
		//		}
		//	}
		//	if (j == path->offset)
		//	{
		//		smooth[index].x = path->wp[path->offset-1].x;
		//		smooth[index].z = path->wp[path->offset-1].z;
		//		index++;
		//		break;
		//	}
		//}
		QueryPerformanceCounter(&counterEnd);

		double pathCost = (double)((counterEnd.QuadPart - counterBegin.QuadPart) * 1000) / (double)freq.QuadPart;
		CString str;
		str.Format(_T("��ʱ:%fms"), pathCost);
		((CStatic*)GetDlgItem(IDC_STATIC1))->SetWindowTextW(str);

		DrawPath(path->wp, path->offset);
	}
}


void CPathFinderTestDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	this->DrawOver(point);
	CDialogEx::OnRButtonUp(nFlags, point);
}


void CPathFinderTestDlg::OnClose()
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	release_mesh(mesh_ctx);
	_CrtDumpMemoryLeaks();
	CDialogEx::OnClose();
}


void CPathFinderTestDlg::OnLoadMesh()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	TCHAR szFilter[] = _T("json�ļ�(*.json)|*.json|�����ļ�(*.*)|*.*||");
	CFileDialog fileDlg(TRUE, _T("mesh"), NULL, 0, szFilter, this);
	CString strFilePath;
 
	if (IDOK == fileDlg.DoModal())
	{
		if (mesh_ctx != NULL)
		{
			release_mesh(mesh_ctx);
			mesh_ctx = NULL;
		}
		strFilePath = fileDlg.GetPathName();

		rapidjson::Document _config;

		char *lText;
		int iTexLen = WideCharToMultiByte(CP_ACP, 0, strFilePath.GetBuffer(0), -1, NULL, 0, NULL, 0);

		lText = (char*)calloc(iTexLen, sizeof(char));

		memset(lText, 0, iTexLen*sizeof(char));  //��ʼ���ռ�

		WideCharToMultiByte(CP_ACP, 0, strFilePath.GetBuffer(0), -1, lText, iTexLen, NULL, 0);  // ��strText�е��ַ���ȫ��ת����ASCII�������䱣����lText���ٵ��ַ��ռ���

		FILE* file = fopen(lText, "r");
		//FILE* file = fopen("E:\\workplace\\navmesh\\navmesh_pathfinder\\PathFinderTest\\mesh.json", "r");
		assert(file != NULL);
		fseek(file, 0, SEEK_END);
		int len = ftell(file);
		char* json = (char*)malloc(len + 1);
		memset(json, 0, len + 1);
		rewind(file);
		fread(json, 1, len, file);
		fclose(file);
		_config.Parse(json);

		const rapidjson::Value& v = _config["v"];
		double** v_ptr = (double**)malloc(sizeof(*v_ptr) * v.Size());
		for (int i = 0; i < v.Size(); i++)
		{
			v_ptr[i] = (double*)malloc(sizeof(double)* 3);
			const rapidjson::Value& tmp = v[i];
			v_ptr[i][0] = tmp[0].GetDouble();
			v_ptr[i][1] = tmp[1].GetDouble();
			v_ptr[i][2] = tmp[2].GetDouble();
		}

		const rapidjson::Value& p = _config["p"];
		int** p_ptr = (int**)malloc(sizeof(*p_ptr) * p.Size());
		int a = p.Size();
		for (int i = 0; i < p.Size(); i++)
		{
			const rapidjson::Value& tmp = p[i];
			p_ptr[i] = (int*)malloc(sizeof(int)*(tmp.Size()));
			for (int j = 0; j < tmp.Size(); j++)
			{
				p_ptr[i][j] = tmp[j].GetInt();
			}
		}
		free(json);

		mesh_ctx = load_mesh(v_ptr, v.Size(), p_ptr, p.Size());
	}
}


void CPathFinderTestDlg::OnSaveMesh()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CPathFinderTestDlg::OnCheck()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	Invalidate();
}
