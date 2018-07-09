#ifndef _SINGLETON_H_
#define _SINGLETON_H_
#include "log.h"
#include "despatch_command.h"
#include "socket_server.h"
#include "scene_manager.h"
#include "npc_manager.h"
#include "online_user.h"
#include "item.h"
#include "monster.h"
#include "fight.h"
#include "bangpai.h"
#include <boost/pool/detail/singleton.hpp>

typedef boost::details::pool::singleton_default<CLogs> SingletonClog;
typedef boost::details::pool::singleton_default<CDespatchCommand> SingletonDespatch;
typedef boost::details::pool::singleton_default<CSocketServer> SingletonSocket;
typedef boost::details::pool::singleton_default<CSceneManager> SingletonSceneManager;
typedef boost::details::pool::singleton_default<CNpcManager> SingletonNpcManager;
typedef boost::details::pool::singleton_default<COnlineUser> SingletonOnlineUser;
typedef boost::details::pool::singleton_default<CItemTemplateManager> SingletonItemManager;
typedef boost::details::pool::singleton_default<CMonsterManager> SingletonMonsterManager;
typedef boost::details::pool::singleton_default<CFightManager> SingletonFightManager;
typedef boost::details::pool::singleton_default<CBangPaiManager> SingletonCBangPaiManager;
#endif
