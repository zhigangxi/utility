
#pragma  once
#include "net_msg.h"
#include "../common/include/pugixml.hpp"
#include "../common/include/pugiconfig.hpp"
#include "vector3d.hpp"
#include <ctime>
#include <boost/shared_ptr.hpp>
#define MAX_RANDOM 1000
class logic_data_loader
{
public:
	logic_data_loader(const char* file_name)
		:file_name_(file_name)
	{
		srand((unsigned)time(NULL));
		doc_.load_file(file_name_.c_str());
		if (doc_==NULL)
		{
			std::cout<<"加载"<<file_name<<"失败"<<std::endl;
		}
	}

	bool fetch_data(CNetMessage& pack)
	{
		int count = 0;
		pugi::xml_node node = doc_.child("Logic").child("NpcList");
		if (node)
		{
			for (pugi::xml_node::iterator iter = node.begin();
				iter != node.end();
				iter++)
			{
				count++;
			}
			pack<<count;
			for (pugi::xml_node::iterator iter = node.begin();
				iter != node.end();
				iter++)
			{
				npc_id = atoi(iter->attribute("id").value());
				monster_id = atoi(iter->child_value("logicid"));
				pos.x_ = (float)atof(iter->child_value("posx"));
				pos.y_ = (float)atof(iter->child_value("posy"));
				pos.z_ = (float)atof(iter->child_value("posz"));
				dir.x_ = (float)atof(iter->child_value("dirx"));
				dir.y_ = (float)atof(iter->child_value("diry"));
				dir.z_ = (float)atof(iter->child_value("dirz"));

				pack<<npc_id<<monster_id<<pos.x_<<pos.y_<<pos.z_<< rand()%8;/*<<dir.x_<<dir.y_<<dir.z_*/;
       		}
			return true;
		}
		else
		{
			pack<<count;
			return false;
		}	
	}
private:
	pugi::xml_document doc_;
	std::string file_name_;
	float random(float a,float b)         //产生随机浮点数函数
	{
		float minnum = min(a,b);
		float maxnum = max(a,b);
		return (rand()%(int)((maxnum-minnum)*MAX_RANDOM))/(float)MAX_RANDOM+minnum;
	}
private:
	int npc_id;
	int monster_id;
	vector3d pos;
	vector3d dir;
};

typedef boost::shared_ptr<logic_data_loader> logic_data_loader_ptr;