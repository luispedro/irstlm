#ifndef IRSTLM_UTIL_H
#define IRSTLM_UTIL_H

#include <string>
#include <fstream>
#include "gzfilebuf.h"

const float LOWEST_SCORE        = -100.0f;

std::string gettempfolder();
void createtempfile(std::ofstream  &fileStream, std::string &filePath, std::ios_base::openmode flags);
void removefile(const std::string &filePath);

class inputfilestream : public std::istream
{
protected:
	std::streambuf *m_streambuf;
  bool _good;
public:
  
	inputfilestream(const std::string &filePath);
	~inputfilestream();
  bool good(){return _good;}
	void close();
};

void *MMap(int	fd, int	access, off_t	offset, size_t	len, off_t	*gap);
int Munmap(void	*p,size_t	len,int	sync);

inline float FloorScore(float logScore)
{
        return (std::max)(logScore , LOWEST_SCORE);
}

#endif

