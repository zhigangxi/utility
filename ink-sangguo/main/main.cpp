#include "main.h"
#include <iostream>
#include <signal.h>
#include <stdint.h>
#include "user.h"
#include "command.h"
#include "OnlineUser.h"
#include "ini.h"
#include "utility.h"
#include <boost/format.hpp>
#include "GlobalVal.h"
#include "UserPackage.h"
#include "database.h"
#include "mbedtls/pk.h"
#include "mbedtls/base64.h"
#include "mbedtls/ctr_drbg.h"
#include "ReadWriteUser.h"
#include "HeroTmpl.h"
#include "ItemTmpl.h"
#include "Force.h"
#include "SkillTmpl.h"
#include "WorldPath.h"
#include <boost/archive/binary_oarchive.hpp>
#include <google/protobuf/text_format.h>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include "Quest.h"
#include "Expressions.h"
#include "Award.h"
#include "Rank.h"
#include "RankReputation.h"
#include "Title.h"
#include <boost/random.hpp>
#include "EnemyTeam.h"
#include "Shop.h"
#include "Advanture.h"
#include "Evolve.h"
#include "LearnProperty.h"
#include "LearnSkill.h"
#include "HeroInst.h"
#include "ElectionTitle.h"
#include "CampMgr.h"
#include <condition_variable>
#include "CycleBattle.h"
#include "RecruitHero.h"
#include "VsBattle.h"
#include "Biography.h"
#include "RaffleHero.h"
#include "Banquet.h"
#include "WorldCity.h"
#include "MailInfo.h"
#include "WorldWar.h"
#include "LearnAndVsCD.h"
#include "GeneralSoulMgr.h"
#include "ResourceTmpl.h"
#include "ResourceInst.h"
using namespace std;

CServer *CServer::m_thisServer;

void CServer::SigHandler(int)
{
	m_thisServer->m_run = false;
	m_thisServer->m_socket.Stop();
}

#ifdef WIN32
BOOL CServer::HandlerRoutine(int)
{
	m_thisServer->m_run = false;
	m_thisServer->m_socket.Stop();
	return TRUE;
}
#endif

bool CServer::Init()
{
	//std::mutex mtx; // È«¾Ö»¥³âËø.
	//std::unique_lock <std::mutex> lck(mtx);
	//std::condition_variable cv;
	//cv.wait(lck);
#ifdef WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CServer::HandlerRoutine, TRUE);
#else 
	struct sigaction sig;
	sigemptyset(&sig.sa_mask);
	sig.sa_handler = SIG_IGN;
	sig.sa_flags = SA_ONESHOT;//SA_NOMASK;

	for (int i = 0; i < SIGRTMIN; i++)
	{
		if ((i != SIGINT) || (i != SIGTERM)
			|| (i != SIGSEGV) || (i != SIGABRT)
			|| (i != SIGFPE))
		{
			sigaction(i, &sig, NULL);
		}
	}

	sig.sa_handler = &(CServer::SigHandler);
	sigaction(SIGINT, &sig, NULL);
	sigaction(SIGFPE, &sig, NULL);
	sigaction(SIGTERM, &sig, NULL);
	sigaction(SIGSEGV, &sig, NULL);
	sigaction(SIGABRT, &sig, NULL);

#endif // !WIN32

	INI<> *pIniFile = new INI<>("config",true);
	CGlobalVal::SetData("read_ini", pIniFile);

	if (!pIniFile->select("server"))
	{
		cout << "read server config error" << endl;
		return false;
	}
	string port = pIniFile->get("port");

	if (!pIniFile->select("database"))
	{
		cout << "read database config error" << endl;
		return false;
	}

	string dbConInfo =  pIniFile->get("dbconnect");
	CDataBase *db = new CDataBase;
	if (!db->Connect(dbConInfo.c_str()))
	{
		char *err = db->GetErrMsg();
		if(err != nullptr)
			cout << err << endl;
		return false;
	}
	CGlobalVal::SetData("db_connect", db);

	CReadWriteUser::InitDb();
	CReadWriteUser::ReadUId();	
	
	if (!InitSingleton())
		return false;
	return m_socket.Accept(atoi(port.c_str()));
}

bool CServer::InitSingleton()
{
	srand((uint32_t)time(nullptr));

	DesignData::Expressions::sGetInstance().initialize();

	if (!CHeroMgr::get_mutable_instance().InitHero())
	{
		std::cout << "init hero error" << std::endl;
		return false;
	}
	if (!CItemMgr::get_mutable_instance().InitItem())
	{
		std::cout << "init item error" << std::endl;
		return false;
	}
	if (!CForceMgr::get_mutable_instance().Init())
	{
		std::cout << "init force error" << std::endl;
		return false;
	}
	if (!CSkillMgr::get_mutable_instance().InitSkill())
	{
		std::cout << "init skill error" << std::endl;
		return false;
	}
	if (!CWorldPathMgr::get_mutable_instance().Init())
	{
		std::cout << "init path error" << std::endl;
		return false;
	}
	if (!CQuestMgr::get_mutable_instance().InitQuest())
	{
		std::cout << "init quest error" << std::endl;
		return false;
	}
	if (!CAwardMgr::get_mutable_instance().Init())
	{
		cout << "init award error" << endl;
		return false;
	}
	if (!CRankMgr::get_mutable_instance().Init())
	{
		cout << "init rank error" << endl;
		return false;
	}
	if (!CLearnAndVsCDMgr::get_mutable_instance().Init())
	{
		cout << "init CLearnAndVsCDMgr error" << endl;
		return false;
	}
	if (!CRankReputationMgr::get_mutable_instance().Init())
	{
		cout << "init ReputationMgr error" << endl;
		return false;
	}
	if (!CRankLevelMgr::get_mutable_instance().Init())
	{
		cout << "init RankLevelMgr error" << endl;
		return false;
	}
	if (!CTitleMgr::get_mutable_instance().Init())
	{
		cout << "init title error" << endl;
		return false;
	}

	if (!CEnemyMgr::get_mutable_instance().Init())
	{
		cout << "init enemy error" << endl;
		return false;
	}

	if (!CEnemyTeamMgr::get_mutable_instance().Init())
	{
		cout << "init enemy team error" << endl;
		return false;
	}
	if (!CShopMgr::get_mutable_instance().Init())
	{
		cout << "init shop item error" << endl;
		return false;
	}
	if (!CAdvantureMgr::get_mutable_instance().Init())
	{
		cout << "init advanture error" << endl;
		return false;
	}
	if (!CEvolve::get_mutable_instance().Init())
	{
		cout << "init evolve error" << endl; 
		return false;
	}
	if (!CLearnProperty::get_mutable_instance().Init())
	{
		cout << "init learn property error" << endl;
		return false;
	}
	if (!CLearnSKill::get_mutable_instance().Init())
	{
		cout << "init learn skill error" << endl;
		return false;
	}

	if (!CReadWriteUser::ReadHeroPropertyExp())
	{
		cout << "init hero property exp error" << endl;
		return false;
	}
	if (!CElectionTitle::get_mutable_instance().Init())
	{
		cout << "init ElectionTitle error" << endl;
		return false;
	}
	if (!CCampMgr::get_mutable_instance().Init())
	{
		cout << "init CampMgr error" << endl;
		return false;
	}
	if (!CCycleBattle::get_mutable_instance().Init())
	{
		cout << "init cycle battle error" << endl;
		return false;
	}
	if (!CRecruitHero::get_mutable_instance().Init())
	{
		cout << "init recruit hero error" << endl;
		return false;
	}
	if (!CVsBattleMgr::get_mutable_instance().Init())
	{
		cout << "init vs battle error" << endl;
		return false;
	}
	if (!CBiography::get_mutable_instance().Init())
	{
		cout << "init Biography error" << endl;
		return false;
	}
	if (!CRaffleHero::get_mutable_instance().Init())
	{
		cout << "init RaffleHero error" << endl;
		return false;
	}
	if (!CHeroMgr::get_mutable_instance().InitOther())
	{
		cout << "init hero mgr other error" << endl;
		return false;
	}

	if (!CBanquet::get_mutable_instance().Init())
	{
		cout << "init Banquet error" << endl;
		return false;
	}

	if (!CMailInfo::get_mutable_instance().Init())
	{
		cout << "init mail info error" << endl;
		return false;
	}
	if (!CWorldWar::get_mutable_instance().Init())
	{
		cout << "init world war error" << endl;
		return false;
	}
	CReadWriteUser::UpdateTmpUseNick();

	if (!CUser::InitLevelUpExp())
	{
		cout << "init player levelup exp error" << endl;
		return false;
	}
	if (!CUser::InitFriendshipsWhenGetHero())
	{
		cout << "init player InitFriendshipsWhenGetHero exp error" << endl;
		return false;
	}

	if (!CGeneralSoulMgr::get_mutable_instance().Init())
	{
		cout << "init CGeneralSoulMgr error" << endl;
		return false;
	}
	if (!CResourceMgr::get_mutable_instance().Init())
	{
		cout << "init CResourceMgr error" << endl;
		return false;
	}
	/*CElectionTitle &election = CElectionTitle::get_mutable_instance();
	CUser user;
	user.SetUserId(100);
	user.SetNick("hello");
	user.SetForce(3);
	election.AddElectionUser(&user, 3);*/
	return true;
}

void CServer::Run()
{
	CCommand *command = new CCommand;
	COnlineUser *onlineUser = new COnlineUser;
	CGlobalVal::SetData("online_user", onlineUser);
	CGlobalVal::SetData("server_socket", &m_socket);

	//INI<> *ini = CGlobalVal::GetData<INI<>>("read_ini");
	CUtility::Init();

	m_pack.AddCmdPackage(command, new CUserPackage);
	m_pack.Run(command);

	m_socket.Run();
	m_pack.Stop();

	boost::thread *thread = m_pack.GetThread();
	thread->join();

	auto saveUser = [](CUser *user) {
		CReadWriteUser::WriteUser(*user);
	};
	onlineUser->ForEachUser(saveUser);
	CWorldCityMgr::get_const_instance().Save();
}

int RngCallback(void *v, unsigned char *uchar, size_t s)
{
	return s;
}

void decode()
{
	int ret = 0;
	mbedtls_pk_context pk;

	mbedtls_pk_init(&pk);

	/*
	* Read the RSA privatekey
	*/
	if ((ret = mbedtls_pk_parse_public_keyfile(&pk, "sign_key.pub")) != 0)
	{
		printf(" failed\n  ! mbedtls_pk_parse_public_keyfile returned -0x%04x\n", -ret);
		return;
	}
	mbedtls_rsa_context *rsaContext = mbedtls_pk_rsa(pk);

	unsigned char result[MBEDTLS_MPI_MAX_SIZE];
	size_t olen = 0;

	/*
	* Calculate the RSA encryption of the data.
	*/
	printf("\n  . Generating the encrypted value");
	fflush(stdout);

	unsigned char sign[] = "pquuRQTCNqipNaW4YC6o2eJn04fuQs4vxDkn+j/JNYK95+7rJd7AkuPsob4NldX03SR+J/qySeacV0f1SOVHAjmDv+MokBrv4qOQLaauoXuVYyDwVb4ydvqbY3L/XGf+K5XB6+VaohGO8EXWTq3/fH9KuLhccc5nql5obXCDH9b+5F7iR2daYLX2tko4I4XU26irNRGfGQXFOxWzl8MbMUXpQdmC8QfYd0GLHqKO5KvPB20lLxONQQAXeINnVPMUczwYqfFlOKHGokpVZztrnS2NJtGHJd0JPxBIXjOr0EwcJSeWbd/Inft7RIzl3a2/XaI/xWeYXs8/BT+MO6Bwww==";
	//unsigned char sign[] = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA+8lLalxx6Aeo9o/N43QAYQzQ4MiZi//81wqYizGsNg1e8ksKlx6+nO+1HFwJYgmLNO2+9dXtuUFZToiFChxaCASesLVXo7KowRlPUVk3DvJDBAt9RvXUuGz2H8RMa/QNqPnkhQRLIJ349seqLpTmjNmXLIAATDTNSdGTrIK3amXU8ytLFVMqlwcfg4zGRAtjsk14qWQzi5I3pFuHCWyzQ26V3H1t3TJUa2+byGcghixr5DlG5GImBwv42xiOOwdhqWXHhoZJOIPJTo6qnMYihhNjBAZupKKzrvfR3PEA4Xpr+vdlLEvPJJMFV7xFZcA5ja6UBr93okVGDI7SLBh8owIDAQAB";
	//unsigned char noBase64[] = "L2+3LacLQtYTGTWMkuuR+1ZhuARxAAOeeQ3XhSlrjWHb5ub7gstgOYycXvQoX+YTC1dTkbSRavJHFl04lCT8jbU8cSa0TWOoscaZVOYycFjGG4NYPjUIO/Yy78hljgMsBNrpqmYRicylrpUVrV5oCx1E7kTSoECq5TB+LQrYyDM=&emailBound=";

	unsigned char inputBuf[1024];
	size_t outLen = sizeof(inputBuf);

	ret = mbedtls_base64_decode(inputBuf, sizeof(inputBuf), &outLen, sign, strlen((char*)sign));
	//MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL
	//MBEDTLS_ERR_BASE64_INVALID_CHARACTER
	//outLen = 256;
	/*if ((ret = mbedtls_pk_encrypt(&pk, inputBuf, outLen, result, &olen, sizeof(result),
		RngCallback, nullptr)) != 0)
	{
		printf(" failed\n  ! mbedtls_pk_decrypt returned -0x%04x\n", -ret);
		return;
	}*/
	ret = mbedtls_rsa_pkcs1_decrypt(rsaContext,
		nullptr,
		nullptr,
		MBEDTLS_RSA_PUBLIC, &outLen,
		inputBuf,
		result,sizeof(result));

	if ((ret = mbedtls_pk_decrypt(&pk, inputBuf, outLen, result, &olen, sizeof(result),
		mbedtls_ctr_drbg_random, nullptr)) != 0)
	{
		printf(" failed\n  ! mbedtls_pk_decrypt returned -0x%04x\n", -ret);
		return;
	}
	mbedtls_pk_free(&pk);
	/*ret = mbedtls_rsa_pkcs1_encrypt(&pk,
		RngCallback,
		void *p_rng,
		int mode, size_t ilen,
		const unsigned char *input,
		unsigned char *output);*/
}

int main(int argc,char **argv)
{
	/*boost::uniform_real<> distribution(2, 1);
	boost::kreutzer1986 engine;
	boost::variate_generator<boost::kreutzer1986, boost::uniform_real<> > myrandom(engine, distribution);*/
	/*for (int i = 0; i<100; ++i)
		cout << myrandom() << endl;*/
	/*std::vector<int> intVec = { 1002,100,200,500,1000 };
	std::sort(intVec.begin(), intVec.end());
	auto iter = std::lower_bound(intVec.begin(), intVec.end(), 1);*/
	//bool flag = boost::lexical_cast<bool>(0);
	/*std::ostringstream os;
	boost::archive::binary_oarchive oa(os);

	int arr[12] = { 0 };
	oa & arr;

	cout << os.str() << endl;*/

	/*NetMsg::ItemInfo item;
	string strMsg;
	item.set_id(10);
	item.set_num(100);
	item.set_uid(3456);
	google::protobuf::TextFormat::PrintToString(item, &strMsg);
	cout << strMsg << endl;*/

	//decode();
#ifndef WIN32
	int  c = 0;
	while ((c = getopt(argc, argv, "bh")) != -1)
	{
		switch (c)
		{
		case 'b':
			daemon(1, 0);
			break;
		default:
		case 'h':
			cout << "  -b         run as a background process (daemon)" << endl;
			exit(0);
		}
	}
#endif // !WIN32

	CServer server;
	try
	{
		if(server.Init())
			server.Run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}

