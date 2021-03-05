#include <assert.h>
#include <Windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fstream>

#include "FileUtils.h"

using namespace std;


//some of the file handling functions are 16bit unicode, some are 8 bit so we need to support both
//and be able to convert between the two
void ConvertUTF16toUTF8(utf8char* const pStrTgt, const unsigned int maxTgtSz,
	const utf16char* const pStrSource, const unsigned int maxSrcSz)
{
	assert(pStrTgt && pStrSource && maxTgtSz > 0 && maxSrcSz > 0);
	unsigned int lengthAnsiString = 0;
	lengthAnsiString = WideCharToMultiByte(CP_UTF8, 0, pStrSource, maxSrcSz, NULL, 0, NULL, NULL);
	assert(lengthAnsiString <= maxTgtSz);
	int ret = WideCharToMultiByte(CP_UTF8, 0, pStrSource, maxSrcSz, pStrTgt, lengthAnsiString, NULL, NULL);
	assert(ret);
	pStrTgt[ret] = 0;
}

void ConvertUTF16toUTF8(utf8string& strTgt, const utf16string& strSource)
{
	static utf8char txtBuff[1025];
	ConvertUTF16toUTF8(txtBuff, 1024, &strSource[0], static_cast<unsigned int>(strSource.size()));
	strTgt = txtBuff;
}

void ConvertUTF16toUTF8(utf8string& strTgt, const vector<utf16char>& strSource)
{
	static utf8char txtBuff[1025];
	ConvertUTF16toUTF8(txtBuff, 1024, &strSource[0], static_cast<unsigned int>(strSource.size()));
	strTgt = txtBuff;
}

void ConvertUTF8toUTF16(utf16char* const pStrTgt, const unsigned int maxTgtSz,
	const utf8char* const pStrSource, const unsigned int maxSrcSz)
{
	assert(pStrTgt && pStrSource && maxTgtSz > 0 && maxSrcSz > 0);
	unsigned int lengthWideString = 0;
	lengthWideString = MultiByteToWideChar(CP_UTF8, 0, pStrSource, maxSrcSz, NULL, 0);
	assert(lengthWideString < maxTgtSz);
	int ret = MultiByteToWideChar(CP_UTF8, 0, pStrSource, maxSrcSz, pStrTgt, lengthWideString);
	assert(ret);
	pStrTgt[ret] = 0;
}

void ConvertUTF8toUTF16(utf16string& strTgt, const utf8string& strSource)
{
	static utf16char txtBuff[1025];
	ConvertUTF8toUTF16(txtBuff, 1024, strSource.c_str(), static_cast<unsigned int>(strSource.length()));
	strTgt = txtBuff;
}

void ConvertUTF8toUTF16(utf16char* const pStrTgt, const unsigned int maxTgtSz, const utf8string& strSource)
{
	ConvertUTF8toUTF16(pStrTgt, maxTgtSz, strSource.c_str(), static_cast<unsigned int>(strSource.length()));
}

//*****************************************************************************************

bool FileOrFolderExists(const utf8string& name)
{
	assert(!name.empty());
	struct stat tagStat;
	bool ret = (stat(name.c_str(), &tagStat) == 0);
	return ret;
}


bool splitFileName(const utf8string& fileName, utf8string* pNameAndPath, utf8string* pExt,
	utf8string* pPath, utf8string* pName)
{
	assert(pNameAndPath || pExt || pName || pPath);
	if (pNameAndPath)
	{
		string::size_type pos = fileName.find_last_of('.');
		if (pos == string::npos)
			return false;
		*pNameAndPath = fileName.substr(0, pos);
	}
	if (pExt)
	{
		//the last 3 characters
		string::size_type pos = fileName.find_last_of('.');
		if (pos == string::npos)
			return false;
		*pExt = fileName.substr(pos + 1, fileName.length());
	}

	string::size_type lastSlash = fileName.find_last_of('/');
	if (lastSlash == string::npos)
		lastSlash = fileName.find_last_of('\\');
	if (pPath)
	{
		*pPath = "";
		if (lastSlash != string::npos)
			*pPath = fileName.substr(0, lastSlash);
	}
	if (pName)
	{
		if (lastSlash == string::npos)
			lastSlash = -1;

		string::size_type pos = fileName.find_last_of('.');
		if (pos == string::npos)
			pos = fileName.length();
		*pName = fileName.substr(lastSlash + 1, pos);
	}
	return true;
}



bool File::s_initialised = false;
File::Names File::s_fileNames;
utf8string File::s_firstRunDirectory;
utf16string File::s_firstRunDirectory16;


void File::initialiseSystem()
{
	getCurrentFolder(s_firstRunDirectory);
	ConvertUTF8toUTF16(s_firstRunDirectory16, s_firstRunDirectory);
	s_initialised = true;
}


File::File(const utf8string& fileName, const int flags)
	: m_isOpen(false),
	m_readAccess(false),
	m_writeAccess(false),
	m_fileName(fileName),
	m_pFStream(NULL)
{
	assert(s_initialised);// , "File system was not initialised");
	if (m_isOpen)
	{
		assert(0);// , "file=%s - is already open", m_fileName.c_str());
	}

	if (!openFile(m_fileName, flags, m_pFStream, &m_readAccess, &m_writeAccess))//, "file=%s - cannot open", m_fileName.c_str())
		assert(false);
	m_isOpen = true;
}



bool File::read(void* const pBuffer, const unsigned int size, unsigned int& bytesRead)
{
	assert(m_isOpen);
	assert(m_readAccess);
	if (m_pFStream->fail())
	{
		if (m_pFStream->eof())
			m_pFStream->clear();
		else
			assert(false);// , "stream failed reading %s.\n", m_fileName.c_str());
	}
	m_pFStream->read(static_cast<char*>(pBuffer), static_cast<std::streamsize>(size));
	bytesRead = static_cast<unsigned int>(m_pFStream->gcount());
	if (bytesRead == 0)
		return false;

	return true;
}


void File::seek(const unsigned int pos)
{
	assert(m_isOpen && pos >= 0);
	m_pFStream->clear(); //Clear fail status in case eof was set
	m_pFStream->seekg(static_cast<std::streamoff>(pos), std::ios::beg);
}


unsigned int File::getSize()
{
	assert(m_isOpen);

	struct stat tagStat;
	int ret = stat(m_fileName.c_str(), &tagStat);
	assert(ret == 0);// , "Problem getting file size");
	return tagStat.st_size;
}

bool File::close()
{
	if (m_isOpen == false)
		return false;

	assert(m_pFStream);
	m_pFStream->close();
	delete m_pFStream;
	m_isOpen = false;
	return true;
}


bool File::openFile(const utf8string& fileName, int flags, std::fstream*& pTheStream, bool* const pReadAccessFlag, bool* const pWriteAccessFlag)
{
	// Always open in binary mode
	pTheStream = new std::fstream();
	std::ios_base::openmode mode = std::ios::binary;
	if (flags & File::MPF_APPEND)
		mode |= std::ios::app;
	if (flags & File::MPF_READ)
	{
		mode |= std::ios::in;
		if (pReadAccessFlag)
			*pReadAccessFlag = true;
	}
	else if (pReadAccessFlag)
	{
		*pReadAccessFlag = false;
	}
	if (flags & File::MPF_WRITE)
	{
		mode |= std::ios::out;
		if (pWriteAccessFlag)
			*pWriteAccessFlag = true;
	}
	else if (pWriteAccessFlag)
	{
		*pWriteAccessFlag = false;
	}
	pTheStream->open(fileName.c_str(), mode);

	// Should check ensure open succeeded, in case fail for some reason.
	if (pTheStream->fail())
	{
		delete pTheStream;
		pTheStream = NULL;
		return false;
	}
	return true;
}


bool File::remove(const utf8string& fileName)
{
	utf16string tmp;
	ConvertUTF8toUTF16(tmp, fileName);
	if (_wremove(tmp.c_str()) == 0)
		return true;
	utf8char txt[1025];
	strerror_s(txt, 1024, errno);
	assert(0);// , "%s - error deleting file", g_txt);
	return false;
}

bool File::createFolder(const utf8string& folderName)
{
	assert(!folderName.empty());
	utf16char wtxt[1025];
	ConvertUTF8toUTF16(wtxt, 1024, folderName);
	if (CreateDirectoryW(wtxt, NULL))
		return true;
	DWORD dw = GetLastError();
	if (dw == ERROR_ALREADY_EXISTS)
		return true;
	return false;
}

bool File::createFolder(const utf16string& folderName)
{
	assert(!folderName.empty());
	if (CreateDirectoryW(folderName.c_str(), NULL))
		return true;
	DWORD dw = GetLastError();
	if (dw == ERROR_ALREADY_EXISTS)
		return true;
	return false;
}



bool File::fileExists(const utf8string& fileName)
{
	return FileOrFolderExists(fileName);
}

bool File::folderExists(const utf8string& folderName)
{
	return FileOrFolderExists(folderName);
}

void File::getCurrentFolder(utf8string& folder)
{
	//find out what folder we've ended up in
	unsigned int nchars = GetCurrentDirectoryW(0, NULL);
	static vector<utf16char> s_folderBuffer;
	nchars++;
	s_folderBuffer.clear();
	s_folderBuffer.insert(s_folderBuffer.begin(), nchars, 0);
	//growBuffer(s_folderBuffer, nchars, 1024);	//make sure its big, don't want to keep changing its size
	unsigned int ret = GetCurrentDirectoryW(nchars, &s_folderBuffer[0]);
	assert(ret != 0);
	ConvertUTF16toUTF8(folder, s_folderBuffer);
}

bool File::setCurrentFolder(const utf8string& folder, bool error)
{
	static utf16string s_setFolderBuffer;
	ConvertUTF8toUTF16(s_setFolderBuffer, folder);
	if (!SetCurrentDirectoryW(s_setFolderBuffer.c_str()))
	{
		if (error)
			assert(0);
		return false;
	}
	unsigned int sz = static_cast<unsigned int>(s_setFolderBuffer.length());
	return true;
}


const File::Names& File::findFiles(const utf8string& path, const utf8string& filterStr)
{
	//prepare to gether file names
	s_fileNames.clear();
	if (!path.empty() && !File::folderExists(path))
		return s_fileNames;

	//remember where we were, then change the current folder to where these new files might be
	utf8string buff;
	File::getCurrentFolder(buff);
	if (!File::setCurrentFolder(path, false))
	{
		File::setCurrentFolder(buff);
		return s_fileNames;
	}

	//iterate over every file in the folder
	struct _wfinddata_t c_file;
	intptr_t hFile;
	utf16string fStr;
	ConvertUTF8toUTF16(fStr, filterStr);
	if ((hFile = _wfindfirst((wchar_t*)fStr.c_str(), &c_file)) == -1L)
	{
		File::setCurrentFolder(buff);
	}
	else
	{
		do
		{
			if (!(c_file.attrib & _A_SUBDIR))
			{
				if (wcscmp(c_file.name, L".") == 0 || wcscmp(c_file.name, L"..") == 0)
					continue;
				//it's a real fiel so save the name
				utf8string fName;
				ConvertUTF16toUTF8(fName, c_file.name);
				s_fileNames.push_back(fName);
			}
		} while (_wfindnext(hFile, &c_file) == 0);
		_findclose(hFile);
		//set us back to where were before
		File::setCurrentFolder(buff);
	}
	return s_fileNames;
}

bool File::write(void const* pBuffer, const unsigned int size, unsigned int& bytesWritten)
{
	assert(m_isOpen);
	assert(m_writeAccess);
	assert(pBuffer);
	assert(size > 0);

	assert(m_pFStream->good());
	m_pFStream->write(static_cast<char const*>(pBuffer), static_cast<std::streamsize>(size));
	if (!m_pFStream->good())
	{
		assert(0);
		return false;
	}
	bytesWritten = size;
	return true;
}

//convenience functions
bool writeInternal(File& file, void* pData, unsigned int size)
{
	unsigned int bWritten = 0;
	if (!file.write(pData, size, bWritten))
	{
		assert(0);
		return false;
	}
	if (bWritten != size)
	{
		assert(0);
		return false;
	}
	return true;
}

bool File::write(File& file, const int myInt)
{
	return writeInternal(file, const_cast<int*>(&myInt), sizeof(myInt));
}

bool readInternal(File& file, void* pData, unsigned int size)
{
	unsigned int bRead = 0;
	if (!file.read(pData, size, bRead))
		return false;
	if (bRead != size)
		return false;
	return true;
}

bool File::read(File& file, int& myInt)
{
	return readInternal(file, &myInt, sizeof(myInt));
}
