#include "BCP.h"
#include <commdlg.h> //might need this
//static OPENFILENAME ofn ; //do need this


void FileInitialize (HWND hwnd, OPENFILENAME *ofn)
{
	static TCHAR szFilter[] = TEXT ("Text Files (*.TXT)\0*.txt\0")  
		TEXT ("ASCII Files (*.ASC)\0*.asc\0")  
		TEXT ("All Files (*.*)\0*.*\0\0") ;

	ofn->lStructSize       = sizeof (OPENFILENAME) ;
	ofn->hwndOwner         = hwnd ;
	ofn->hInstance         = NULL ;
	ofn->lpstrFilter       = szFilter ;
	ofn->lpstrCustomFilter = NULL ;
	ofn->nMaxCustFilter    = 0 ;
	ofn->nFilterIndex      = 0 ;
	ofn->lpstrFile         = NULL ;          // Set in Open and Close functions
	ofn->nMaxFile          = MAX_PATH ;
	ofn->lpstrFileTitle    = NULL ;          // Set in Open and Close functions
	ofn->nMaxFileTitle     = MAX_PATH ;
	ofn->lpstrInitialDir   = NULL ;
	ofn->lpstrTitle        = NULL ;
	ofn->Flags             = 0 ;             // Set in Open and Close functions
	ofn->nFileOffset       = 0 ;
	ofn->nFileExtension    = 0 ;
	ofn->lpstrDefExt       = TEXT ("txt") ;
	ofn->lCustData         = 0L ;
	ofn->lpfnHook          = NULL ;
	ofn->lpTemplateName    = NULL ;
	ofn->Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;
}

BOOL FileOpenDlg (HWND hwnd, PTSTR pstrFileName, OPENFILENAME *ofn)
{
	ofn->hwndOwner         = hwnd ;
	ofn->lpstrFile         = pstrFileName ;
	

	return GetOpenFileName (ofn) ;
}