// test_client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "net_msg.h"
#include "comnect_socket.h"
#include "IniFile.h"
#include "DatabaseSql.h"
#include <boost/thread.hpp>


int _tmain(int argc, _TCHAR* argv[])
{
    string str = CIniFile::GetValue("ConnectNumber","TestClient","test.ini");
    int num = atoi(str.c_str());

    string serverPort = CIniFile::GetValue("Port","TestClient","test.ini");
    string serverIp = CIniFile::GetValue("Ip","TestClient","test.ini");
    
    CConnect con;
    con.Init();
    int i = 0;
    for(; i < num; i++)
    {
        con.Connect(serverIp.c_str(),serverPort.c_str());
    }
    
    CDealMsg package;

    boost::thread th(boost::bind(&CDealMsg::SendMsg,&package,&con));
    boost::thread th1(boost::bind(&CDealMsg::RecvMsg,&package,&con));

    while (con.DespatchEvent())
    {
    }
    th.join();
    //CDbPool *pPool = CDbPool::CreateInstance();
    //pPool->SetDbConfigure("root","123456","localhost","supress","3306");
    //for (int i = 0; i < 10; i++)
    //{
    //    pPool->AddDbConnect();
    //}
    //

    //while(1)
    //{
    //    CGetDbConnect getDb;
    //    CDatabaseSql *pDb = getDb.GetDbConnect();
    //    pDb->Query("UPDATE `supress`.`npc_task` SET `name` = '【主线.杀戮】' WHERE `npc_task`.`id` =1 AND `npc_task`.`npc_id` =1 AND `npc_task`.`name` = '【主?' AND `npc_task`.`type` =0 AND `npc_task`.`repeat_times` =1 AND `npc_task`.`need_level` =0 AND `npc_task`.`occupation` =0 AND `npc_task`.`depend_task_id` =0 AND `npc_task`.`need_item` = '1|6|1|1|14001|2' AND `npc_task`.`award_item` = '3|200|0|4|500|0' LIMIT 1 ;");
    //    FieldsRow row;
    //    while(pDb->GetResult(row))
    //    {
    //        //cout<<atoi(row["userid"])<<endl<<atoi(row["roleid"])<<endl<<row["rolename"]<<endl<<row["id"]<<endl;;
    //        // 	race 	occupation 	level 	experence 	power 	quick 	intelligence 	energy 	
    //    }
    //}
    
	return 0;
}

