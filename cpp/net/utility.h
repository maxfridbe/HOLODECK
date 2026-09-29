#ifndef UTILITY_H
#define UTILITY_H

struct Header;

class Utility
{
	public:
		static int Serialize(const Header & header, char * buffer);
		static int Deserialize(Header & header, const char * buffer);
		static int Serialize(const float * array, int count, char * buffer);
		static int Deserialize(float * array, int count, const char * buffer);
	private:
		Utility() {}
};

#endif
