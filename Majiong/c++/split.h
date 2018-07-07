#pragma once

class split
{
public:
	const static int CardTypeNum = 34;
	static bool get_hu_info(char* cards);
	static int GetHuPaiNeedCardNum(char *cards);
private:
	static int ForeachNeedCardNum(char* cards, int eye_color);
	static int NormalNeedCardNum(char* cards, int from, int to);
	static int ZiNeedCardNum(char* cards);
	static int Split(int n,int &needCard);
	static int One(int n,int &needCard);
	static int Two(int n, int &needCard);

	static bool foreach_eye(char* cards, char gui_num, char max_gui, int eye_color, int* cache);

	static int check_normal(char* cards, int from, int to, int max_gui, int cache_index, int* cache);

	static int next_split(int n, int gui_num, int max_gui);

	static int one(int n, int gui_num, int max_gui);
	static int two(int n, int gui_num, int max_gui);

	static inline int check_zi(char* cards, int max_gui, int cache_index, int* cache);
};
