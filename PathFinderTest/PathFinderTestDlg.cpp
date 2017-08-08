
// PathFinderTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PathFinderTest.h"
#include "PathFinderTestDlg.h"
#include "afxdialogex.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"  
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CPathFinderTestDlg 对话框



CPathFinderTestDlg::CPathFinderTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPathFinderTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);


	_beginOffset.x = 100;
	_beginOffset.y = 20;
	_overOffset.x = 950;
	_overOffset.y = 470;
	_tile = 10;
	_checkState = BLOCK;
	_beginRect = NULL;
	_overRect = NULL;
}

void CPathFinderTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPathFinderTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CPathFinderTestDlg::OnBnClickedButton1)
	ON_WM_MOUSEACTIVATE()
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_CHECK1, &CPathFinderTestDlg::OnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, &CPathFinderTestDlg::OnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CPathFinderTestDlg::OnClickedCheck3)
	ON_UPDATE_COMMAND_UI(IDD_PATHFINDERTEST_DIALOG, &CPathFinderTestDlg::OnUpdateIddPathfindertestDialog)
	ON_BN_CLICKED(IDC_BUTTON2, &CPathFinderTestDlg::OnBnClickedButton2)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_CHECK4, &CPathFinderTestDlg::OnClickedCheck4)
	ON_BN_CLICKED(IDC_BUTTON3, &CPathFinderTestDlg::OnBnClickedButton3)
	ON_EN_CHANGE(IDC_EDIT2, &CPathFinderTestDlg::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT3, &CPathFinderTestDlg::OnEnChangeEdit3)
	ON_EN_CHANGE(IDC_EDIT4, &CPathFinderTestDlg::OnEnChangeEdit4)
END_MESSAGE_MAP()


// CPathFinderTestDlg 消息处理程序

BOOL CPathFinderTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	OnClickedCheck1();

	rapidjson::Document _config;
	FILE* file = fopen("mesh.json","r");
	assert(file != NULL);
	fseek(file,0,SEEK_END);
	int len = ftell(file);
	char* json = (char*)malloc(len);
	memset(json,0,len);
	rewind(file);
	fread(json,1,len,file);
	fclose(file);
	assert(_config.ParseInsitu(json).HasParseError() == false);

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
	for (int i = 0;i < p.Size();i++)
	{
		const rapidjson::Value& tmp = p[i];
		p_ptr[i] = (int*)malloc(sizeof(int) *(tmp.Size()+1));
		p_ptr[i][0] = tmp.Size();
		for (int j = 0;j < tmp.Size();j++)
		{
			p_ptr[i][j+1] = tmp[j].GetInt();
		}
	}
	this->mesh_ctx  = load_mesh(v_ptr,v.Size(),p_ptr, p.Size());
	xoffset = 100;
	yoffset = -350;
	polyBegin = -1;
	polyOver = -1;
	scale = 15;
	vtOver = vtBegin = NULL;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPathFinderTestDlg::OnPaint()
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

	DrawMap();
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPathFinderTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

struct DumpArgs
{
	CPathFinderTestDlg* self;
	CClientDC* cdc;
};

void pathDump(void* ud, int x, int y)
{
	DumpArgs* args = (DumpArgs*)ud;
	CPathFinderTestDlg* self = args->self;
	CClientDC* cdc = args->cdc;

	CBrush brush(RGB(0, 255, 0));
	POINT from;
	from.x = self->_beginOffset.x + self->_tile * x;
	from.y = self->_beginOffset.y + self->_tile * y;
	POINT to;
	to.x = from.x + self->_tile;
	to.y = from.y + self->_tile;

	cdc->FillRect(CRect(from, to), &brush);
}

void pathCallback(void* ud, int x, int y,int flag)
{
	DumpArgs* args = (DumpArgs*)ud;
	CPathFinderTestDlg* self = args->self;
	CClientDC* cdc = args->cdc;

	POINT to;
	to.x = self->_beginOffset.x + x * self->_tile + self->_tile / 2;
	to.y = self->_beginOffset.y + y * self->_tile + self->_tile / 2;
	cdc->LineTo(to);
	cdc->MoveTo(to);

	if (flag == 0)
	{
		CBrush brush(RGB(0x8b, 0x86, 0x82));
		POINT from;
		from.x = self->_beginOffset.x + self->_tile * x;
		from.y = self->_beginOffset.y + self->_tile * y;
		POINT to;
		to.x = from.x + self->_tile;
		to.y = from.y + self->_tile;

		cdc->FillRect(CRect(from, to), &brush);
	}
	else
	{
		CBrush brush(RGB(0x00, 0x86, 0x82));
		POINT from;
		from.x = self->_beginOffset.x + self->_tile * x;
		from.y = self->_beginOffset.y + self->_tile * y;
		POINT to;
		to.x = from.x + self->_tile;
		to.y = from.y + self->_tile;

		cdc->FillRect(CRect(from, to), &brush);
	}
	
}

void CPathFinderTestDlg::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序代码
	
	if (polyBegin == -1)
	{
		CString str;
		str.Format(_T("始点还没设置"));
		MessageBox(str);
	}
	else if (polyOver == -1)
	{
		CString str;
		str.Format(_T("终点还没设置"));
		MessageBox(str);
	}
	else
	{
		struct vector3 ptBegin;
		ptBegin.x = (double)(vtBegin->x-xoffset)/scale;
		ptBegin.z = (double)(vtBegin->z-yoffset)/scale;
		struct vector3 ptOver;
		ptOver.x = (double)(vtOver->x-xoffset)/scale;
		ptOver.z = (double)(vtOver->z-yoffset)/scale;

		polyPath = astar_find(mesh_ctx,&ptBegin,&ptOver,resultPath,&sizePath);
	}
	Invalidate();
}


int CPathFinderTestDlg::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	return CDialogEx::OnMouseActivate(pDesktopWnd, nHitTest, message);
}


void CPathFinderTestDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch (_checkState)
	{
	case BLOCK:
		this->DrawBlock(point);
		break;
	case BEGIN:
		this->DrawBegin(point);
		break;
	case OVER:
		this->DrawOver(point);
		break;
	default:
		break;
	}
	CDialogEx::OnLButtonDblClk(nFlags, point);
}

void CPathFinderTestDlg::DrawMap()
{
	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
	CClientDC dc(this);
	CPen *pOldPen = dc.SelectObject(&pen);

	CBrush brush(RGB(255,0,0));
	CBrush *obrush = dc.SelectObject(&brush);

	for (int i = 0;i < mesh_ctx->size;i++)
	{
		struct NavNode* node = find_node(mesh_ctx,i);
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

	

	if (polyBegin != -1)
	{
		CBrush brush(RGB(0,255,0));
		dc.SelectObject(&brush);

		struct NavNode* node = find_node(mesh_ctx,polyBegin);
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

		struct NavNode* node = find_node(mesh_ctx,polyOver);
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

	if (polyPath != NULL)
	{
		CBrush brush(RGB(100,54,255));
		dc.SelectObject(&brush);

		struct NavNode* tmp = polyPath;
		while (tmp)
		{
			CPoint* pt = new CPoint[tmp->size];
			for (int j = 0; j < tmp->size;j++)
			{
				struct vector3* pos = &mesh_ctx->vertices[tmp->poly[j]];
				pt[j].x = pos->x*scale+xoffset;
				pt[j].y = pos->z*scale+yoffset;
			}
			dc.Polygon(pt,tmp->size);
			delete[] pt;
			tmp = tmp->link_parent;
		}

		
	}
	CPen pen1(PS_SOLID, 1, RGB(255, 255, 255));
	dc.SelectObject(&pen1);
	dc.MoveTo(resultPath[0].x*scale+xoffset,resultPath[0].z*scale+yoffset);
	for (int i = 1;i < sizePath;i++)
	{
		dc.LineTo(resultPath[i].x*scale+xoffset,resultPath[i].z*scale+yoffset);
		dc.MoveTo(resultPath[i].x*scale+xoffset,resultPath[i].z*scale+yoffset);
	}
	if (vtBegin != NULL)
	{
		CBrush brush(RGB(50,50,50));
		dc.SelectObject(&brush);
		dc.Ellipse(vtBegin->x-3,vtBegin->z-3,vtBegin->x+3,vtBegin->z+3); 
	}

	if (vtOver != NULL)
	{
		CBrush brush(RGB(250,50,50));
		dc.SelectObject(&brush);
		dc.Ellipse(vtOver->x-3,vtOver->z-3,vtOver->x+3,vtOver->z+3); 
	}

	dc.SelectObject(pOldPen);
	dc.SelectObject(obrush);
	/*int height = 0;
	int width = 0;

	for (int i = _beginOffset.y; i <= _overOffset.y; i += _tile)
	{
	POINT from;
	from.x = _beginOffset.x;
	from.y = i;
	POINT to;
	to.x = _overOffset.x;
	to.y = i;
	dc.MoveTo(from);
	dc.LineTo(to);
	height++;
	}

	for (int i = _beginOffset.x; i <= _overOffset.x; i += _tile)
	{
	POINT from;
	from.x = i;
	from.y = _beginOffset.y;
	POINT to;
	to.x = i;
	to.y = _overOffset.y;
	dc.MoveTo(from);
	dc.LineTo(to);
	width++;
	}

	_mapData = (char*)malloc(sizeof(char)* height * width);
	memset(_mapData, 1, sizeof(char)* height * width);

	dc.SelectObject(pOldPen);

	CClientDC rectDc(this);

	std::vector<TileRect*>::iterator iter = _blockVector.begin();
	for (; iter != _blockVector.end();iter++)
	{
	TileRect* br = *iter;
	if (br->_block == 0)
	{
	CBrush brush(RGB(255, 0, 0));
	rectDc.FillRect(br->_rect, &brush);
	}
	else
	{
	CBrush brush(RGB(0xff, 0xff, 0));
	rectDc.FillRect(br->_rect, &brush);
	}

	_mapData[br->_index] = br->_block;
	}

	_width = width;
	_heigh = height;

	if (_beginRect != NULL)
	{
	CBrush brush(RGB(0, 0, 0));
	rectDc.FillRect(_beginRect->_rect, &brush);
	}
	if (_overRect != NULL)
	{
	CBrush brush(RGB(0, 255, 0));
	rectDc.FillRect(_overRect->_rect, &brush);
	}*/
}

void CPathFinderTestDlg::DrawBlock(CPoint& pos)
{
	int xIndex, yIndex;
	int index = GetRectIndex(pos, &xIndex, &yIndex);
	
	bool already = false;

	std::vector<TileRect*>::iterator iter = _blockVector.begin();
	for (; iter != _blockVector.end();)
	{
		if ((*iter)->_index == index)
		{
			_blockVector.erase(iter++);
			already = true;
			break;
		} 
		else
			iter++;
	}



	if (!already)
	{
		TileRect* rect = this->CreateTileRect(index, xIndex, yIndex);
		rect->_block = 0;
		_blockVector.push_back(rect);
	}

	Invalidate();
}

void CPathFinderTestDlg::DrawDoor(CPoint& pos)
{
	int xIndex, yIndex;
	int index = GetRectIndex(pos, &xIndex, &yIndex);

	bool already = false;

	std::vector<TileRect*>::iterator iter = _blockVector.begin();
	for (; iter != _blockVector.end();)
	{
		if ((*iter)->_index == index)
		{
			_blockVector.erase(iter++);
			already = true;
			break;
		}
		else
			iter++;
	}

	if (!already)
	{
		TileRect* rect = this->CreateTileRect(index, xIndex, yIndex);
		rect->_block = 2;
		_blockVector.push_back(rect);
	}

	Invalidate();
}

void CPathFinderTestDlg::DrawBegin(CPoint& pos)
{
	if (vtBegin != NULL)
	{
		free(vtBegin);
		vtBegin = NULL;
	}
	struct NavNode* node = find_node_with_pos(mesh_ctx,(double)(pos.x-xoffset)/scale,0,(double)(pos.y-yoffset)/scale);

	if (node == NULL)
		return;

	if (node->id != polyBegin)
	{
		polyBegin = node->id;
		polyPath = NULL;
	}
	vtBegin = (vector3*)malloc(sizeof(*vtBegin));
	vtBegin->x = pos.x;
	vtBegin->z = pos.y;

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

void CPathFinderTestDlg::DrawOver(CPoint& pos)
{
	if (vtOver != NULL)
	{
		free(vtOver);
		vtOver = NULL;
	}
	struct NavNode* node = find_node_with_pos(mesh_ctx,(double)(pos.x-xoffset)/scale,0,(double)(pos.y-yoffset)/scale);
	if (node == NULL)
		return;
	if (node->id != polyOver)
	{
		polyOver = node->id;
		polyPath = NULL;
	}
	vtOver = (vector3*)malloc(sizeof(*vtOver));
	vtOver->x = pos.x;
	vtOver->z = pos.y;
	Invalidate();
}

int CPathFinderTestDlg::GetRectIndex(CPoint& pos, int* xIndexPtr, int* yIndexPtr)
{
	int xIndex = (pos.x - _beginOffset.x) / _tile;
	int yIndex = (pos.y - _beginOffset.y) / _tile;;
	int xCnt = (_overOffset.x - _beginOffset.x) / _tile;
	int yCnt = (_overOffset.y - _beginOffset.y) / _tile;
	*xIndexPtr = xIndex;
	*yIndexPtr = yIndex;
	return (yCnt+1) * xIndex + yIndex;
}

CPathFinderTestDlg::TileRect* CPathFinderTestDlg::CreateTileRect(int index, int xIndex, int yIndex)
{
	TileRect* rect = new TileRect();
	rect->_index = index;
	rect->_xIndex = xIndex;
	rect->_yIndex = yIndex;
	rect->_rect.left = _beginOffset.x + _tile * xIndex;
	rect->_rect.top = _beginOffset.y + _tile * yIndex;
	rect->_rect.right = rect->_rect.left + 10;
	rect->_rect.bottom = rect->_rect.top + 10;
	return rect;
}

void CPathFinderTestDlg::ClearCheck()
{
	for (int i = IDC_CHECK1; i <= IDC_CHECK4; i++)
	{
		((CButton*)GetDlgItem(i))->SetCheck(false);
	}

}

void CPathFinderTestDlg::OnClickedCheck1()
{
	// TODO:  在此添加控件通知处理程序代码
	ClearCheck();
	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(true);
	_checkState = BLOCK;
}


void CPathFinderTestDlg::OnClickedCheck2()
{
	// TODO:  在此添加控件通知处理程序代码
	ClearCheck();
	((CButton*)GetDlgItem(IDC_CHECK2))->SetCheck(true);
	_checkState = BEGIN;
}


void CPathFinderTestDlg::OnClickedCheck3()
{
	// TODO:  在此添加控件通知处理程序代码
	ClearCheck();
	((CButton*)GetDlgItem(IDC_CHECK3))->SetCheck(true);
	_checkState = OVER;
}

void CPathFinderTestDlg::OnClickedCheck4()
{
	// TODO:  在此添加控件通知处理程序代码
	ClearCheck();
	((CButton*)GetDlgItem(IDC_CHECK4))->SetCheck(true);
	_checkState = DOOR;
}

void CPathFinderTestDlg::OnUpdateIddPathfindertestDialog(CCmdUI *pCmdUI)
{
	// TODO:  在此添加命令更新用户界面处理程序代码
	DrawMap();
}

void raycastDump(void* ud, int x, int y)
{
	DumpArgs* args = (DumpArgs*)ud;
	CPathFinderTestDlg* self = args->self;
	CClientDC* cdc = args->cdc;

	CBrush brush(RGB(0, 255, 0));
	POINT from;
	from.x = self->_beginOffset.x + self->_tile * x;
	from.y = self->_beginOffset.y + self->_tile * y;
	POINT to;
	to.x = from.x + self->_tile;
	to.y = from.y + self->_tile;

	cdc->FillRect(CRect(from, to), &brush);
}

void CPathFinderTestDlg::OnBnClickedButton2()
{
	// TODO:  在此添加控件通知处理程序代码
	if (polyBegin == -1)
	{
		CString str;
		str.Format(_T("始点还没设置"));
		MessageBox(str);
	}
	else if (polyOver == -1)
	{
		CString str;
		str.Format(_T("终点还没设置"));
		MessageBox(str);
	}
	else
	{

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
	}
}


void CPathFinderTestDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CPathFinderTestDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	switch (_checkState)
	{
	case BLOCK:
		this->DrawBlock(point);
		break;
	case BEGIN:
		this->DrawBegin(point);
		break;
	case OVER:
		this->DrawOver(point);
		break;
	case DOOR:
		this->DrawDoor(point);
		break;
	default:
		break;
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}



void CPathFinderTestDlg::OnBnClickedButton3()
{
	// TODO:  在此添加控件通知处理程序代码
	if (_beginRect == NULL)
	{
		CString str;
		str.Format(_T("始点还没设置"));
		MessageBox(str);
	}
	else if (_overRect == NULL)
	{
		CString str;
		str.Format(_T("终点还没设置"));
		MessageBox(str);
	}
	else
	{
	/*	pathfinder_context* finder = finder_create(1);
		finder_init(finder,0, 101, _width, _heigh, _mapData);
		finder_mask_set(finder, 0, 0, 0);
		finder_mask_set(finder, 0, 1, 1);
		finder_mask_set(finder, 0, 2, 0);
		int rx = 0;
		int ry = 0;

		CPen pen(PS_SOLID, 1, RGB(255, 100, 100));
		CClientDC dc(this);
		CPen *pOldPen = dc.SelectObject(&pen);

		POINT from;
		from.x = _beginOffset.x + _tile *_overRect->_xIndex + _tile / 2;
		from.y = _beginOffset.y + _tile * _overRect->_yIndex + _tile / 2;

		dc.MoveTo(from);

		DumpArgs* args = new DumpArgs();
		args->self = this;
		args->cdc = &dc;

		finder_find(finder, 0, _beginRect->_xIndex, _beginRect->_yIndex, _overRect->_xIndex, _overRect->_yIndex, 0, pathCallback, args, pathDump, args);

		delete args;

		dc.SelectObject(pOldPen);

		finder_release(finder);*/
	}

}


void CPathFinderTestDlg::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。



	// TODO:  在此添加控件通知处理程序代码
	CString str;
	((CEdit*)GetDlgItem(IDC_EDIT2))->GetWindowTextW(str);
	xoffset = _ttoi(str);
	Invalidate();
}


void CPathFinderTestDlg::OnEnChangeEdit3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。



	// TODO:  在此添加控件通知处理程序代码
	CString str;
	((CEdit*)GetDlgItem(IDC_EDIT3))->GetWindowTextW(str);
	yoffset = _ttoi(str);
	Invalidate();
}


void CPathFinderTestDlg::OnEnChangeEdit4()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	CString str;
	((CEdit*)GetDlgItem(IDC_EDIT4))->GetWindowTextW(str);
	scale = _ttoi(str);
	Invalidate();

	// TODO:  在此添加控件通知处理程序代码
}
